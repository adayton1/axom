/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2017-2018, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-741217
 *
 * All rights reserved.
 *
 * This file is part of Axom.
 *
 * For details about use and distribution, please read axom/LICENSE.
 *
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#ifndef QUEST_HELPERS_HPP_
#define QUEST_HELPERS_HPP_

// Axom includes
#include "axom/config.hpp"           // for compile-time definitions

// Mint includes
#include "axom/mint/mesh/Mesh.hpp"             // for mint::Mesh
#include "axom/mint/mesh/UnstructuredMesh.hpp" // for mint::UnstructuredMesh

// Slic includes
#include "axom/slic/interface/slic.hpp"  // for SLIC macros
#include "axom/slic/streams/GenericOutputStream.hpp"
#if defined(AXOM_USE_MPI) && defined(AXOM_USE_LUMBERJACK)
  #include "axom/slic/streams/LumberjackStream.hpp"
#elif defined(AXOM_USE_MPI) && !defined(AXOM_USE_LUMBERJACK)
  #include "axom/slic/streams/SynchronizedStream.hpp"
#endif

// Quest includes
#include "axom/quest/interface/internal/mpicomm_wrapper.hpp"
#include "axom/quest/stl/STLReader.hpp"
#ifdef AXOM_USE_MPI
#include "axom/quest/stl/PSTLReader.hpp"
#endif

// C/C++ includes
#include <string> // for C++ string

/*!
 * \file
 *
 * \brief Helper methods that can be used across the different Quest queries.
 */
namespace axom
{
namespace quest
{
namespace internal
{

constexpr int READ_FAILED  = -1;
constexpr int READ_SUCCESS = 0;

/// \name MPI Helper/Wrapper Methods
/// @{
#ifdef AXOM_USE_MPI

/*!
 * \brief Deallocates the specified MPI window object.
 * \param [in] window handle to the MPI window.
 * \note All buffers attached to the window are also deallocated.
 */
void mpi_win_free( MPI_Win* window )
{
  if ( *window != MPI_WIN_NULL )
  {
    MPI_Win_free( window );
  }
}

/*!
 * \brief Deallocates the specified MPI communicator object.
 * \param [in] comm handle to the MPI communicator object.
 */
void mpi_comm_free( MPI_Comm* comm )
{

  if ( *comm != MPI_COMM_NULL )
  {
    MPI_Comm_free( comm );
  }
}

/*!
 * \brief Reads the mesh on rank 0 and exchanges the mesh metadata, i.e., the
 *  number of nodes and faces with all other ranks.
 *
 * \param [in] global_rank_id MPI rank w.r.t. the global communicator
 * \param [in] global_comm handle to the global communicator
 * \param [in,out] reader the corresponding STL reader
 * \param [out] mesh_metadata an array consisting of the mesh metadata.
 *
 * \note This method calls read() on the reader on rank 0.
 *
 * \pre global_comm != MPI_COMM_NULL
 * \pre mesh_metadata != nullptr
 */
int read_and_exchange_mesh_metadata( int global_rank_id,
                                     MPI_Comm global_comm,
                                     quest::STLReader& reader,
                                     mint::IndexType mesh_metadata[ 2 ] )
{
  constexpr int NUM_NODES = 0;
  constexpr int NUM_FACES = 1;
  constexpr int ROOT_RANK = 0;

  switch( global_rank_id )
  {
  case 0:
    if ( reader.read() == READ_SUCCESS )
    {
      mesh_metadata[ NUM_NODES ] = reader.getNumNodes();
      mesh_metadata[ NUM_FACES ] = reader.getNumFaces();
    }
    else
    {
      SLIC_WARNING( "reading STL file failed, setting mesh to NULL" );
      mesh_metadata[ NUM_NODES ] = READ_FAILED;
      mesh_metadata[ NUM_FACES ] = READ_FAILED;
    }
    MPI_Bcast( mesh_metadata, 2, MPI_INT, ROOT_RANK, global_comm );
    break;
  default:
    MPI_Bcast( mesh_metadata, 2, MPI_INT, ROOT_RANK, global_comm );
  }

  int rc = (mesh_metadata[NUM_NODES]==READ_FAILED) ? READ_FAILED : READ_SUCCESS;
  return rc;
}

#endif /* AXOM_USE_MPI */

/*!
 * \brief Creates inter-node and intra-node communicators from the given global
 *  MPI communicator handle.
 *
 *  The intra-node communicator groups the ranks within the same compute node.
 *  Consequently, all ranks have a global rank ID, w.r.t. the global
 *  communicator, and a corresponding local rank ID, w.r.t. the intra-node
 *  communicator. The global rank ID can span and is unique across multiple
 *  compute nodes, while the local rank ID, is only uniquely defined within the
 *  same compute node and is generally different from the global rank ID.
 *
 *  In contrast, the inter-node communicator groups only a subset of the ranks
 *  defined by the global communicator. Specifically, ranks that have a
 *  local rank ID of zero are included in the inter-node commuinicator and
 *  have a corresponding inter-comm rank ID, w.r.t., the inter-node
 *  communicator. For all other ranks, that are not included in the inter-node
 *  communicator, the inter-comm rank ID is set to "-1".
 *
 * \param [in]  global_comm handle to the global MPI communicator.
 * \param [out] intra_node_comm handle to the intra-node communicator object.
 * \param [out] inter_node_comm handle to the inter-node communicator object.
 * \param [out] global_rank_id rank ID w.r.t. the global communicator
 * \param [out] local_rank_id rank ID within the a compute node
 * \param [out] intercom_rank_id rank ID w.r.t. the inter-node communicator

 * \note The caller must call `MPI_Comm_free` on the corresponding communicator
 *  handles, namely, `intra_node_comm` and `inter_node_comm` which are created
 *  by this routine.
 *
 * \pre global_comm != MPI_COMM_NULL
 * \pre intra_node_comm == MPI_COMM_NULL
 * \pre inter_node_comm == MPI_COMM_NULL
 * \post intra_node_comm != MPI_COMM_NULL
 * \post inter_node_comm != MPI_COMM_NULL
 */
#ifdef AXOM_USE_MPI3
void create_communicators( MPI_Comm global_comm,
                           MPI_Comm& intra_node_comm,
                           MPI_Comm& inter_node_comm,
                           int& global_rank_id,
                           int& local_rank_id,
                           int& intercom_rank_id )
{
  // Sanity checks
  SLIC_ASSERT( global_comm != MPI_COMM_NULL );
  SLIC_ASSERT( intra_node_comm == MPI_COMM_NULL );
  SLIC_ASSERT( inter_node_comm == MPI_COMM_NULL );

  constexpr int IGNORE_KEY = 0;

  // STEP 0: get global rank, used to order ranks in the inter-node comm.
  MPI_Comm_rank( global_comm, &global_rank_id );

  // STEP 1: create the intra-node communicator
  MPI_Comm_split_type( global_comm, MPI_COMM_TYPE_SHARED, IGNORE_KEY,
                       MPI_INFO_NULL, &intra_node_comm );
  MPI_Comm_rank( intra_node_comm, &local_rank_id );
  SLIC_ASSERT( local_rank_id >= 0 );

  // STEP 2: create inter-node communicator
  const int color = ( local_rank_id==0 ) ? 1 : MPI_UNDEFINED;
  MPI_Comm_split( global_comm, color, global_rank_id, &inter_node_comm );

  if ( color == 1 )
  {
    MPI_Comm_rank( inter_node_comm, &intercom_rank_id );
  }

  SLIC_ASSERT( intra_node_comm != MPI_COMM_NULL );
}
#endif

/*!
 * \brief Allocates a shared memory buffer for the mesh that is shared among
 *  all the ranks within the same compute node.
 *
 * \param [in] intra_node_comm intra-node communicator within a node.
 * \param [in] mesh_metada tuple with the number of nodes/faces on the mesh
 * \param [out] x pointer into the buffer where the x--coordinates are stored.
 * \param [out] y pointer into the buffer where the y--coordinates are stored.
 * \param [out] z pointer into the buffer where the z--coordinates are stored.
 * \param [out] conn pointer into the buffer consisting the cell-connectivity.
 * \param [out] mesh_buffer raw buffer consisting of all the mesh data.
 * \param [out] shared_window MPI window to which the shared buffer is attached.
 *
 * \return bytesize the number of bytes in the raw buffer.
 *
 * \pre intra_node_comm != MPI_COMM_NULL
 * \pre mesh_metadata != nullptr
 * \pre x == nullptr
 * \pre y == nullptr
 * \pre z == nullptr
 * \pre conn == nullptr
 * \pre mesh_buffer == nullptr
 * \pre shared_window == MPI_WIN_NULL
 *
 * \post x != nullptr
 * \post y != nullptr
 * \post z != nullptr
 * \post coon != nullptr
 * \post mesh_buffer != nullptr
 * \post shared_window != MPI_WIN_NULL
 */
#if defined(AXOM_USE_MPI) && defined(AXOM_USE_MPI3)
MPI_Aint allocate_shared_buffer( int local_rank_id,
                                 MPI_Comm intra_node_comm,
                                 const mint::IndexType mesh_metadata[ 2 ],
                                 double*& x,
                                 double*& y,
                                 double*& z,
                                 mint::IndexType*& conn,
                                 unsigned char*& mesh_buffer,
                                 MPI_Win& shared_window )
{
  constexpr int ROOT_RANK = 0;

  const int nnodes = mesh_metadata[ 0 ];
  const int nfaces = mesh_metadata[ 1 ];

  int disp          = sizeof( unsigned char );
  MPI_Aint bytesize = nnodes * 3 * sizeof( double ) +
                      nfaces * 3 * sizeof( mint::IndexType );
  MPI_Aint window_size = ( local_rank_id != ROOT_RANK ) ? 0 : bytesize;

  MPI_Win_allocate_shared( window_size, disp, MPI_INFO_NULL, intra_node_comm,
                           &mesh_buffer, &shared_window );
  MPI_Win_shared_query( shared_window, ROOT_RANK, &bytesize,
                        &disp, &mesh_buffer );

  // calculate offset to the coordinates & cell connectivity in the buffer
  int baseOffset  = nnodes*sizeof( double );
  int x_offset    = 0;
  int y_offset    = baseOffset;
  int z_offset    = y_offset + baseOffset;
  int conn_offset = z_offset + baseOffset;

  x    = reinterpret_cast< double* >( &mesh_buffer[ x_offset ] );
  y    = reinterpret_cast< double* >( &mesh_buffer[ y_offset ] );
  z    = reinterpret_cast< double* >( &mesh_buffer[ z_offset ] );
  conn = reinterpret_cast< mint::IndexType* >( &mesh_buffer[ conn_offset ] );

  return ( bytesize );
}
#endif

/// @}


/// \name Mesh I/O methods
/// @{

/*!
 * \brief Reads in the surface mesh from the specified file into a shared
 *  memory buffer that is attached to the given MPI shared window.
 *
 * \param [in] file the file consisting of the surface mesh
 * \param [in] global_comm handle to the global MPI communicator
 * \param [out] mesh_buffer pointer to the raw mesh buffer
 * \param [out] m pointer to the mesh object
 * \param [out] intra_node_comm handle to the shared MPI communicator.
 * \param [out] shared_window handle to the MPI shared window.
 *
 * \return status set to READ_SUCCESS, or READ_FAILED on error.
 *
 * \note Each rank has a unique mint::Mesh object instance, however, the
 *  mint::Mesh object is constructed using external pointers that point into
 *  the supplied mesh_buffer, an on-node data-structure shared across all
 *  MPI ranks within the same compute node.
 *
 * \pre global_comm != MPI_COMM_NULL
 * \pre mesh_buffer == nullptr
 * \pre m == nullptr
 * \pre intra_node_comm == MPI_COMM_NULL
 * \pre shared_window == MPI_WIN_NULL
 *
 * \post m != nullptr
 * \post m->isExternal() == true
 * \post mesh_buffer != nullptr
 * \post intra_node_comm != MPI_COMM_NULL
 * \post shared_window != MPI_WIN_NULL
 */
#if defined(AXOM_USE_MPI) && defined(AXOM_USE_MPI3)
int read_mesh_shared( const std::string& file,
                      MPI_Comm global_comm,
                      unsigned char*& mesh_buffer,
                      mint::Mesh*& m,
                      MPI_Comm& intra_node_comm,
                      MPI_Win& shared_window )
{
  SLIC_ASSERT( global_comm != MPI_COMM_NULL );
  SLIC_ASSERT( intra_node_comm == MPI_COMM_NULL );
  SLIC_ASSERT( shared_window == MPI_WIN_NULL );

  // NOTE: STL meshes are always 3D mesh consisting of triangles.
  using TriangleMesh = mint::UnstructuredMesh< mint::SINGLE_SHAPE >;

  // STEP 0: check input mesh pointer
  if ( m != nullptr )
  {
    SLIC_WARNING( "supplied mesh pointer is not null!" );
    return READ_FAILED;
  }

  if ( mesh_buffer != nullptr )
  {
    SLIC_WARNING( "supplied mesh buffer should be null!" );
    return READ_FAILED;
  }

  // STEP 1: create intra-node and inter-node MPI communicators
  int global_rank_id       = -1;
  int local_rank_id        = -1;
  int intercom_rank_id     = -1;
  MPI_Comm inter_node_comm = MPI_COMM_NULL;
  create_communicators( global_comm, intra_node_comm, inter_node_comm,
                        global_rank_id, local_rank_id, intercom_rank_id  );

  // STEP 2: Exchange mesh metadata
  constexpr int NUM_NODES = 0;
  constexpr int NUM_FACES = 1;
  mint::IndexType mesh_metadata[ 2 ]  = { 0, 0 };

  quest::STLReader reader;
  reader.setFileName( file );
  int rc = read_and_exchange_mesh_metadata( global_rank_id, global_comm,
                                            reader, mesh_metadata );
  if( rc != READ_SUCCESS )
  {
    return READ_FAILED;
  }

  // STEP 3: allocate shared buffer and wire pointers
  double* x             = nullptr;
  double* y             = nullptr;
  double* z             = nullptr;
  mint::IndexType* conn = nullptr;
  MPI_Aint numBytes     = allocate_shared_buffer( local_rank_id,
                                                  intra_node_comm,
                                                  mesh_metadata,
                                                  x, y, z, conn,
                                                  mesh_buffer,
                                                  shared_window );
  SLIC_ASSERT( x != nullptr );
  SLIC_ASSERT( y != nullptr );
  SLIC_ASSERT( z != nullptr );
  SLIC_ASSERT( conn != nullptr );

  // STEP 5: allocate corresponding mesh object with external pointers.
  m = new TriangleMesh( mint::TRIANGLE, mesh_metadata[ NUM_FACES ], conn,
                        mesh_metadata[ NUM_NODES ],
                        x, y, z );

  // STEP 4: read in data to shared buffer
  if ( global_rank_id == 0 )
  {
    reader.getMesh( static_cast< TriangleMesh* >( m ) );
  }

  // STEP 5: inter-node communication
  if ( intercom_rank_id >= 0 )
  {
    MPI_Bcast( mesh_buffer, numBytes, MPI_UNSIGNED_CHAR, 0, inter_node_comm );
  }

  // STEP 6 free communicators

  MPI_Barrier( global_comm );
  mpi_comm_free( &inter_node_comm );
  return READ_SUCCESS;
}
#endif


/*!
 * \brief Reads in the surface mesh from the specified file.
 *
 * \param [in] file the file consisting of the surface
 * \param [out] m user-supplied pointer to point to the mesh object.
 * \param [in] comm the MPI communicator, only applicable when MPI is available.
 *
 * \note This method currently expects the surface mesh to be given in STL
 *  format.
 *
 * \note The caller is responsible for properly de-allocating the mesh object
 *  that is returned by this function.
 *
 * \return status set to zero on success, or to a non-zero value otherwise.
 *
 * \pre m == nullptr
 * \pre !file.empty()
 *
 * \post m != nullptr
 * \post m->getMeshType() == mint::UNSTRUCTURED_MESH
 * \post m->hasMixedCellTypes() == false
 * \post m->getCellType() == mint::TRIANGLE
 *
 * \see STLReader
 * \see PSTLReader
 */
int read_mesh( const std::string& file,
               mint::Mesh*& m,
               MPI_Comm comm=MPI_COMM_SELF )
{
  // NOTE: STL meshes are always 3D
  constexpr int DIMENSION = 3;
  using TriangleMesh      = mint::UnstructuredMesh< mint::SINGLE_SHAPE >;

  // STEP 0: check input mesh pointer
  if ( m != nullptr )
  {
    SLIC_WARNING( "supplied mesh pointer is not null!" );
    return READ_FAILED;
  }

  // STEP 1: allocate output mesh object
  m = new TriangleMesh( DIMENSION, mint::TRIANGLE );

  // STEP 2: allocate reader
  quest::STLReader* reader = nullptr;
#ifdef AXOM_USE_MPI
  reader = new quest::PSTLReader( comm );
#else
  static_cast< void >( comm );        // to silence compiler warnings
  reader = new quest::STLReader();
#endif

  // STEP 3: read the mesh from the STL file
  reader->setFileName( file );
  int rc = reader->read( );
  if ( rc == READ_SUCCESS )
  {
    reader->getMesh( static_cast< TriangleMesh* >( m )  );
  }
  else
  {
    SLIC_WARNING( "reading STL file failed, setting mesh to NULL" );
    delete m;
    m = nullptr;
  }

  // STEP 4: delete the reader
  delete reader;
  reader = nullptr;

  return rc;
}

/// @}

/// \name Mesh Helper Methods
/// @{

/*!
 * \brief Computes the bounds of the given mesh.
 *
 * \param [in] mesh pointer to the mesh whose bounds will be computed.
 * \param [out] lo buffer to store the lower bound mesh coordinates
 * \param [out] hi buffer to store the upper bound mesh coordinates
 *
 * \pre mesh != nullptr
 * \pre lo != nullptr
 * \pre hi != nullptr
 * \pre hi & lo must point to buffers that are at least N long, where N
 *  corresponds to the mesh dimension.
 */
void compute_mesh_bounds( const mint::Mesh* mesh, double* lo, double* hi )
{

  SLIC_ASSERT( mesh != nullptr );
  SLIC_ASSERT( lo != nullptr );
  SLIC_ASSERT( hi != nullptr );

  const int ndims = mesh->getDimension();

  // STEP 0: initialize lo,hi
  for ( int i=0 ; i < ndims ; ++i )
  {
    lo[ i ] = std::numeric_limits< double >::max();
    hi[ i ] = std::numeric_limits< double >::lowest();
  } // END for all dimensions

  // STEP 1: compute lo,hi
  double pt[ 3 ];
  const mint::IndexType numNodes = mesh->getNumberOfNodes();
  for ( mint::IndexType inode=0 ; inode < numNodes ; ++inode )
  {
    mesh->getNode( inode, pt );
    for ( int i=0 ; i < ndims ; ++i )
    {
      lo[ i ] = ( pt[ i ] < lo[ i ] ) ? pt[ i ] : lo[ i ];
      hi[ i ] = ( pt[ i ] > hi[ i ] ) ? pt[ i ] : hi[ i ];
    } // END for all dimensions

  } // END for all nodes

}
/// @}

/// \name Logger Initialize/Finalize Methods
/// @{

/*!
 * \brief Helper method to initialize the Slic logger if needed.
 *
 * \param [in,out] isInitialized indicates if Slic is already initialized.
 * \param [out] mustFinalize inidicates if the caller would be responsible
 *  for finalizing the Slic logger.
 * \param [in] verbose flag to control the verbosity
 * \param [in] comm the MPI communicator (applicable when compiled with MPI)
 *
 * \note If Slic is not already initialized, this method will initialize the
 *  Slic Logging environment and set the `isInitialized` flag to true.
 *
 * \note The 'verbose' flag is only applicable when the Slic logging environment
 *  is not already initialized by the calling application. In that case, when
 *  'verbose' is true, all messages will get logged to the console, including,
 *  Info and debug messages. Otherwise, if 'false', only errors will be printed
 *  out.
 *
 *  \see logger_finalize
 */
void logger_init( bool& isInitialized,
                  bool& mustFinalize,
                  bool verbose,
                  MPI_Comm comm )
{
  if ( isInitialized )
  {
    // Query has already initialized the logger
    return;
  }

  if ( slic::isInitialized() )
  {
    // logger is initialized by an application, the application will finalize
    isInitialized = true;
    mustFinalize  = false;
    return;
  }

  // The SignedDistance Query must initialize the Slic logger and is then
  // also responsible for finalizing it when done
  isInitialized = true;
  mustFinalize  = true;
  slic::initialize();

  slic::LogStream* ls = nullptr;
  std::string msgfmt  = "[<LEVEL>]: <MESSAGE>\n";

#if defined(AXOM_USE_MPI) && defined(AXOM_USE_LUMBERJACK)
  constexpr int RLIMIT = 8;
  ls = new slic::LumberjackStream( &std::cout, comm, RLIMIT, msgfmt );
#elif defined(AXOM_USE_MPI) && !defined(AXOM_USE_LUMBERJACK)
  msgfmt.insert( 0, "[<RANK>]", 8 );
  ls = new slic::SynchronizedStream( &std::cout, comm, msgfmt );
#else
  static_cast< void >( comm );        // to silence compiler warnings
  ls = new slic::GenericOutputStream( &std::cout, msgfmt );
#endif

  slic::addStreamToAllMsgLevels( ls );
  slic::setLoggingMsgLevel(
    ( verbose ) ? slic::message::Info : slic::message::Error );
}

/*!
 * \brief Finalizes the Slic logger (if needed)
 *
 * \param [in] mustFinalize flag that indicates whether the query is responsible
 *  for finalizing the Slic logger.
 *
 * \see logger_init
 */
void logger_finalize( bool mustFinalize )
{

  if ( mustFinalize )
  {
    slic::finalize();
  }

}
/// @}

} /* end namespace internal */
} /* end namespace quest    */
} /* end namespace axom     */

#endif /* QUEST_HELPERS_HPP_ */