// Copyright (c) 2017-2022, Lawrence Livermore National Security, LLC and
// other Axom Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

/*!
 * \file evaluate_integral.hpp
 *
 * \brief Consists of methods that evaluate integrals over regions defined
 * by Bezier curves, such as 2D area integrals and scalar/vector field line integrals
 *
 * Line integrals are computed with 1D quadrature rules supplied
 * by MFEM. 2D area integrals computed with "Spectral Mesh-Free Quadrature for Planar 
 * Regions Bounded by Rational Parametric Curves" by David Gunderman et al.
 * 
 * \note This requires the MFEM third-party library
 */

#ifndef PRIMAL_EVAL_INTEGRAL_HPP_
#define PRIMAL_EVAL_INTEGRAL_HPP_

// Axom includes
#include "axom/config.hpp"
#include "axom/primal.hpp"

#include "axom/primal/operators/detail/evaluate_integral_impl.hpp"

// MFEM includes
#ifdef AXOM_USE_MFEM
  #include "mfem.hpp"
#else
  #error "Primal's integral evaluation functions require mfem library."
#endif

// C++ includes
#include <cmath>

namespace axom
{
namespace primal
{
/*!
 * \brief Evaluate a line integral along a collection of Bezier curves
 *
 * The line integral is evaluated on each curve in the array, and added
 * together to represent the total integral. The curves need not be connected.
 * Uses 1D Gaussian quadrature generated by MFEM.
 *
 * \param [in] cs the array of Bezier curve objects
 * \param [in] integrand the lambda function representing the integrand. 
 * Must accept a 2D point as input, and return a double (scalar field) or 
 * 2D vector object (vector field).
 * \param [in] npts the number of Gaussian quadrature nodes for each component
 * \return the value of the integral
 */
template <class Lambda, typename T, int NDIMS>
double evaluate_line_integral(const axom::Array<primal::BezierCurve<T, NDIMS>>& cs,
                              Lambda&& integrand,
                              int npts)
{
  // Generate quadrature library, defaulting to GaussLegendre quadrature.
  //  Use the same one for every curve in the polygon
  //  Quadrature order is equal to 2*N - 1
  static mfem::IntegrationRules my_IntRules(0, mfem::Quadrature1D::GaussLegendre);
  const mfem::IntegrationRule& quad =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts - 1);

  double total_integral = 0.0;
  for(const auto& curve : cs)
  {
    // Compute the line integral along each component.
    total_integral +=
      detail::evaluate_line_integral_component(curve, integrand, quad);
  }

  return total_integral;
}

/*!
 * \brief Evaluate a line integral along the boundary of a CurvedPolygon object.
 *
 * See above definition for details.
 * 
 * \param [in] cpoly the CurvedPolygon object
 * \param [in] integrand the lambda function representing the integrand. 
 * Must accept a 2D point as input and return a double
 * \param [in] npts_Q the number of quadrature points to evaluate the line integral
 * \param [in] npts_P the number of quadrature points to evaluate the antiderivative
 * \return the value of the integral
 */
template <class Lambda, typename T, int NDIMS>
double evaluate_line_integral(const primal::CurvedPolygon<T, NDIMS> cpoly,
                              Lambda&& integrand,
                              int npts)
{
  // Generate quadrature library, defaulting to GaussLegendre quadrature.
  //  Use the same one for every curve in the polygon
  //  Quadrature order is equal to 2*N - 1
  static mfem::IntegrationRules my_IntRules(0, mfem::Quadrature1D::GaussLegendre);
  const mfem::IntegrationRule& quad =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts - 1);

  double total_integral = 0.0;
  for(int i = 0; i < cpoly.numEdges(); i++)
  {
    // Compute the line integral along each component.
    total_integral +=
      detail::evaluate_line_integral_component(cpoly[i], integrand, quad);
  }

  return total_integral;
}

/*!
 * \brief Evaluate a line integral on a single Bezier curve.
 *
 * Evaluate the line integral with a given number of Gaussian
 * quadrature nodes generated by MFEM.
 *
 * \param [in] c the Bezier curve object
 * \param [in] integrand the lambda function representing the integrand. 
 * Must accept a 2D point as input, and return a double (scalar field) or 
 * 2D vector object (vector field).
 * \param [in] npts the number of quadrature nodes
 * \return the value of the integral
 */
template <class Lambda, typename T, int NDIMS>
double evaluate_line_integral(const primal::BezierCurve<T, NDIMS>& c,
                              Lambda&& integrand,
                              int npts)
{
  // Generate quadrature library, defaulting to GaussLegendre quadrature.
  //  Use the same one for every curve in the polygon
  //  Gaussian quadrature order is equal to 2*Npts - 1
  static mfem::IntegrationRules my_IntRules(0, mfem::Quadrature1D::GaussLegendre);
  const mfem::IntegrationRule& quad =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts - 1);

  return detail::evaluate_line_integral_component(c, integrand, quad);
}

/*!
 * \brief Evaluate an integral across a 2D domain bounded by Bezier curves.
 *
 * Assumes that the array of Bezier curves is closed and connected. Will compute
 * the integral regardless, but the result will be meaningless.
 * Uses a Spectral Mesh-Free Quadrature derived from Green's theorem, evaluating
 * the area integral as a line integral of the antiderivative over the curve.
 *
 * \param [in] cs the array of Bezier curve objects that bound the region
 * \param [in] integrand the lambda function representing the integrand. 
 * Must accept a 2D point as input and return a double
 * \param [in] npts_Q the number of quadrature points to evaluate the line integral
 * \param [in] npts_P the number of quadrature points to evaluate the antiderivative
 * \return the value of the integral
 */
template <class Lambda, typename T>
double evaluate_area_integral(const axom::Array<primal::BezierCurve<T, 2>>& cs,
                              Lambda&& integrand,
                              int npts_Q,
                              int npts_P = 0)
{
  // Generate quadrature library, defaulting to GaussLegendre quadrature.
  //  Use the same one for every curve in the polygon
  static mfem::IntegrationRules my_IntRules(0, mfem::Quadrature1D::GaussLegendre);

  if(npts_P <= 0) npts_P = npts_Q;

  // Get the quadrature for the line integral.
  //  Quadrature order is equal to 2*N - 1
  const mfem::IntegrationRule& quad_Q =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts_Q - 1);
  const mfem::IntegrationRule& quad_P =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts_P - 1);

  // Use minimum y-coord of control nodes as lower bound for integration
  double int_lb = cs[0][0][1];
  for(int i = 0; i < cs.size(); i++)
    for(int j = 1; j < cs[i].getOrder() + 1; j++)
      int_lb = std::min(int_lb, cs[i][j][1]);

  // Evaluate the antiderivative line integral along each component
  double total_integral = 0.0;
  for(const auto& curve : cs)
  {
    total_integral += detail::evaluate_area_integral_component(curve,
                                                               integrand,
                                                               int_lb,
                                                               quad_Q,
                                                               quad_P);
  }

  return total_integral;
}

/*!
 * \brief Evaluate an integral on the interior of a CurvedPolygon object.
 *
 * See above definition for details.
 * 
 * \param [in] cs the array of Bezier curve objects that bound the region
 * \param [in] integrand the lambda function representing the integrand. 
 * Must accept a 2D point as input and return a double
 * \param [in] npts_Q the number of quadrature points to evaluate the line integral
 * \param [in] npts_P the number of quadrature points to evaluate the antiderivative
 * \return the value of the integral
 */
template <class Lambda, typename T>
double evaluate_area_integral(const primal::CurvedPolygon<T, 2> cpoly,
                              Lambda&& integrand,
                              int npts_Q,
                              int npts_P = 0)
{
  // Generate quadrature library, defaulting to GaussLegendre quadrature.
  //  Use the same one for every curve in the polygon
  static mfem::IntegrationRules my_IntRules(0, mfem::Quadrature1D::GaussLegendre);

  if(npts_P <= 0) npts_P = npts_Q;

  // Get the quadrature for the line integral.
  //  Quadrature order is equal to 2*N - 1
  const mfem::IntegrationRule& quad_Q =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts_Q - 1);
  const mfem::IntegrationRule& quad_P =
    my_IntRules.Get(mfem::Geometry::SEGMENT, 2 * npts_P - 1);

  // Use minimum y-coord of control nodes as lower bound for integration
  double int_lb = cpoly[0][0][1];
  for(int i = 0; i < cpoly.numEdges(); i++)
    for(int j = 1; j < cpoly[i].getOrder() + 1; j++)
      int_lb = std::min(int_lb, cpoly[i][j][1]);

  // Evaluate the antiderivative line integral along each component
  double total_integral = 0.0;
  for(int i = 0; i < cpoly.numEdges(); i++)
  {
    total_integral += detail::evaluate_area_integral_component(cpoly[i],
                                                               integrand,
                                                               int_lb,
                                                               quad_Q,
                                                               quad_P);
  }

  return total_integral;
}

}  // namespace primal
}  // end namespace axom

#endif
