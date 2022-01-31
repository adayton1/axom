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

#include "axom/quest/DistributedClosestPoint.hpp"

#include "axom/fmt.hpp"
#include "axom/CLI11.hpp"

#ifndef AXOM_USE_MFEM
  #error This example requires Axom to be configured with MFEM and the AXOM_ENABLE_MFEM_SIDRE_DATACOLLECTION option
#endif
#include "mfem.hpp"

#ifdef AXOM_USE_MPI
  #include "mpi.h"
#endif

// RAJA
#ifdef AXOM_USE_RAJA
  #include "RAJA/RAJA.hpp"
#endif

// C/C++ includes
#include <string>
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

/** 
 * Helper class to generate a mesh blueprint-conforming particle mesh for the input object.
 * The mesh is represented using a Sidre hierarchy
 */
class ObjectMeshWrapper
{
public:
  ObjectMeshWrapper(sidre::Group* group) : m_group(group), m_mesh(nullptr)
  {
#ifdef AXOM_USE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &m_nranks);
#else
    m_rank = 0;
    m_nranks = 1;
#endif

    SLIC_ASSERT(m_group != nullptr);
  }

  ~ObjectMeshWrapper() { delete m_mesh; }

  /// Get a pointer to the root group for this mesh
  sidre::Group* getBlueprintGroup() const { return m_group; }

  /** 
   * Generates a collection of \a numPoints points along a circle
   * of radius \a radius centered at the origin
   */
  void generateCircleMesh(double radius, int numPoints)
  {
    using axom::utilities::random_real;

    // Check that we're starting with a valid group
    SLIC_ASSERT(m_group != nullptr);
    SLIC_ASSERT(m_group->getNumGroups() == 0);

    constexpr int DIM = 2;
    m_mesh = new mint::ParticleMesh(DIM, 0, m_group, "mesh", "coords", numPoints);

    for(int i = 0; i < numPoints; ++i)
    {
      const double angleInRadians = random_real(0., 2 * M_PI);
      const double rsinT = radius * std::sin(angleInRadians);
      const double rcosT = radius * std::cos(angleInRadians);

      m_mesh->append(rcosT, rsinT);
    }
  }

  /// Outputs the object mesh to disk
  void saveMesh(const std::string& outputMesh = "object_mesh")
  {
    SLIC_INFO(axom::fmt::format(
      "{:=^80}",
      axom::fmt::format("Saving particle mesh '{}' to disk", outputMesh)));

    const bool use_blueprint_writer = false;
    if(use_blueprint_writer)
    {
      // Note: I encountered the following problem with the Visit blueprint plugin
      // when using the implicit point topology:
      //    Encountered unknown topology type, "points"
      //    Skipping this mesh for now
      // It is either not yet supported, or I'm writing this out incorrectly

      auto* ds = m_group->getDataStore();
      sidre::IOManager writer(MPI_COMM_WORLD);
      writer.write(ds->getRoot(), 1, outputMesh, "sidre_hdf5");

      MPI_Barrier(MPI_COMM_WORLD);

      // Add the bp index to the root file
      writer.writeBlueprintIndexToRootFile(m_group->getDataStore(),
                                           m_group->getPathName(),
                                           outputMesh + ".root",
                                           m_group->getName());
    }
    else  // use mint's vtk writer
    {
      auto filename = m_nranks > 1
        ? axom::fmt::format("{}_{:04}.vtk", outputMesh, m_rank)
        : axom::fmt::format("{}.vtk", outputMesh);

      mint::write_vtk(m_mesh, filename);
    }
  }

private:
  sidre::Group* m_group;
  mint::ParticleMesh* m_mesh;
  int m_rank;
  int m_nranks;
};

class QueryMeshWrapper
{
public:
  QueryMeshWrapper() : m_dc("closest_point", nullptr, true) { }

  // Returns a pointer to the MFEMSidreDataCollection
  sidre::MFEMSidreDataCollection* getDC() { return &m_dc; }
  const sidre::MFEMSidreDataCollection* getDC() const { return &m_dc; }

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
#ifdef MFEM_USE_MPI
    SLIC_INFO(
      axom::fmt::format("{:=^80}",
                        axom::fmt::format("Saving query mesh '{}' to disk",
                                          m_dc.GetCollectionName())));

    m_dc.Save();
#endif
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
#ifdef MFEM_USE_MPI
    auto* pmesh = dynamic_cast<mfem::ParMesh*>(mesh);
    if(pmesh != nullptr)
    {
      pmesh->GetBoundingBox(mins, maxs);
      numElements = pmesh->ReduceInt(numElements);
      myRank = pmesh->GetMyRank();
    }
    else
#endif
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
};

/// Choose runtime policy for RAJA
enum class RuntimePolicy
{
  seq = 0,
  omp = 1,
  cuda = 2
};

/// Struct to parse and store the input parameters
struct Input
{
public:
  std::string meshFile;

  double circleRadius {1.0};
  int circlePoints {100};
  RuntimePolicy policy {RuntimePolicy::seq};

private:
  bool m_verboseOutput {false};

  // clang-format off
  const std::map<std::string, RuntimePolicy> s_validPolicies{
    #if defined(AXOM_USE_RAJA) && defined(AXOM_USE_UMPIRE)
      {"seq", RuntimePolicy::seq}
      #ifdef AXOM_USE_OPENMP
    , {"omp", RuntimePolicy::omp}
      #endif
      #ifdef AXOM_USE_CUDA
    , {"cuda", RuntimePolicy::cuda}
      #endif
    #endif
  };
  // clang-format on

public:
  bool isVerbose() const { return m_verboseOutput; }

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

    app.add_flag("-v,--verbose,!--no-verbose", m_verboseOutput)
      ->description("Enable/disable verbose output")
      ->capture_default_str();

    app.add_option("-r,--radius", circleRadius)
      ->description("Radius for circle")
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

//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
#ifdef AXOM_USE_MPI
  MPI_Init(&argc, &argv);
  int my_rank, num_ranks;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
#else
  int my_rank = 0;
  int num_ranks = 1;
#endif

  slic::SimpleLogger logger;

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

#ifdef AXOM_USE_MPI
    MPI_Bcast(&retval, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif
    exit(retval);
  }

  constexpr int DIM = 2;

#if defined(AXOM_USE_RAJA) && defined(AXOM_USE_UMPIRE)
  using SeqClosestPointQueryType =
    quest::DistributedClosestPoint<2, axom::SEQ_EXEC>;

  #ifdef AXOM_USE_OPENMP
  using OmpClosestPointQueryType =
    quest::DistributedClosestPoint<2, axom::OMP_EXEC>;
  #endif

  #ifdef AXOM_USE_CUDA
  using CudaClosestPointQueryType =
    quest::DistributedClosestPoint<2, axom::CUDA_EXEC<256>>;
  #endif
#endif

  using PointArray = SeqClosestPointQueryType::PointArray;
  using IndexSet = slam::PositionSet<>;
  using IndexArray = axom::Array<axom::IndexType>;

  //---------------------------------------------------------------------------
  // Load/generate object mesh
  //---------------------------------------------------------------------------
  sidre::DataStore objectDS;
  ObjectMeshWrapper object_mesh_wrapper(
    objectDS.getRoot()->createGroup("object_mesh"));

  object_mesh_wrapper.generateCircleMesh(params.circleRadius,
                                         params.circlePoints);
  object_mesh_wrapper.saveMesh();

  //---------------------------------------------------------------------------
  // Load mesh and get vertex positions
  //---------------------------------------------------------------------------

  QueryMeshWrapper query_mesh_wrapper;

  query_mesh_wrapper.setupMesh(params.getDCMeshName(), params.meshFile);
  query_mesh_wrapper.printMeshInfo();

  // Copy mesh nodes into qpts array
  auto qPts = query_mesh_wrapper.getVertexPositions<PointArray>();
  const int nQueryPts = qPts.size();

  // Create an array for the indices of the closest points
  IndexArray cpIndices;

  // Create an array to hold the object points
  PointArray objectPts;

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

  switch(params.policy)
  {
  case RuntimePolicy::seq:
  {
    SeqClosestPointQueryType query;
    query.setVerbosity(params.isVerbose());
    query.setObjectMesh(object_mesh_wrapper.getBlueprintGroup());

    SLIC_INFO(init_str);
    initTimer.start();
    query.generateBVHTree();
    initTimer.stop();

    SLIC_INFO(query_str);
    queryTimer.start();
    query.computeClosestPoints(qPts, cpIndices);
    queryTimer.stop();
    objectPts = query.points();
  }
  break;
  case RuntimePolicy::omp:
#if defined(AXOM_USE_RAJA) && defined(AXOM_USE_UMPIRE) && \
  defined(AXOM_USE_OPENMP)
  {
    OmpClosestPointQueryType query;
    query.setVerbosity(params.isVerbose());
    query.setObjectMesh(object_mesh_wrapper.getBlueprintGroup());

    SLIC_INFO(init_str);
    initTimer.start();
    query.generateBVHTree();
    initTimer.stop();

    SLIC_INFO(query_str);
    queryTimer.start();
    query.computeClosestPoints(qPts, cpIndices);
    queryTimer.stop();

    objectPts = query.points();
  }
#endif
  break;
  case RuntimePolicy::cuda:
#if defined(AXOM_USE_RAJA) && defined(AXOM_USE_UMPIRE) && defined(AXOM_USE_CUDA)
  {
    CudaClosestPointQueryType query;
    query.setVerbosity(params.isVerbose());
    query.setObjectMesh(object_mesh_wrapper.getBlueprintGroup());

    SLIC_INFO(init_str);
    initTimer.start();
    query.generateBVHTree();
    initTimer.stop();

    SLIC_INFO(query_str);
    queryTimer.start();
    query.computeClosestPoints(qPts, cpIndices);
    queryTimer.stop();

    objectPts = query.points();
  }
#endif
  break;
  }

  if(params.isVerbose())
  {
    SLIC_INFO(axom::fmt::format("Closest points ({}):", cpIndices.size()));
    for(auto i : IndexSet(cpIndices.size()))
    {
      SLIC_INFO(axom::fmt::format("\t{}: {}", i, cpIndices[i]));
    }
  }

  SLIC_INFO(axom::fmt::format("Initialization with policy {} took {} seconds",
                              params.policy,
                              initTimer.elapsedTimeInSec()));
  SLIC_INFO(axom::fmt::format("Query with policy {} took {} seconds",
                              params.policy,
                              queryTimer.elapsedTimeInSec()));

  //---------------------------------------------------------------------------
  // Transform closest points to distances and directions
  //---------------------------------------------------------------------------
  using primal::squared_distance;

  auto* distances = query_mesh_wrapper.getDC()->GetField("distance");
  auto* directions = query_mesh_wrapper.getDC()->GetField("direction");
  SLIC_INFO(axom::fmt::format(" distance size: {}", distances->Size()));
  mfem::Array<int> dofs;
  for(auto i : IndexSet(nQueryPts))
  {
    const auto& cp = objectPts[cpIndices[i]];
    (*distances)(i) = sqrt(squared_distance(qPts[i], cp));

    primal::Vector<double, DIM> dir(qPts[i], cp);
    directions->FESpace()->GetVertexVDofs(i, dofs);
    directions->SetSubVector(dofs, dir.data());
  }

  //---------------------------------------------------------------------------
  // Cleanup, save mesh/fields and exit
  //---------------------------------------------------------------------------
  query_mesh_wrapper.saveMesh();

#ifdef AXOM_USE_MPI
  MPI_Finalize();
#endif

  return 0;
}