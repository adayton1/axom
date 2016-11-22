/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * All rights reserved.
 *
 * This source code cannot be distributed without permission and
 * further review from Lawrence Livermore National Laboratory.
 */


/**
 * \file
 * \brief This file contains a utility to convert a Sidre datastore
 *        from the sidre_hdf5 protocol to another supported protocol.
 *
 * Users must supply a path to a sidre_hdf5 rootfile and base name for
 * the output datastores.  Can optionally provide a --protocol option
 * and/or  a --strip option to truncate the data to at most N elements.
 *
 * Usage:
 *    ./convert_sidre_protocol     -- prints out usage information
 *    ./convert_sidre_protocol --input path_to_datastore_root_file \
 *                             --output path_to_output_datastore \
 *                             --protocol a_supported_protocol  (default json) \
 *                             --strip N
 */
#include "mpi.h"

#include "fmt/fmt.hpp"
#include "slic/slic.hpp"
#include "slic/LumberjackStream.hpp"

#include "sidre/SidreTypes.hpp"
#include "sidre/DataStore.hpp"
#include "sidre/DataGroup.hpp"
#include "sidre/DataBuffer.hpp"
#include "sidre/DataView.hpp"

#include "spio/IOManager.hpp"

#include "slam/SizePolicies.hpp"
#include "slam/OffsetPolicies.hpp"
#include "slam/StridePolicies.hpp"
#include "slam/OrderedSet.hpp"

#include <limits>       // for numeric_limits<int>
#include <cstdlib>      // for atoi


using asctoolkit::sidre::DataStore;
using asctoolkit::sidre::DataGroup;
using asctoolkit::sidre::DataBuffer;
using asctoolkit::sidre::DataView;
using asctoolkit::spio::IOManager;


typedef asctoolkit::sidre::IndexType IndexType;
typedef asctoolkit::slam::policies::RuntimeOffsetHolder<IndexType> OffsetPolicy;
typedef asctoolkit::slam::policies::RuntimeStrideHolder<IndexType> StridePolicy;
typedef asctoolkit::slam::policies::RuntimeSizeHolder<IndexType>   SizePolicy;
typedef asctoolkit::slam::OrderedSet<SizePolicy, OffsetPolicy, StridePolicy> DataViewSet;

void setupLogging();
void teardownLogging();

/** Simple structure to hold the parsed command line arguments */
struct CommandLineArguments
{
    static const int NUM_SIDRE_PROTOCOLS = 7;
    static const std::string s_validProtocols[NUM_SIDRE_PROTOCOLS];

    std::string m_inputName;
    std::string m_outputName;
    std::string m_protocol;
    int m_numStripElts;

    CommandLineArguments()
     : m_inputName(""),
       m_outputName(""),
       m_protocol(""),
       m_numStripElts(-1)
     {}

   bool hasInputName() const { return !m_inputName.empty(); }
   bool hasOutputName() const { return !m_outputName.empty(); }
   bool hasOutputProtocol() const { return !m_protocol.empty(); }
   bool shouldStripData() const { return m_numStripElts >= 0; }

   /**  Returns the maximum allowed elements in a view of the output datastore */
   int maxEltsPerView() const
   {
       return shouldStripData()
               ? m_numStripElts
               : std::numeric_limits<int>::max();
   }

   /** Checks whether the input string is a valid sidre protocol */
   static bool isValidProtocol(const std::string& protocol)
   {
       return std::find(s_validProtocols, s_validProtocols + NUM_SIDRE_PROTOCOLS, protocol);
   }

   /** Logs usage information for the utility */
   static void usage()
   {
       fmt::MemoryWriter out;
       out << "Usage ./spio_convert_format <options>";
       out.write("\n\t{:<30}{}", "--help", "Output this message and quit");
       out.write("\n\t{:<30}{}", "--input <file>", "(required) Filename of input datastore");
       out.write("\n\t{:<30}{}", "--output <file>", "(required) Filename of output datastore");
       out.write("\n\t{:<30}{}", "--strip <N>", "Indicates if data in output file should be "
                                                    "stripped (to first N entries) (default: off)");
       out.write("\n\t{:<30}{}", "--protocol <str>", "Desired protocol for output datastore");

       out.write("\n\n\t{: <40}","Available protocols:");
       for(int i=0; i< NUM_SIDRE_PROTOCOLS; ++i)
       {
           out.write("\n\t  {: <50}", s_validProtocols[i]);
       }

       SLIC_INFO( out.str() );
   }

} ;

const std::string CommandLineArguments::s_validProtocols[] = {
        "json",
        "sidre_hdf5",
        "sidre_conduit_json",
        "sidre_json",
        "conduit_hdf5",
        "conduit_bin",
        "conduit_json",
};


/** Terminates execution */
void quitProgram(int exitCode = 0)
{
    teardownLogging();
    MPI_Finalize();
    exit(exitCode);
}


/**
 * \brief Utility to parse the command line options
 * \return An instance of the CommandLineArguments struct.
 */
CommandLineArguments parseArguments(int argc, char** argv, int myRank)
{
    CommandLineArguments clargs;

    for(int i=1; i< argc; ++i)
    {
        std::string arg(argv[i]);
        if(arg == "--input")
        {
            clargs.m_inputName = std::string(argv[++i]);
        }
        else if(arg == "--output")
        {
            clargs.m_outputName = std::string(argv[++i]);
        }
        else if(arg == "--protocol")
        {
            clargs.m_protocol = std::string(argv[++i]);
        }
        else if(arg == "--strip")
        {
            clargs.m_numStripElts = std::atoi(argv[++i]);
        }
        else if(arg == "--help" || arg == "-h" )
        {
            if(myRank == 0)
            {
                clargs.usage();
            }
            quitProgram();
        }
    }

    // Input file name is required
    bool isValid = true;;
    if(!clargs.hasInputName())
    {
        SLIC_WARNING("Must supply an input datastore root file.");
        isValid = false;
    }

    if(!clargs.hasOutputName())
    {
        SLIC_WARNING("Must supply a filename for the output datastore.");
        isValid = false;
    }


    // Check that protocol is valid or supply one
    if(!clargs.hasOutputProtocol())
    {
        clargs.m_protocol = CommandLineArguments::s_validProtocols[0];
    }
    else
    {
        if( ! clargs.isValidProtocol( clargs.m_protocol ) )
        {
            SLIC_WARNING( clargs.m_protocol << " is not a valid sidre protocol.");
            isValid = false;
        }

    }

    if(!isValid)
    {
        if(myRank == 0)
        {
            clargs.usage();
        }
        quitProgram(1);
    }

    return clargs;
}


/**
 * \brief Helper function to allocate storage for the external data of the input datastore
 *
 * Iterates recursively through the views and groups of the provided group to find
 * the external data views and allocates the required storage within the extPtrs vector
 *
 * \param grp  The group on which we are traversing
 * \param extPtrs [in] An input vector to hold pointers to the allocated data
 *
 * \note We also set the data in each allocated array to zeros
 */
void allocateExternalData(DataGroup* grp, std::vector<void*>& extPtrs)
{
    // for each view
    for(asctoolkit::sidre::IndexType idx =  grp->getFirstValidViewIndex();
        asctoolkit::sidre::indexIsValid(idx);
        idx = grp->getNextValidViewIndex(idx) )
    {
        DataView* view = grp->getView(idx);
        if(view->isExternal())
        {
            SLIC_INFO("External view " << view->getPathName()
                      << " has " << view->getNumElements() << " elements "
                      << "(" << view->getTotalBytes() << " bytes)."
                );

            const int idx = extPtrs.size();
            const int sz = view->getTotalBytes();
            extPtrs.push_back(new char[sz]);
            std::memset(extPtrs[idx], 0, sz);
            view->setExternalDataPtr( extPtrs[ idx ]);
        }
    }

    // for each group
    for(asctoolkit::sidre::IndexType idx =  grp->getFirstValidGroupIndex();
        asctoolkit::sidre::indexIsValid(idx);
        idx = grp->getNextValidGroupIndex(idx) )
    {
        allocateExternalData(grp->getGroup(idx), extPtrs);
    }
}

/**
 * Shifts the data to the right by two elements,
 * The new first value will be the size of the original array
 * The next values will be 0 for integer data and Nan for float data
 * This is followed by the initial values in the original dataset
 *
 * \param view The array view on which we are operating
 * \param origSize The size of the original array
 */
template<typename sidre_type>
void modifyFinalValuesImpl(DataView* view, int origSize)
{
    SLIC_DEBUG("Looking at view " << view->getPathName());

    // Note: offset is set to zero since getData() already accounts for the offset
    sidre_type* arr = view->getData();
    const int elem_bytes = view->getSchema().dtype().element_bytes();
    const int offset = 0; 
    const int stride = view->getSchema().dtype().stride() / elem_bytes;

    // Uses a Slam set to help manage the indirection to the view data
    DataViewSet idxSet = DataViewSet::SetBuilder()
                        .size(view->getNumElements())
                        .offset(offset)
                        .stride(stride);

    fmt::MemoryWriter out_fwd;
    for(int i=0; i < idxSet.size(); ++i)
    {
        out_fwd.write("\n\ti: {}; set[i]: {}; arr [ set[i] ] = {}", i, idxSet[i], arr[ idxSet[i] ] );
    }
    SLIC_DEBUG( out_fwd.str() );


    // Shift the data over by two
    const int local_offset = 2;
    for(int i=idxSet.size()-1; i >= local_offset; --i)
    {
        arr[ idxSet[i]] = arr[ idxSet[i-local_offset]];
    }

    // Set the first two elements
    arr[ idxSet[0] ] = static_cast<sidre_type>(origSize);
    arr[ idxSet[1] ] = std::numeric_limits<sidre_type>::quiet_NaN();


    fmt::MemoryWriter out_rev;
    for(int i=0; i < idxSet.size(); ++i)
    {
        out_rev.write("\n\ti: {}; set[i]: {}; arr [ set[i] ] = {}", i, idxSet[i], arr[ idxSet[i] ] );
    }
    SLIC_DEBUG( out_rev.str() );
}


void modifyFinalValues(DataView* view, int origSize)
{
    SLIC_DEBUG("Truncating view " << view->getPathName());

    using namespace asctoolkit::sidre;

    switch(view->getTypeID())
    {
      case DataTypeId::INT8_ID:
          modifyFinalValuesImpl<detail::sidre_int8>(view, origSize);
          break;
      case DataTypeId::INT16_ID:
          modifyFinalValuesImpl<detail::sidre_int16>(view, origSize);
          break;
      case DataTypeId::INT32_ID:
          modifyFinalValuesImpl<detail::sidre_int32>(view, origSize);
          break;
      case DataTypeId::INT64_ID:
          modifyFinalValuesImpl<detail::sidre_int64>(view, origSize);
          break;
      case DataTypeId::UINT8_ID:
          modifyFinalValuesImpl<detail::sidre_uint8>(view, origSize);
          break;
      case DataTypeId::UINT16_ID:
          modifyFinalValuesImpl<detail::sidre_uint16>(view, origSize);
          break;
      case DataTypeId::UINT32_ID:
          modifyFinalValuesImpl<detail::sidre_uint32>(view, origSize);
          break;
      case DataTypeId::UINT64_ID:
          modifyFinalValuesImpl<detail::sidre_uint64>(view, origSize);
          break;
      case DataTypeId::FLOAT32_ID:
          modifyFinalValuesImpl<detail::sidre_float32>(view, origSize);
          break;
      case DataTypeId::FLOAT64_ID:
          modifyFinalValuesImpl<detail::sidre_float64>(view, origSize);
          break;
      default:
          break;
    }
}

/**
 * \brief Recursively traverse views and groups in grp and truncate views to have at most maxSize+2 elements.
 *
 * Within the truncated arrays, the first element will be the size of the original array and the second will
 * be 0 for integers and nan for floating points.
 * This will be followed by (at most) the first maxSize elements of the original array
 */
void truncateBulkData(DataGroup* grp, int maxSize)
{
    // Add two to maxSize
    for(asctoolkit::sidre::IndexType idx =  grp->getFirstValidViewIndex();
        asctoolkit::sidre::indexIsValid(idx);
        idx = grp->getNextValidViewIndex(idx) )
    {
        DataView* view = grp->getView(idx);
        bool isArray = view->hasBuffer() || view->isExternal();

        if(isArray)
        {
            const int numOrigElts = view->getNumElements();
            const int newSize = std::min(maxSize+2, numOrigElts);

            if(view->hasBuffer() && numOrigElts > newSize)
            {
                const int numEltBytes = view->getSchema().dtype().element_bytes();
                const int viewStride = view->getSchema().dtype().stride() / numEltBytes;
                const int viewOffset = view->getSchema().dtype().offset() / numEltBytes;
                view->apply(newSize,viewOffset, viewStride);
            }
            // external
            else if(view->isExternal() && numOrigElts > newSize)
            {
                view->setExternalDataPtr(view->getTypeID(),newSize, view->getVoidPtr());
            }

            modifyFinalValues(view, numOrigElts);
        }
    }

    // for each group
    for(asctoolkit::sidre::IndexType idx =  grp->getFirstValidGroupIndex();
        asctoolkit::sidre::indexIsValid(idx);
        idx = grp->getNextValidGroupIndex(idx) )
    {
        truncateBulkData(grp->getGroup(idx), maxSize);
    }
}

/** Sets up the logging using lumberjack */
void setupLogging()
{
    using namespace asctoolkit;

    slic::initialize();

    slic::setLoggingMsgLevel(slic::message::Info);

    // Formatting for warning, errors and fatal message
       fmt::MemoryWriter wefFmt;
       wefFmt << "\n***********************************\n"
            <<"[<RANK>][<LEVEL> in line <LINE> of file <FILE>]\n"
            <<"MESSAGE=<MESSAGE>\n"
            << "***********************************\n";
    std::string wefFormatStr = wefFmt.str();

    // Simple formatting for debug and info messages
    std::string diFormatStr = "[<RANK>][<LEVEL>]: <MESSAGE>\n";

    const int ranksLimit = 16;

    slic::LumberjackStream* wefStream =
          new slic::LumberjackStream( &std::cout, MPI_COMM_WORLD, ranksLimit, wefFormatStr );
    slic::LumberjackStream* diStream =
          new slic::LumberjackStream( &std::cout, MPI_COMM_WORLD, ranksLimit, diFormatStr );

    slic::addStreamToMsgLevel(wefStream, slic::message::Fatal) ;
    slic::addStreamToMsgLevel(wefStream, slic::message::Error);
    slic::addStreamToMsgLevel(wefStream, slic::message::Warning);
    slic::addStreamToMsgLevel(diStream,  slic::message::Info);
    slic::addStreamToMsgLevel(diStream,  slic::message::Debug);
}

/** Finalizes logging and flushes streams */
void teardownLogging()
{
    asctoolkit::slic::finalize();
}

int main(int argc, char * argv[])
{
  MPI_Init(&argc, &argv);


  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  setupLogging();

  // parse the command arguments
  CommandLineArguments args = parseArguments(argc, argv, my_rank);

  // Load the original datastore
  DataStore ds;

  SLIC_INFO("Loading datastore from " << args.m_inputName);
  IOManager manager(MPI_COMM_WORLD);
  manager.read(ds.getRoot(), args.m_inputName);
  int num_files = manager.getNumFilesFromRoot(args.m_inputName);

  // restore any external data pointers
  SLIC_INFO("Loading external data from datastore");
  std::vector<void*> externalDataPointers;
  allocateExternalData(ds.getRoot(), externalDataPointers);
  manager.loadExternalData(ds.getRoot(), args.m_inputName);

  asctoolkit::slic::flushStreams();

  // Internal processing
  if(args.shouldStripData())
  {
      const int numElts = args.maxEltsPerView();
      SLIC_INFO("Truncating views to at most " << numElts << " elements.");

      truncateBulkData(ds.getRoot(), numElts);

      // Add a string view to the datastore to indicate that we modified the data
      fmt::MemoryWriter fout;
      fout << "This datastore was created by the spio_convert_format utility "
           << "with option '--strip " << numElts <<  "'. "
           << "To simplify debugging, the bulk data in this datastore has been truncated to have "
           << "at most " << numElts << " actual values. The first value is the original array size, "
           << "which is followed by a zero/Nan, "
           << "which is followed by (at most) the first " << numElts << " values.";
      ds.getRoot()->createViewString("Note", fout.str());
  }

  // Write out datastore to the output file in the specified protocol
  SLIC_INFO("Writing out datastore in " << args.m_protocol
            << " protocol to file(s) with base name " << args.m_outputName);
  manager.write(ds.getRoot(), num_files, args.m_outputName, args.m_protocol);


  // Free up memory associated with external data
  for(std::vector<void*>::iterator it = externalDataPointers.begin();
          it != externalDataPointers.end();
          ++it)
  {
      delete [] static_cast<char*>(*it);
      *it = ATK_NULLPTR;
  }

  teardownLogging();
  MPI_Finalize();
  return 0;
}
