// Copyright (c) 2017-2021, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef AXOM_QUEST_POINT_IN_CELL_POINT_FINDER_HPP_
#define AXOM_QUEST_POINT_IN_CELL_POINT_FINDER_HPP_

#include "axom/spin/ImplicitGrid.hpp"
#include "axom/primal/geometry/BoundingBox.hpp"

namespace axom
{
namespace quest
{
// Predeclare mesh traits class
template <typename mesh_tag>
struct PointInCellTraits;

namespace detail
{
// Predeclare mesh wrapper class
template <typename mesh_tag>
class PointInCellMeshWrapper;

/*!
 * \class PointFinder
 *
 * \brief A class to encapsulate locating points within the cells
 * of a computational mesh.
 *
 * \tparam NDIMS The dimension of the mesh
 * \tparam mesh_tag A tag struct used to identify the mesh
 *
 * \note This class implements part of the functionality of \a PointInCell
 * \note This class assumes the existence of specialized implementations of
 * the following two classes for the provided \a mesh_tag:
 *   \arg axom::quest::PointInCellTraits
 *   \arg axom::quest::detail::PointInCellMeshWrapper
 */
template <int NDIMS, typename mesh_tag, typename ExecSpace>
class PointFinder
{
public:
  using GridType = spin::ImplicitGrid<NDIMS, ExecSpace>;

  using SpacePoint = typename GridType::SpacePoint;
  using SpatialBoundingBox = typename GridType::SpatialBoundingBox;

  using MeshWrapperType = PointInCellMeshWrapper<mesh_tag>;
  using IndexType = typename MeshWrapperType::IndexType;

public:
  /*!
   * Constructor for PointFinder
   *
   * \param meshWrapper A non-null MeshWrapperType
   * \param res The grid resolution for the spatial acceleration structure
   * \param bboxScaleFactor A number slightly larger than 1 by which to expand
   * cell bounding boxes
   *
   * \sa constructors in PointInCell class for more details about parameters
   */
  PointFinder(const MeshWrapperType* meshWrapper,
              const int* res,
              double bboxScaleFactor,
              int allocatorID)
    : m_meshWrapper(meshWrapper)
    , m_allocatorID(allocatorID)
  {
    SLIC_ASSERT(m_meshWrapper != nullptr);
    SLIC_ASSERT(bboxScaleFactor >= 1.);

    const axom::IndexType numCells = m_meshWrapper->numElements();

    // setup bounding boxes -- Slightly scaled for robustness

    SpatialBoundingBox meshBBox;
    m_cellBBoxes =
      axom::Array<SpatialBoundingBox>(numCells, numCells, allocatorID);
    m_meshWrapper->template computeBoundingBoxes<NDIMS>(bboxScaleFactor,
                                                        m_cellBBoxes.data(),
                                                        meshBBox);

    // initialize implicit grid, handle case where resolution is a NULL pointer
    if(res != nullptr)
    {
      using GridResolution = axom::primal::Point<int, NDIMS>;
      GridResolution gridRes(res);
      m_grid.initialize(meshBBox, &gridRes, numCells, allocatorID);
    }
    else
    {
      m_grid.initialize(meshBBox, nullptr, numCells, allocatorID);
    }

    // add mesh elements to grid
    m_grid.insert(numCells, m_cellBBoxes.data());
  }

  /*!
   * Query to find the mesh cell containing query point with coordinates \a pos
   *
   * \sa PointInCell::locatePoint() for more details about parameters
   */
  IndexType locatePoint(const double* pos, double* isoparametric) const
  {
    IndexType containingCell = PointInCellTraits<mesh_tag>::NO_CELL;

    SLIC_ASSERT(pos != nullptr);
    SpacePoint pt(pos);
    SpacePoint isopar;

    locatePoints(axom::ArrayView<const SpacePoint>(&pt, 1),
                 &containingCell,
                 &isopar);

    // Copy data back to input parameter isoparametric, if necessary
    if(isoparametric != nullptr)
    {
      isopar.array().to_array(isoparametric);
    }

    return containingCell;
  }

  void locatePoints(axom::ArrayView<const SpacePoint> pts,
                    IndexType* outCellIds,
                    SpacePoint* outIsoparametricCoords) const
  {
    auto gridQuery = m_grid.getQueryObject();

    axom::IndexType npts = pts.size();

    axom::Array<IndexType> offsets(npts, npts, m_allocatorID);
    axom::Array<IndexType> counts(npts, npts, m_allocatorID);

#ifdef AXOM_USE_RAJA
    IndexType* countsPtr = counts.data();

    using reduce_pol = typename axom::execution_space<ExecSpace>::reduce_policy;
    RAJA::ReduceSum<reduce_pol, IndexType> totalCountReduce(0);
    // Step 1: count number of candidate intersections for each point
    for_all<ExecSpace>(
      npts,
      AXOM_LAMBDA(IndexType i) {
        countsPtr[i] = gridQuery.countCandidates(pts[i]);
        totalCountReduce += countsPtr[i];
      });

    // Step 2: exclusive scan for offsets in candidate array
    using exec_policy = typename axom::execution_space<ExecSpace>::loop_policy;
    RAJA::exclusive_scan<exec_policy>(RAJA::make_span(counts.data(), npts),
                                      RAJA::make_span(offsets.data(), npts),
                                      RAJA::operators::plus<IndexType> {});

    axom::IndexType totalCount = totalCountReduce.get();

    // Step 3: allocate memory for all candidates
    axom::Array<IndexType> candidates(totalCount, totalCount, m_allocatorID);
    IndexType* candidatesPtr = candidates.data();
    IndexType* offsetsPtr = offsets.data();
    const SpatialBoundingBox* cellBBoxes = m_cellBBoxes.data();

    // Step 4: fill candidate array for each query box
    for_all<ExecSpace>(
      npts,
      AXOM_LAMBDA(IndexType i) {
        int startIdx = offsetsPtr[i];
        int currCount = 0;
        auto onCandidate = [&](int candidateIdx) -> bool {
          // Check that point is in bounding box of candidate element
          if(cellBBoxes[candidateIdx].contains(pts[i]))
          {
            candidatesPtr[startIdx] = candidateIdx;
            currCount++;
            startIdx++;
          }
          return currCount >= countsPtr[i];
        };
        gridQuery.visitCandidates(pts[i], onCandidate);
        countsPtr[i] = currCount;
      });

    // Step 5: Check each candidate
    for_all<SEQ_EXEC>(
      npts,
      AXOM_HOST_LAMBDA(IndexType i) {
        outCellIds[i] = PointInCellTraits<mesh_tag>::NO_CELL;
        SpacePoint pt = pts[i];
        SpacePoint isopar;
        for(int icell = 0; icell < countsPtr[i]; icell++)
        {
          int cellIdx = candidates[icell + offsetsPtr[i]];
          // if isopar is in the proper range
          if(m_meshWrapper->locatePointInCell(cellIdx, pt.data(), isopar.data()))
          {
            // then we have found the cellID
            outCellIds[i] = cellIdx;
            break;
          }
        }
        if(outIsoparametricCoords != nullptr)
        {
          outIsoparametricCoords[i] = isopar;
        }
      });
#else
    for(int i = 0; i < npts; i++)
    {
      SpacePoint pt = pts[i];
      SpacePoint isopar;
      gridQuery.visitCandidates(pt, [&](int candidateIdx) -> bool {
        if(m_cellBBoxes[candidateIdx].contains(pts[i]))
        {
          if(m_meshWrapper->locatePointInCell(candidateIdx,
                                              pt.data(),
                                              isopar.data()))
          {
            outCellIds[i] = candidateIdx;
            return true;
          }
        }
        return false;
      });
      if(outIsoparametricCoords != nullptr)
      {
        outIsoparametricCoords[i] = isopar;
      }
    }
#endif
  }

  /*! Returns a const reference to the given cells's bounding box */
  const SpatialBoundingBox& cellBoundingBox(IndexType cellIdx) const
  {
    return m_cellBBoxes[cellIdx];
  }

private:
  GridType m_grid;
  const MeshWrapperType* m_meshWrapper;
  axom::Array<SpatialBoundingBox> m_cellBBoxes;
  int m_allocatorID;
};

}  // end namespace detail
}  // end namespace quest
}  // end namespace axom

#endif  // AXOM_QUEST_POINT_IN_CELL_POINT_FINDER_HPP_
