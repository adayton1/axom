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
namespace numerics = axom::numerics;

template <int NDIMS = 2, typename ExecSpace = axom::SEQ_EXEC>
class ClosestPointQuery
{
public:
  // TODO: generalize to 3D
  static_assert(NDIMS == 2, "ClosestPointQuery only currently supports 2D");

  static constexpr int DIM = NDIMS;
  using PointType = primal::Point<double, DIM>;
  using BoxType = primal::BoundingBox<double, DIM>;
  using PointArray = axom::Array<PointType>;
  using BoxArray = axom::Array<BoxType>;
  using BVHTreeType = spin::BVH<DIM, ExecSpace>;

private:
  struct MinCandidate
  {
    /// Squared distance to query point
    double minSqDist {numerics::floating_point_limits<double>::max()};
    /// Index within mesh of closest element
    int minElem;
  };

public:
  ClosestPointQuery(
    int allocatorID = axom::execution_space<ExecSpace>::allocatorID())
    : m_allocatorID(allocatorID)
  { }

public:  // Query properties
  void setVerbosity(bool isVerbose) { m_isVerbose = isVerbose; }

public:
  /// Utility function to generate an array of 2D points
  void generatePoints(double radius, int numPoints)
  {
    using axom::utilities::random_real;

    // Generate in host because random_real is not yet ported to the device
    PointArray pts;
    pts.reserve(numPoints);
    for(int i = 0; i < numPoints; ++i)
    {
      const double angleInRadians = random_real(0., 2 * M_PI);
      const double rsinT = radius * std::sin(angleInRadians);
      const double rcosT = radius * std::cos(angleInRadians);

      pts.emplace_back(PointType {rcosT, rsinT});
    }

    if(m_isVerbose)
    {
      SLIC_INFO("Points on object:");
      const auto& arr = m_points;
      for(auto i : slam::PositionSet<int>(arr.size()))
      {
        const double mag = sqrt(arr[i][0] * arr[i][0] + arr[i][1] * arr[i][1]);
        SLIC_INFO(axom::fmt::format("\t{}: {} -- {}", i, arr[i], mag));
      }
    }

    m_points = PointArray(pts, m_allocatorID);  // copy to ExecSpace
  }

  bool generateBVHTree()
  {
    const int npts = m_points.size();
    axom::Array<BoxType> boxesArray(npts, npts, m_allocatorID);
    auto boxesView = boxesArray.view();
    axom::for_all<ExecSpace>(
      npts,
      AXOM_LAMBDA(axom::IndexType i) { boxesView[i] = BoxType {m_points[i]}; });

    // Build bounding volume hierarchy
    m_bvh.setAllocatorID(m_allocatorID);
    int result = m_bvh.initialize(boxesView, npts);

    return (result == spin::BVH_BUILD_OK);
  }

  void computeClosestPoints(const PointArray& queryPts,
                            axom::Array<axom::IndexType>& cpIndexes) const
  {
    SLIC_ASSERT(!queryPts.empty());

    const int nPts = queryPts.size();

    /// Create an ArrayView in ExecSpace that is compatible with cpIndexes
    axom::Array<axom::IndexType> cp_idx(nPts, nPts, m_allocatorID);
    auto query_inds = cp_idx.view();

    /// Create an ArrayView in ExecSpace that is compatible with queryPts
    PointArray execPoints(nPts, nPts, m_allocatorID);
    execPoints = queryPts;
    auto query_pts = execPoints.view();

    // Get a device-useable iterator
    auto it = m_bvh.getTraverser();

    using axom::primal::squared_distance;
    using int32 = axom::int32;

    AXOM_PERF_MARK_SECTION(
      "ComputeClosestPoints",
      axom::for_all<ExecSpace>(
        nPts,
        AXOM_LAMBDA(int32 idx) mutable {
          PointType qpt = query_pts[idx];

          MinCandidate curr_min {};

          auto searchMinDist = [&](int32 current_node, const int32* leaf_nodes) {
            const int candidate_idx = leaf_nodes[current_node];
            const PointType candidate_pt = m_points[candidate_idx];
            const double sq_dist = squared_distance(qpt, candidate_pt);

            if(sq_dist < curr_min.minSqDist)
            {
              curr_min.minSqDist = sq_dist;
              curr_min.minElem = candidate_idx;
            }
          };

          auto traversePredicate = [&](const PointType& p,
                                       const BoxType& bb) -> bool {
            return squared_distance(p, bb) <= curr_min.minSqDist;
          };

          // Traverse the tree, searching for the point with minimum distance.
          it.traverse_tree(qpt, searchMinDist, traversePredicate);

          query_inds[idx] = curr_min.minElem;
        }););

    cpIndexes = query_inds;
  }

  const PointArray& points() const { return m_points; }

private:
  PointArray m_points;
  BoxArray m_boxes;
  BVHTreeType m_bvh;

  int m_allocatorID;
  bool m_isVerbose {false};
};

class MeshWrapper
{
public:
  MeshWrapper() : m_dc("closest_point", nullptr, true) { }

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
    SLIC_INFO(axom::fmt::format(
      "{:=^80}",
      axom::fmt::format("Saving mesh '{}' to disk", m_dc.GetCollectionName())));

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
  using SeqClosestPointQueryType = ClosestPointQuery<2, axom::SEQ_EXEC>;

  #ifdef AXOM_USE_OPENMP
  using OmpClosestPointQueryType = ClosestPointQuery<2, axom::OMP_EXEC>;
  #endif

  #ifdef AXOM_USE_CUDA
  using CudaClosestPointQueryType = ClosestPointQuery<2, axom::CUDA_EXEC<256>>;
  #endif
#endif

  using PointArray = SeqClosestPointQueryType::PointArray;
  using IndexSet = slam::PositionSet<>;
  using IndexArray = axom::Array<axom::IndexType>;

  //---------------------------------------------------------------------------
  // Load mesh and get vertex positions
  //---------------------------------------------------------------------------

  MeshWrapper mesh_wrapper;

  mesh_wrapper.setupMesh(params.getDCMeshName(), params.meshFile);
  mesh_wrapper.printMeshInfo();

  // Copy mesh nodes into qpts array
  auto qPts = mesh_wrapper.getVertexPositions<PointArray>();
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
    query.generatePoints(params.circleRadius, params.circlePoints);

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
    query.generatePoints(params.circleRadius, params.circlePoints);

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
    query.generatePoints(params.circleRadius, params.circlePoints);

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

  auto* distances = mesh_wrapper.getDC()->GetField("distance");
  auto* directions = mesh_wrapper.getDC()->GetField("direction");
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
  mesh_wrapper.saveMesh();

#ifdef AXOM_USE_MPI
  MPI_Finalize();
#endif

  return 0;
}
