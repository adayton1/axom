// Copyright (c) 2017-2022, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

/*!
 * \file quest_distributed_distance_query_example.cpp
 * \brief Driver for a distributed distance query
 */

// Axom includes
#include "axom/config.hpp"
#include "axom/core.hpp"
#include "axom/slic.hpp"
#include "axom/primal.hpp"
#include "axom/sidre.hpp"
#include "axom/quest.hpp"
#include "axom/slam.hpp"

#include "conduit_blueprint.hpp"
#include "conduit_blueprint_mpi.hpp"

#include "axom/quest/DistributedClosestPoint.hpp"

#include "axom/fmt.hpp"
#include "axom/CLI11.hpp"

#ifndef AXOM_USE_MFEM
  #error This example requires Axom to be configured with MFEM and the AXOM_ENABLE_MFEM_SIDRE_DATACOLLECTION option
#endif
#include "mfem.hpp"

#ifndef AXOM_USE_MPI
  #error This example requires Axom to be configured with MPI
#endif
#include "mpi.h"

// C/C++ includes
#include <string>
#include <limits>
#include <map>
#include <cmath>

namespace quest = axom::quest;
namespace slic = axom::slic;
namespace sidre = axom::sidre;
namespace slam = axom::slam;
namespace spin = axom::spin;
namespace primal = axom::primal;
namespace mint = axom::mint;
namespace numerics = axom::numerics;

using RuntimePolicy = axom::quest::DistributedClosestPoint::RuntimePolicy;

/// Struct to parse and store the input parameters
struct Input
{
public:
  std::string meshFile;
  std::string distanceFile {"closest_point"};
  std::string objectFile {"object_mesh"};

  double circleRadius {1.0};
  std::vector<double> circleCenter {0.0, 0.0};
  // TODO: Ensure that circleCenter size matches dimensionality.
  int circlePoints {100};
  RuntimePolicy policy {RuntimePolicy::seq};

  double distThreshold {std::numeric_limits<double>::max()};

private:
  bool m_verboseOutput {false};
  double m_emptyRankProbability {0.};

  // clang-format off
  const std::map<std::string, RuntimePolicy> s_validPolicies
  {
      {"seq", RuntimePolicy::seq}
#if defined(AXOM_USE_RAJA) && defined(AXOM_USE_UMPIRE)
  #ifdef AXOM_USE_OPENMP
    , {"omp", RuntimePolicy::omp}
  #endif
  #ifdef AXOM_USE_CUDA
    , {"cuda", RuntimePolicy::cuda}
  #endif
  #ifdef AXOM_USE_HIP
    , {"hip", RuntimePolicy::hip}
  #endif
#endif
  };
  // clang-format on

public:
  bool isVerbose() const { return m_verboseOutput; }
  double percentEmptyRanks() const { return m_emptyRankProbability; }

  std::string getDCMeshName() const
  {
    using axom::utilities::string::removeSuffix;

    // Remove the parent directories and file suffix
    std::string name = axom::Path(meshFile).baseName();
    name = removeSuffix(name, ".root");

    return name;
  }

  void parse(int argc, char** argv, axom::CLI::App& app)
  {
    app.add_option("-m,--mesh-file", meshFile)
      ->description(
        "Path to computational mesh (generated by MFEMSidreDataCollection)")
      ->check(axom::CLI::ExistingFile)
      ->required();

    app.add_option("-s,--distance-file", distanceFile)
      ->description("Name of output mesh file containing closest distance.")
      ->capture_default_str();

    app.add_option("-o,--object-file", objectFile)
      ->description("Name of output file containing object mesh.")
      ->capture_default_str();

    app.add_flag("-v,--verbose,!--no-verbose", m_verboseOutput)
      ->description("Enable/disable verbose output")
      ->capture_default_str();

    app.add_option("--empty-rank-probability", m_emptyRankProbability)
      ->description(
        "Probability that a rank's data is empty "
        "(tests code's ability to handle empty ranks)")
      ->check(axom::CLI::Range(0., 1.))
      ->capture_default_str();

    app.add_option("-r,--radius", circleRadius)
      ->description("Radius for circle")
      ->capture_default_str();

    auto* circle_options =
      app.add_option_group("circle",
                           "Options for setting up the circle of points");
    circle_options->add_option("--center", circleCenter)
      ->description("Center for object (x,y[,z])")
      ->expected(2, 3);

    app.add_option("-d,--dist-threshold", distThreshold)
      ->check(axom::CLI::NonNegativeNumber)
      ->description("Distance threshold to search")
      ->capture_default_str();

    app.add_option("-n,--num-samples", circlePoints)
      ->description("Number of points for circle")
      ->capture_default_str();

    app.add_option("-p, --policy", policy)
      ->description("Set runtime policy for point query method")
      ->capture_default_str()
      ->transform(axom::CLI::CheckedTransformer(s_validPolicies));

    app.get_formatter()->column_width(60);

    // could throw an exception
    app.parse(argc, argv);

    slic::setLoggingMsgLevel(m_verboseOutput ? slic::message::Debug
                                             : slic::message::Info);
  }
};

/**
 *  \brief Simple wrapper to a blueprint particle mesh
 *
 *  Given a sidre Group, creates the stubs for a mesh blueptint particle mesh
 */
struct BlueprintParticleMesh
{
public:
  explicit BlueprintParticleMesh(sidre::Group* group = nullptr,
                                 const std::string& coordset = "coords",
                                 const std::string& topology = "mesh")
    : m_group(group)
  {
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_nranks);

    setBlueprintGroup(m_group, coordset, topology);
  }
  /// Gets the root group for this mesh blueprint
  sidre::Group* rootGroup() const { return m_group; }
  /// Gets the parent group for the blueprint coordinate set
  sidre::Group* coordsGroup() const { return m_coordsGroup; }
  /// Gets the parent group for the blueprint mesh topology
  sidre::Group* topoGroup() const { return m_topoGroup; }

  /// Gets the MPI rank for this mesh
  int getRank() const { return m_rank; }
  /// Gets the number of ranks in the problem
  int getNumRanks() const { return m_nranks; }

  /// Returns true if points have been added to the particle mesh
  bool hasPoints() const
  {
    return m_coordsGroup != nullptr && m_coordsGroup->hasView("values/x");
  }

  /// Returns the number of points in the particle mesh
  int numPoints() const
  {
    return hasPoints() ? m_coordsGroup->getView("values/x")->getNumElements() : 0;
  }

  int dimension() const { return m_dimension; }

  /**
   * Sets the parent group for the entire mesh and sets up the blueprint stubs
   * for the "coordset", "topologies", "fields" and "state"
   */
  void setBlueprintGroup(sidre::Group* group,
                         const std::string& coordset = "coords",
                         const std::string& topology = "mesh")
  {
    // TODO: Ensure that we delete previous hierarchy if it existed

    m_group = group;

    if(m_group != nullptr)
    {
      createBlueprintStubs(coordset, topology);
    }
  }

  /// Set the coordinate data from an array of primal Points, templated on the dimension
  template <int NDIMS>
  void setPoints(const axom::Array<primal::Point<double, NDIMS>>& pts)
  {
    SLIC_ASSERT_MSG(m_group != nullptr,
                    "Must set blueprint group before setPoints()");

    const int SZ = pts.size();

    m_dimension = NDIMS;

    // lamda to create a strided view into the buffer
    // uses workaround for empty meshes since apply() requires size > 0
    auto createAndApplyView = [=](sidre::Group* grp,
                                  const std::string& path,
                                  sidre::Buffer* buf,
                                  int dim,
                                  int sz) {
      if(sz > 0)
      {
        grp->createView(path)->attachBuffer(buf)->apply(sz, dim, NDIMS);
      }
      else
      {
        grp->createViewAndAllocate(path, sidre::DOUBLE_ID, 0);
      }
    };

    // create views into a shared buffer for the coordinates, with stride NDIMS
    {
      auto* buf = m_group->getDataStore()
                    ->createBuffer(sidre::DOUBLE_ID, NDIMS * SZ)
                    ->allocate();

      createAndApplyView(m_coordsGroup, "values/x", buf, 0, SZ);
      if(NDIMS > 1)
      {
        createAndApplyView(m_coordsGroup, "values/y", buf, 1, SZ);
      }
      if(NDIMS > 2)
      {
        createAndApplyView(m_coordsGroup, "values/z", buf, 2, SZ);
      }

      // copy coordinate data into the buffer
      const std::size_t nbytes = sizeof(double) * SZ * NDIMS;
      axom::copy(buf->getVoidPtr(), pts.data(), nbytes);
    }

    // set the default connectivity
    sidre::Array<int> arr(m_topoGroup->createView("elements/connectivity"), SZ, SZ);
    for(int i = 0; i < SZ; ++i)
    {
      arr[i] = i;
    }
  }

  template <typename T>
  void registerNodalScalarField(const std::string& fieldName)
  {
    SLIC_ASSERT_MSG(hasPoints(),
                    "Cannot register a field with the BlueprintParticleMesh "
                    "before adding points");

    auto* fld = m_fieldsGroup->createGroup(fieldName);
    fld->createViewString("association", "vertex");
    fld->createViewString("topology", m_topoGroup->getName());
    fld->createViewAndAllocate("values",
                               sidre::detail::SidreTT<T>::id,
                               numPoints());
  }

  template <typename T>
  void registerNodalVectorField(const std::string& fieldName)
  {
    SLIC_ASSERT_MSG(hasPoints(),
                    "Cannot register a field with the BlueprintParticleMesh "
                    "before adding points");

    const int SZ = numPoints();
    const int DIM = dimension();

    auto* fld = m_fieldsGroup->createGroup(fieldName);
    fld->createViewString("association", "vertex");
    fld->createViewString("topology", m_topoGroup->getName());

    // create views into a shared buffer for the coordinates, with stride NDIMS
    auto* buf = m_group->getDataStore()
                  ->createBuffer(sidre::detail::SidreTT<T>::id, DIM * SZ)
                  ->allocate();
    switch(DIM)
    {
    case 3:
      fld->createView("values/x")->attachBuffer(buf)->apply(SZ, 0, DIM);
      fld->createView("values/y")->attachBuffer(buf)->apply(SZ, 1, DIM);
      fld->createView("values/z")->attachBuffer(buf)->apply(SZ, 2, DIM);
      break;
    case 2:
      fld->createView("values/x")->attachBuffer(buf)->apply(SZ, 0, DIM);
      fld->createView("values/y")->attachBuffer(buf)->apply(SZ, 1, DIM);
      break;
    default:
      fld->createView("values/x")->attachBuffer(buf)->apply(SZ, 0, DIM);
      break;
    }
  }

  bool hasField(const std::string& fieldName) const
  {
    return m_fieldsGroup->hasGroup(fieldName);
  }

  template <typename T>
  axom::ArrayView<T> getNodalScalarField(const std::string& fieldName) const
  {
    SLIC_ASSERT_MSG(hasPoints(),
                    "Cannot extract a field from the BlueprintParticleMesh "
                    "before adding points");

    T* data = hasField(fieldName)
      ? static_cast<T*>(
          m_fieldsGroup->getView(axom::fmt::format("{}/values", fieldName))
            ->getVoidPtr())
      : nullptr;

    return axom::ArrayView<T>(data, numPoints());
  }

  template <typename T>
  axom::ArrayView<T> getNodalVectorField(const std::string& fieldName) const
  {
    SLIC_ASSERT_MSG(hasPoints(),
                    "Cannot extract a field from the BlueprintParticleMesh "
                    "before adding points");

    // Note: the implementation currently assumes that the field data is
    // interleaved, so it is safe to get a pointer to the beginning of the
    // x-coordinate's data. This will be relaxed in the future, and we will
    // need to modify this implementation accordingly.
    T* data = hasField(fieldName)
      ? static_cast<T*>(
          m_fieldsGroup->getView(axom::fmt::format("{}/values/x", fieldName))
            ->getVoidPtr())
      : nullptr;

    return axom::ArrayView<T>(data, numPoints());
  }

  /// Checks whether the blueprint is valid and prints diagnostics
  bool isValid() const
  {
    conduit::Node mesh_node;

    // use an empty conduit node for meshes with 0 elements
    if(numPoints() > 0)
    {
      m_group->createNativeLayout(mesh_node);
    }

    bool success = true;
    conduit::Node info;
    if(!conduit::blueprint::mpi::verify("mesh", mesh_node, info, MPI_COMM_WORLD))
    {
      SLIC_INFO("Invalid blueprint for particle mesh: \n" << info.to_yaml());
      success = false;
    }

    return success;
  }

  /// Outputs the object mesh to disk
  void saveMesh(const std::string& outputMesh)
  {
    auto* ds = m_group->getDataStore();
    sidre::IOManager writer(MPI_COMM_WORLD);
    writer.write(ds->getRoot(), m_nranks, outputMesh, "sidre_hdf5");

    MPI_Barrier(MPI_COMM_WORLD);

    // Add the bp index to the root file
    writer.writeBlueprintIndexToRootFile(m_group->getDataStore(),
                                         m_group->getPathName(),
                                         outputMesh + ".root",
                                         m_group->getName());
  }

private:
  /// Creates blueprint stubs for this mesh
  void createBlueprintStubs(const std::string& coords, const std::string& topo)
  {
    SLIC_ASSERT(m_group != nullptr);

    m_coordsGroup = m_group->createGroup("coordsets")->createGroup(coords);
    m_coordsGroup->createViewString("type", "explicit");
    m_coordsGroup->createGroup("values");

    m_topoGroup = m_group->createGroup("topologies")->createGroup(topo);
    m_topoGroup->createViewString("coordset", coords);
    m_topoGroup->createViewString("type", "unstructured");
    m_topoGroup->createViewString("elements/shape", "point");

    m_fieldsGroup = m_group->createGroup("fields");

    m_group->createViewScalar<axom::int64>("state/domain_id", m_rank);
  }

private:
  sidre::Group* m_group;

  sidre::Group* m_coordsGroup;
  sidre::Group* m_topoGroup;
  sidre::Group* m_fieldsGroup;

  int m_rank;
  int m_nranks;
  int m_dimension {-1};
};

/**
 * Helper class to generate a mesh blueprint-conforming particle mesh for the input object.
 * The mesh is represented using a Sidre hierarchy
 */
class ObjectMeshWrapper
{
public:
  ObjectMeshWrapper(sidre::Group* group) : m_group(group), m_mesh(m_group)
  {
    SLIC_ASSERT(m_group != nullptr);
  }

  /// Get a pointer to the root group for this mesh
  sidre::Group* getBlueprintGroup() const { return m_group; }

  std::string getCoordsetName() const
  {
    return m_mesh.coordsGroup()->getName();
  }

  int numPoints() const { return m_mesh.numPoints(); }

  /**
   * Generates a collection of \a numPoints points along a circle
   * of radius \a radius centered at the origin
   */
  void generateCircleMesh(double radius,
                          std::vector<double>& center,
                          bool rankHasPoints,
                          int totalNumPoints)
  {
    using axom::utilities::random_real;

    // Check that we're starting with a valid group
    SLIC_ASSERT(m_group != nullptr);

    constexpr int DIM = 2;
    using PointType = primal::Point<double, DIM>;
    using PointArray = axom::Array<PointType>;

    // compute start and stop angle, allowing for some empty ranks
    double thetaStart, thetaEnd;
    int numPoints = 0;

    // perform scan on ranks to compute numPoints, thetaStart and thetaEnd
    {
      int hasPoints = rankHasPoints ? 1 : 0;
      int myRank = m_mesh.getRank();
      int numRanks = m_mesh.getNumRanks();

      axom::Array<int> arr(numRanks, numRanks);
      arr.fill(-1);
      MPI_Allgather(&hasPoints, 1, MPI_INT, arr.data(), 1, MPI_INT, MPI_COMM_WORLD);

      SLIC_DEBUG(
        axom::fmt::format("After all gather: [{}]", axom::fmt::join(arr, ",")));

      axom::Array<int> sums(numRanks + 1, numRanks + 1);
      sums[0] = 0;
      for(int i = 1; i <= numRanks; ++i)
      {
        sums[i] = sums[i - 1] + arr[i - 1];
      }

      SLIC_DEBUG(
        axom::fmt::format("After scan: [{}]", axom::fmt::join(sums, ",")));

      const int numNonEmpty = sums[numRanks];

      if(numNonEmpty > 0)
      {
        const double thetaScale = 2. * M_PI / numNonEmpty;
        thetaStart = sums[myRank] * thetaScale;
        thetaEnd = sums[myRank + 1] * thetaScale;
        numPoints = rankHasPoints ? totalNumPoints / numNonEmpty : 0;
      }
      else
      {
        if(myRank < numRanks - 1)
        {
          thetaStart = 0.;
          thetaEnd = 0.;
          numPoints = 0;
        }
        else
        {
          thetaStart = 0.;
          thetaEnd = 2. * M_PI;
          numPoints = totalNumPoints;
        }
      }

      SLIC_DEBUG(
        axom::fmt::format("Rank {}, start angle {}, stop angle {}, num "
                          "non-empty {}, num points {}",
                          myRank,
                          thetaStart,
                          thetaEnd,
                          numNonEmpty,
                          numPoints));

      axom::slic::flushStreams();
    }

    PointArray pts(0, numPoints);

    for(int i = 0; i < numPoints; ++i)
    {
      const double angleInRadians =
        (thetaStart < thetaEnd) ? random_real(thetaStart, thetaEnd) : thetaStart;
      const double rsinT = center[1] + radius * std::sin(angleInRadians);
      const double rcosT = center[0] + radius * std::cos(angleInRadians);

      pts.push_back(PointType {rcosT, rsinT});
    }

    m_mesh.setPoints(pts);

    SLIC_ASSERT(m_mesh.isValid());
  }

  /// Outputs the object mesh to disk
  void saveMesh(const std::string& outputMesh = "object_mesh")
  {
    SLIC_INFO(axom::fmt::format(
      "{:=^80}",
      axom::fmt::format("Saving particle mesh '{}' to disk", outputMesh)));

    m_mesh.saveMesh(outputMesh);
  }

private:
  sidre::Group* m_group;
  BlueprintParticleMesh m_mesh;
};

class QueryMeshWrapper
{
public:
  QueryMeshWrapper(const std::string& cpFilename = "closest_point")
    : m_dc(cpFilename, nullptr, true)
  { }

  // Returns a pointer to the MFEMSidreDataCollection
  sidre::MFEMSidreDataCollection* getDC() { return &m_dc; }
  const sidre::MFEMSidreDataCollection* getDC() const { return &m_dc; }

  const BlueprintParticleMesh& getParticleMesh() const { return m_queryMesh; }

  sidre::Group* getBlueprintGroup() const { return m_queryMesh.rootGroup(); }

  std::string getCoordsetName() const
  {
    return m_queryMesh.coordsGroup()->getName();
  }

  /// Returns an array containing the positions of the mesh vertices
  template <typename PointArray>
  PointArray getVertexPositions()
  {
    auto* mesh = m_dc.GetMesh();
    const int NV = mesh->GetNV();
    PointArray arr(NV);

    for(auto i : slam::PositionSet<int>(NV))
    {
      mesh->GetNode(i, arr[i].data());
    }

    return arr;
  }

  /// Saves the data collection to disk
  void saveMesh()
  {
    SLIC_INFO(
      axom::fmt::format("{:=^80}",
                        axom::fmt::format("Saving query mesh '{}' to disk",
                                          m_dc.GetCollectionName())));

    m_dc.Save();
  }

  void setupParticleMesh()
  {
    using PointArray2D = axom::Array<primal::Point<double, 2>>;
    using PointArray3D = axom::Array<primal::Point<double, 3>>;

    auto* dsRoot = m_dc.GetBPGroup()->getDataStore()->getRoot();
    m_queryMesh = BlueprintParticleMesh(dsRoot->createGroup("query_mesh"));

    const int DIM = m_dc.GetMesh()->Dimension();
    SLIC_ERROR_IF(DIM != 2 && DIM != 3,
                  "Only 2D and 3D meshes are supported in setupParticleMesh(). "
                  "Attempted mesh dimension was "
                    << DIM);

    switch(DIM)
    {
    case 2:
      m_queryMesh.setPoints<2>(getVertexPositions<PointArray2D>());
      break;
    case 3:
      m_queryMesh.setPoints<3>(getVertexPositions<PointArray3D>());
      break;
    }

    m_queryMesh.registerNodalScalarField<axom::IndexType>("cp_rank");
    m_queryMesh.registerNodalScalarField<axom::IndexType>("cp_index");
    m_queryMesh.registerNodalScalarField<double>("min_distance");
    m_queryMesh.registerNodalVectorField<double>("closest_point");

    SLIC_ASSERT(m_queryMesh.isValid());
  }

  /**
   * Loads the mesh as an MFEMSidreDataCollection with
   * the following fields: "positions", "distances", "directions"
   */
  void setupMesh(const std::string& fileName, const std::string meshFile)
  {
    SLIC_INFO(axom::fmt::format("{:=^80}",
                                axom::fmt::format("Loading '{}' mesh", fileName)));

    sidre::MFEMSidreDataCollection originalMeshDC(fileName, nullptr, false);
    {
      originalMeshDC.SetComm(MPI_COMM_WORLD);
      originalMeshDC.Load(meshFile, "sidre_hdf5");
    }
    SLIC_ASSERT_MSG(originalMeshDC.GetMesh()->Dimension() == 2,
                    "This application currently only supports 2D meshes");
    // TODO: Check order and apply LOR, if necessary

    const int DIM = originalMeshDC.GetMesh()->Dimension();

    // Create the data collection
    mfem::Mesh* cpMesh = nullptr;
    {
      m_dc.SetMeshNodesName("positions");

      auto* pmesh = dynamic_cast<mfem::ParMesh*>(originalMeshDC.GetMesh());
      cpMesh = (pmesh != nullptr) ? new mfem::ParMesh(*pmesh)
                                  : new mfem::Mesh(*originalMeshDC.GetMesh());
      m_dc.SetMesh(cpMesh);
    }

    // Register the distance and direction grid function
    constexpr int order = 1;
    auto* fec = new mfem::H1_FECollection(order, DIM, mfem::BasisType::Positive);
    mfem::FiniteElementSpace* fes = new mfem::FiniteElementSpace(cpMesh, fec);
    mfem::GridFunction* distances = new mfem::GridFunction(fes);
    distances->MakeOwner(fec);
    m_dc.RegisterField("distance", distances);

    auto* vfec = new mfem::H1_FECollection(order, DIM, mfem::BasisType::Positive);
    mfem::FiniteElementSpace* vfes =
      new mfem::FiniteElementSpace(cpMesh, vfec, DIM);
    mfem::GridFunction* directions = new mfem::GridFunction(vfes);
    directions->MakeOwner(vfec);
    m_dc.RegisterField("direction", directions);
  }

  /// Prints some info about the mesh
  void printMeshInfo()
  {
    switch(m_dc.GetMesh()->Dimension())
    {
    case 2:
      printMeshInfo<2>();
      break;
    case 3:
      printMeshInfo<3>();
      break;
    }
  }

private:
  /**
  * \brief Print some info about the mesh
  *
  * \note In MPI-based configurations, this is a collective call, but only prints on rank 0
  */
  template <int DIM>
  void printMeshInfo()
  {
    mfem::Mesh* mesh = m_dc.GetMesh();

    int myRank = 0;
    int numElements = mesh->GetNE();

    mfem::Vector mins, maxs;

    auto* pmesh = dynamic_cast<mfem::ParMesh*>(mesh);
    if(pmesh != nullptr)
    {
      pmesh->GetBoundingBox(mins, maxs);
      numElements = pmesh->ReduceInt(numElements);
      myRank = pmesh->GetMyRank();
    }
    else
    {
      mesh->GetBoundingBox(mins, maxs);
    }

    if(myRank == 0)
    {
      SLIC_INFO(axom::fmt::format(
        "Mesh has {} elements and (approximate) bounding box {}",
        numElements,
        primal::BoundingBox<double, DIM>(
          primal::Point<double, DIM>(mins.GetData()),
          primal::Point<double, DIM>(maxs.GetData()))));
    }

    slic::flushStreams();
  }

private:
  sidre::MFEMSidreDataCollection m_dc;
  BlueprintParticleMesh m_queryMesh;
};

/// Utility function to initialize the logger
void initializeLogger()
{
  // Initialize Logger
  slic::initialize();
  slic::setLoggingMsgLevel(slic::message::Info);

  slic::LogStream* logStream;

#ifdef AXOM_USE_MPI
  std::string fmt = "[<RANK>][<LEVEL>]: <MESSAGE>\n";
  #ifdef AXOM_USE_LUMBERJACK
  const int RLIMIT = 8;
  logStream = new slic::LumberjackStream(&std::cout, MPI_COMM_WORLD, RLIMIT, fmt);
  #else
  logStream = new slic::SynchronizedStream(&std::cout, MPI_COMM_WORLD, fmt);
  #endif
#else
  std::string fmt = "[<LEVEL>]: <MESSAGE>\n";
  logStream = new slic::GenericOutputStream(&std::cout, fmt);
#endif  // AXOM_USE_MPI

  slic::addStreamToAllMsgLevels(logStream);
}

/// Utility function to finalize the logger
void finalizeLogger()
{
  if(slic::isInitialized())
  {
    slic::flushStreams();
    slic::finalize();
  }
}

//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  int my_rank, num_ranks;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

  initializeLogger();
  //slic::setAbortOnWarning(true);

  //---------------------------------------------------------------------------
  // Set up and parse command line arguments
  //---------------------------------------------------------------------------
  Input params;
  axom::CLI::App app {"Driver for distributed distance query"};

  try
  {
    params.parse(argc, argv, app);
  }
  catch(const axom::CLI::ParseError& e)
  {
    int retval = -1;
    if(my_rank == 0)
    {
      retval = app.exit(e);
    }

    MPI_Bcast(&retval, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();

    exit(retval);
  }

  constexpr int DIM = 2;

  using PointType = primal::Point<double, DIM>;
  using PointArray = axom::Array<PointType>;
  using IndexSet = slam::PositionSet<>;

  //---------------------------------------------------------------------------
  // Load/generate object mesh
  //---------------------------------------------------------------------------
  sidre::DataStore objectDS;
  ObjectMeshWrapper object_mesh_wrapper(
    objectDS.getRoot()->createGroup("object_mesh"));

  const double prob = axom::utilities::random_real(0., 1.);
  const bool rankHasPoints = prob < (1. - params.percentEmptyRanks());
  object_mesh_wrapper.generateCircleMesh(params.circleRadius,
                                         params.circleCenter,
                                         rankHasPoints,
                                         params.circlePoints);

  SLIC_INFO(axom::fmt::format("Object mesh has {} points",
                              object_mesh_wrapper.numPoints()));
  slic::flushStreams();

  object_mesh_wrapper.saveMesh(params.objectFile);

  //---------------------------------------------------------------------------
  // Load computational mesh and generate a particle mesh over its nodes
  // These will be used to query the closest points on the object mesh(es)
  //---------------------------------------------------------------------------
  QueryMeshWrapper query_mesh_wrapper(params.distanceFile);

  query_mesh_wrapper.setupMesh(params.getDCMeshName(), params.meshFile);
  query_mesh_wrapper.printMeshInfo();
  query_mesh_wrapper.setupParticleMesh();

  // Copy mesh nodes into qpts array
  auto qPts = query_mesh_wrapper.getVertexPositions<PointArray>();
  const int nQueryPts = qPts.size();

  //---------------------------------------------------------------------------
  // Initialize spatial index for querying points, and run query
  //---------------------------------------------------------------------------

  auto init_str =
    axom::fmt::format("{:=^80}",
                      axom::fmt::format("Initializing BVH tree over {} points",
                                        params.circlePoints));

  auto query_str = axom::fmt::format(
    "{:=^80}",
    axom::fmt::format("Computing closest points for {} query points", nQueryPts));

  axom::utilities::Timer initTimer(false);
  axom::utilities::Timer queryTimer(false);

  // Convert blueprint representation from sidre to conduit
  conduit::Node object_mesh_node;
  if(object_mesh_wrapper.numPoints() > 0)
  {
    object_mesh_wrapper.getBlueprintGroup()->createNativeLayout(object_mesh_node);
  }

  // Put sidre data into Conduit Node query_mesh_node.
  conduit::Node query_mesh_node;
  query_mesh_wrapper.getBlueprintGroup()->createNativeLayout(query_mesh_node);
  query_mesh_node.fetch("fields/min_distance/values");

  // Create distributed closest point query object and set some parameters
  quest::DistributedClosestPoint query;
  query.setRuntimePolicy(params.policy);
  query.setDimension(DIM);
  query.setVerbosity(params.isVerbose());
  query.setDistanceThreshold(params.distThreshold);
  query.setObjectMesh(object_mesh_node, object_mesh_wrapper.getCoordsetName());

  // Build the spatial index over the object on each rank
  SLIC_INFO(init_str);
  initTimer.start();
  query.generateBVHTree();
  initTimer.stop();

  // Run the distributed closest point query over the nodes of the computational mesh
  SLIC_INFO(query_str);
  queryTimer.start();
  query.computeClosestPoints(query_mesh_node,
                             query_mesh_wrapper.getCoordsetName());
  queryTimer.stop();

  auto getMinMax =
    [](double inVal, double& minVal, double& maxVal, double& sumVal) {
      MPI_Allreduce(&inVal, &minVal, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
      MPI_Allreduce(&inVal, &maxVal, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      MPI_Allreduce(&inVal, &sumVal, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    };

  // Output some timing stats
  {
    double minInit, maxInit, sumInit;
    getMinMax(initTimer.elapsedTimeInSec(), minInit, maxInit, sumInit);

    double minQuery, maxQuery, sumQuery;
    getMinMax(queryTimer.elapsedTimeInSec(), minQuery, maxQuery, sumQuery);

    SLIC_INFO(axom::fmt::format(
      "Initialization with policy {} took {{avg:{}, min:{}, max:{}}} seconds",
      params.policy,
      sumInit / num_ranks,
      minInit,
      maxInit));
    SLIC_INFO(axom::fmt::format(
      "Query with policy {} took {{avg:{}, min:{}, max:{}}} seconds",
      params.policy,
      sumQuery / num_ranks,
      minQuery,
      maxQuery));
  }

  auto cpPositions =
    query_mesh_wrapper.getParticleMesh().getNodalVectorField<PointType>(
      "closest_point");

  auto cpIndices =
    query_mesh_wrapper.getParticleMesh().getNodalScalarField<axom::IndexType>(
      "cp_index");

  if(params.isVerbose())
  {
    auto cpRank =
      query_mesh_wrapper.getParticleMesh().getNodalScalarField<axom::IndexType>(
        "cp_rank");

    SLIC_INFO(axom::fmt::format("Closest points ({}):", cpPositions.size()));
    for(auto i : IndexSet(cpPositions.size()))
    {
      SLIC_INFO(axom::fmt::format("\t{}: {{rank:{}, index:{}, position:{}}}",
                                  i,
                                  cpRank[i],
                                  cpIndices[i],
                                  cpPositions[i]));
    }
  }

  //---------------------------------------------------------------------------
  // Transform closest points to distances and directions
  //---------------------------------------------------------------------------
  using primal::squared_distance;

  auto* distances = query_mesh_wrapper.getDC()->GetField("distance");
  auto* directions = query_mesh_wrapper.getDC()->GetField("direction");

  // Output some stats about the per-rank query points
  {
    double minPts, maxPts, sumPts;
    getMinMax(distances->Size(), minPts, maxPts, sumPts);
    SLIC_INFO(
      axom::fmt::format(" Query points: {{total:{}, min:{}, max:{}, avg:{}}}",
                        sumPts,
                        minPts,
                        maxPts,
                        sumPts / num_ranks));
  }

  mfem::Array<int> dofs;
  const PointType nowhere(std::numeric_limits<double>::signaling_NaN());
  const double nodist = std::numeric_limits<double>::signaling_NaN();
  for(auto idx : IndexSet(nQueryPts))
  {
    const auto& cp = cpIndices[idx] >= 0 ? cpPositions[idx] : nowhere;
    (*distances)(idx) =
      cpIndices[idx] >= 0 ? sqrt(squared_distance(qPts[idx], cp)) : nodist;
    primal::Vector<double, DIM> dir(qPts[idx], cp);
    directions->FESpace()->GetVertexVDofs(idx, dofs);
    directions->SetSubVector(dofs, dir.data());
  }

  //---------------------------------------------------------------------------
  // Cleanup, save mesh/fields and exit
  //---------------------------------------------------------------------------
  query_mesh_wrapper.saveMesh();

  finalizeLogger();
  MPI_Finalize();

  return 0;
}
