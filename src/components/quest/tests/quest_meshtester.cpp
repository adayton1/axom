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

// Axom includes
#include "axom/config.hpp"
#include "axom_utils/FileUtilities.hpp"
#include "mint/config.hpp"
#include "mint/Mesh.hpp"
#include "quest/STLReader.hpp"
#include "quest/MeshTester.hpp"
#include "slic/slic.hpp"

// Google test include
#include "gtest/gtest.h"

// C++ includes
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>

typedef axom::mint::UnstructuredMesh< axom::mint::Topology::SINGLE > UMesh;

std::string vecToString(const std::vector<int> & v)
{
  std::stringstream retval;
  for (unsigned int i = 0 ; i < v.size() ; ++i)
  {
    retval << v[i] << "  ";
  }
  return retval.str();
}

std::string vecToString(const std::vector< std::pair<int, int> > & v)
{
  std::stringstream retval;
  for (unsigned int i = 0 ; i < v.size() ; ++i)
  {
    retval << "(" << v[i].first << " " << v[i].second << ")  ";
  }
  return retval.str();
}

template<typename T>
void reportVectorMismatch(const std::vector<T> & standard,
                          const std::vector<T> & result,
                          const std::string & label)
{
  std::vector<T> missing, unexpected;

  std::set_difference(standard.begin(), standard.end(),
                      result.begin(),   result.end(),
                      std::inserter(missing, missing.begin()));
  std::set_difference(result.begin(),   result.end(),
                      standard.begin(), standard.end(),
                      std::inserter(unexpected, unexpected.begin()));

  EXPECT_TRUE(missing.size() == 0)
    << "Missing " << missing.size()
    << " " << label << ":" << std::endl << vecToString(missing);
  EXPECT_TRUE(unexpected.size() == 0)
    << "Unexpectedly, " << unexpected.size()
    << " extra " << label << ":" << std::endl << vecToString(unexpected);
}

void runIntersectTest(const std::string &tname,
                      UMesh* surface_mesh,
                      const std::vector< std::pair<int, int> > & expisect,
                      const std::vector< int > & expdegen)
{
  SCOPED_TRACE(tname);

  SLIC_INFO("Intersection test " << tname);

  std::vector< int > degenerate;
  std::vector< std::pair<int, int> > collisions;
  // Later, perhaps capture the return value as a status and report it.
  (void) axom::quest::findTriMeshIntersections(surface_mesh, collisions,
                                               degenerate);

  // report discrepancies
  std::sort(collisions.begin(), collisions.end());
  std::sort(degenerate.begin(), degenerate.end());

  reportVectorMismatch(expisect, collisions, "triangle collisions");
  reportVectorMismatch(expdegen, degenerate, "degenerate triangles");
}

void splitStringToIntPairs(
  std::string & pairs,
  std::vector< std::pair<int, int> > & dat)
{
  if (!pairs.empty())
  {
    std::istringstream iss(pairs);
    while (iss.good())
    {
      std::pair<int, int> p;
      iss >> p.first >> p.second;
      dat.push_back(p);
    }
  }
}

void splitStringToInts(std::string & ints, std::vector< int > & dat)
{
  if (!ints.empty())
  {
    std::istringstream iss(ints);
    while (iss.good())
    {
      int i;
      iss >> i;
      dat.push_back(i);
    }
  }
}

std::string readIntersectTest(std::string & test,
                              std::string & tfname,
                              std::vector< std::pair<int, int> > & expisect,
                              std::vector< int > & expdegen)
{
  // given a test file path in argument test,
  // return the display name for the test (from the first line of the file).
  // Output argument tfname supplies the mesh file to read in (second line, path
  // relative to test)
  // Output arg expisect (third line) supplies the expected intersecting
  // triangles
  // Output arg expdegen (fourth line) supplies the expected degenerate
  // triangles

  std::string testdir;
  axom::utilities::filesystem::getDirName(testdir, test);

  std::ifstream testfile(test.c_str());
  std::string retval;
  std::getline(testfile, retval);
  std::getline(testfile, tfname);
  tfname = axom::utilities::filesystem::joinPath(testdir, tfname);
  std::string splitline;
  std::getline(testfile, splitline);
  splitStringToIntPairs(splitline, expisect);
  std::sort(expisect.begin(), expisect.end());
  std::getline(testfile, splitline);
  splitStringToInts(splitline, expdegen);
  std::sort(expdegen.begin(), expdegen.end());

  return retval;
}

std::vector<std::string> findIntersectTests()
{
  std::vector<std::string> tests;

  std::string catalogue =
    axom::utilities::filesystem::
    joinPath(AXOM_SRC_DIR, "components/quest/data/meshtester/catalogue.txt");

  std::string testdir;
  axom::utilities::filesystem::getDirName(testdir, catalogue);

  // open file, and put each of its lines into return value tests.
  std::ifstream catfile(catalogue.c_str());
  std::string line;
  while (std::getline(catfile, line))
  {
    tests.push_back(axom::utilities::filesystem::joinPath(testdir, line));
  }
  return tests;
}

TEST( quest_mesh_tester, surfacemesh_self_intersection_intrinsic )
{
  std::vector< std::pair<int, int> > intersections;
  std::vector< int > degenerate;
  UMesh* surface_mesh = AXOM_NULLPTR;
  std::string testname;
  std::string testdescription;

  {
    testname = "tetrahedron";
    testdescription = "Tetrahedron with no errors";

    // Construct and fill the mesh.
    // There are many ways to do this, some nicer than others.  Whether the
    // mesh has nice de-duplicated nodes or is a tiresome STL-style triangle
    // soup, it should not matter.  We will test the deduplicated triangles.
    // Nice (non-duplicated) vertices
    surface_mesh = new UMesh(3, axom::mint::TRIANGLE, 4, 4 );
    surface_mesh->appendNode( -0.000003, -0.000003, 19.999999);
    surface_mesh->appendNode(-18.213671,  4.880339, -6.666668);
    surface_mesh->appendNode(  4.880339,-18.213671, -6.666668);
    surface_mesh->appendNode( 13.333334, 13.333334, -6.666663);
    axom::mint::IndexType cell[3];
    cell[0] = 0;    cell[1] = 1;    cell[2] = 2;
    surface_mesh->appendCell(cell);
    cell[0] = 0;    cell[1] = 3;    cell[2] = 1;
    surface_mesh->appendCell(cell);
    cell[0] = 0;    cell[1] = 2;    cell[2] = 3;
    surface_mesh->appendCell(cell);
    cell[0] = 1;    cell[1] = 3;    cell[2] = 2;
    surface_mesh->appendCell(cell);

    // No self-intersections or degenerate triangles
    intersections.clear();
    degenerate.clear();
    runIntersectTest(testdescription,
                     surface_mesh, intersections, degenerate);
    delete surface_mesh;
  }

  {
    testname = "cracked tetrahedron";
    testdescription =
      "Tetrahedron with a crack but no self-intersections or degenerate triangles";

    // Construct and fill the mesh.
    surface_mesh = new UMesh(3, axom::mint::TRIANGLE, 5, 4 );
    surface_mesh->appendNode( -0.000003, -0.000003, 19.999999);
    surface_mesh->appendNode(-18.213671,  4.880339, -6.666668);
    surface_mesh->appendNode(  4.880339,-18.213671, -6.666668);
    surface_mesh->appendNode( 13.333334, 13.333334, -6.666663);
    surface_mesh->appendNode( -0.200003, -0.100003, 18.999999);
    axom::mint::IndexType cell[3];
    cell[0] = 4;    cell[1] = 1;    cell[2] = 2;
    surface_mesh->appendCell(cell);
    cell[0] = 0;    cell[1] = 3;    cell[2] = 1;
    surface_mesh->appendCell(cell);
    cell[0] = 0;    cell[1] = 2;    cell[2] = 3;
    surface_mesh->appendCell(cell);
    cell[0] = 1;    cell[1] = 3;    cell[2] = 2;
    surface_mesh->appendCell(cell);

    // No self-intersections or degenerate triangles
    intersections.clear();
    degenerate.clear();
    runIntersectTest(testdescription,
                     surface_mesh, intersections, degenerate);
    delete surface_mesh;
  }

  {
    testname = "caved-in tetrahedron";
    testdescription =
      "Tetrahedron with one side intersecting two others, no degenerate triangles";

    // Construct and fill the mesh.
    surface_mesh = new UMesh(3, axom::mint::TRIANGLE, 5, 4);
    surface_mesh->appendNode(  2.00003,   1.00003,  18.999999);
    surface_mesh->appendNode(-18.213671,  4.880339, -6.666668);
    surface_mesh->appendNode(  4.880339,-18.213671, -6.666668);
    surface_mesh->appendNode( -0.000003, -0.000003, 19.999999);
    surface_mesh->appendNode( 13.333334, 13.333334, -6.666663);
    axom::mint::IndexType cell[3];
    cell[0] = 0;    cell[1] = 1;    cell[2] = 2;
    surface_mesh->appendCell(cell);
    cell[0] = 3;    cell[1] = 4;    cell[2] = 1;
    surface_mesh->appendCell(cell);
    cell[0] = 3;    cell[1] = 2;    cell[2] = 4;
    surface_mesh->appendCell(cell);
    cell[0] = 1;    cell[1] = 4;    cell[2] = 2;
    surface_mesh->appendCell(cell);

    intersections.clear();
    intersections.push_back(std::make_pair(0, 1));
    intersections.push_back(std::make_pair(0, 2));
    // No degenerate triangles
    degenerate.clear();
    runIntersectTest(testdescription,
                     surface_mesh, intersections, degenerate);
    delete surface_mesh;
  }

  {
    testname = "caved-in tet with added degenerate tris";
    testdescription =
      "Tetrahedron with one side intersecting two others, some degenerate triangles";

    // Construct and fill the mesh.
    surface_mesh = new UMesh(3, axom::mint::TRIANGLE, 5, 6);
    surface_mesh->appendNode(  2.00003,   1.00003,  18.999999);
    surface_mesh->appendNode(-18.213671,  4.880339, -6.666668);
    surface_mesh->appendNode(  4.880339,-18.213671, -6.666668);
    surface_mesh->appendNode( -0.000003, -0.000003, 19.999999);
    surface_mesh->appendNode( 13.333334, 13.333334, -6.666663);
    axom::mint::IndexType cell[3];
    cell[0] = 0;    cell[1] = 1;    cell[2] = 2;
    surface_mesh->appendCell(cell);
    cell[0] = 3;    cell[1] = 4;    cell[2] = 1;
    surface_mesh->appendCell(cell);
    cell[0] = 3;    cell[1] = 2;    cell[2] = 4;
    surface_mesh->appendCell(cell);
    cell[0] = 1;    cell[1] = 4;    cell[2] = 2;
    surface_mesh->appendCell(cell);
    cell[0] = 0;    cell[1] = 0;    cell[2] = 0;
    surface_mesh->appendCell(cell);
    cell[0] = 3;    cell[1] = 4;    cell[2] = 3;
    surface_mesh->appendCell(cell);

    intersections.clear();
    intersections.push_back(std::make_pair(0, 1));
    intersections.push_back(std::make_pair(0, 2));
    degenerate.clear();
    degenerate.push_back(4);
    degenerate.push_back(5);
    runIntersectTest(testdescription,
                     surface_mesh, intersections, degenerate);
    delete surface_mesh;
  }
}

TEST( quest_mesh_tester, surfacemesh_self_intersection_ondisk )
{
  std::vector<std::string> tests = findIntersectTests();

  if (tests.size() < 1)
  {
    SLIC_INFO("*** No surface mesh self intersection tests found.");

    SUCCEED();
  }

  std::vector<std::string>::iterator it = tests.begin();
  for ( ; it != tests.end() ; ++it)
  {
    std::string & test = *it;
    if (!axom::utilities::filesystem::pathExists(test))
    {
      SLIC_INFO("Test file does not exist; skipping: " << test);
    }
    else
    {
      std::vector< std::pair<int, int> > expisect;
      std::vector< int > expdegen;
      std::string tfname;
      std::string tname = readIntersectTest(test, tfname, expisect, expdegen);

      // read in the test file into a Mesh
      axom::quest::STLReader reader;
      reader.setFileName( tfname );
      reader.read();

      // Get surface mesh
      UMesh* surface_mesh = new UMesh( 3, axom::mint::TRIANGLE );
      reader.getMesh( surface_mesh );

      runIntersectTest(tname, surface_mesh, expisect, expdegen);
      delete surface_mesh;
    }
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
#include "slic/UnitTestLogger.hpp"
using axom::slic::UnitTestLogger;

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  UnitTestLogger logger;  // create & initialize test logger,
  axom::slic::setLoggingMsgLevel(axom::slic::message::Info);

  int result = RUN_ALL_TESTS();
  return result;
}
