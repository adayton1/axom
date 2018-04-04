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

#include "mint/Mesh.hpp"

// axom includes
#include "axom/Types.hpp"
#include "mint/FieldData.hpp"

#ifdef MINT_USE_SIDRE
#include "sidre/sidre.hpp"
#endif

namespace axom
{
namespace mint
{


//------------------------------------------------------------------------------
Mesh::Mesh( int ndims, int type, int blockId, int partId ) :
  m_ndims( ndims ),
  m_type( type ),
  m_block_idx( blockId ),
  m_part_idx( partId ),
  m_num_cells( 0 ),
  m_num_faces( 0 ),
  m_num_edges( 0 ),
  m_num_nodes( 0 ),
  m_coordinates( AXOM_NULLPTR ),
#ifdef MINT_USE_SIDRE
  m_group( AXOM_NULLPTR ),
  m_fields_group( AXOM_NULLPTR ),
  m_coordsets_group( AXOM_NULLPTR ),
  m_topologies_group( AXOM_NULLPTR )
#endif
{
  SLIC_ERROR_IF( m_ndims < 0 || m_ndims > 3, "invalid dimension" );
}

#ifdef MINT_USE_SIDRE
//------------------------------------------------------------------------------
Mesh::Mesh( sidre::Group* group ) :
  m_ndims( 0 ),
  m_type( 0 ),
  m_block_idx( 0 ),
  m_part_idx( 0 ),
  m_num_cells( 0 ),
  m_num_faces( 0 ),
  m_num_edges( 0 ),
  m_num_nodes( 0 ),
  m_group( group )
{
//  SLIC_ERROR_IF( m_ndims < 0 || m_ndims > 3, "");
//  SLIC_ERROR_IF( m_group == AXOM_NULLPTR, "");
//  SLIC_ERROR_IF( m_group->getNumGroups() == 0, "");
//  SLIC_ERROR_IF( m_group->getNumViews() == 0, "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "ndims" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "type" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "block_idx" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "part_idx" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "num_cells" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "cell_resize_ratio" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "num_faces" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "face_resize_ratio" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "num_edges" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "edge_resize_ratio" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "num_nodes" ), "");
//  SLIC_ERROR_IF( !m_group->hasChildView( "node_resize_ratio" ), "");
//
//  sidre::View* view = m_group->getView( "ndims" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_ndims = view->getData();
//
//  view = m_group->getView( "type" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_type = view->getData();
//
//  view = m_group->getView( "block_idx" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_block_idx = view->getData();
//
//  view = m_group->getView( "part_idx" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_part_idx = view->getData();
//
//  view = m_group->getView( "num_cells" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_num_cells = static_cast< IndexType* >( view->getVoidPtr() );
//
//  view = m_group->getView( "cell_resize_ratio" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_cell_resize_ratio = static_cast< double* >( view->getVoidPtr() );
//
//  view = m_group->getView( "num_faces" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_num_faces = static_cast< IndexType* >( view->getVoidPtr() );
//
//  view = m_group->getView( "face_resize_ratio" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_face_resize_ratio = static_cast< double* >( view->getVoidPtr() );
//
//  view = m_group->getView( "num_edges" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_num_edges = static_cast< IndexType* >( view->getVoidPtr() );
//
//  view = m_group->getView( "edge_resize_ratio" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_edge_resize_ratio = static_cast< double* >( view->getVoidPtr() );
//
//  view = m_group->getView( "num_nodes" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_num_nodes = static_cast< IndexType* >( view->getVoidPtr() );
//
//  view = m_group->getView( "node_resize_ratio" );
//  SLIC_ERROR_IF( !view->isScalar(), "" );
//  m_node_resize_ratio = static_cast< double* >( view->getVoidPtr() );
}

//------------------------------------------------------------------------------
Mesh::Mesh( sidre::Group* group, int ndims, int type, int blockId,
            int partId ) :
  m_ndims( ndims ),
  m_type( type ),
  m_block_idx( blockId ),
  m_part_idx( partId ),
  m_num_cells( 0 ),
  m_num_faces( 0),
  m_num_edges( 0 ),
  m_num_nodes( 0 ),
  m_coordinates( AXOM_NULLPTR ),
  m_group( group )
{

// TODO: implement this

//  SLIC_ERROR_IF( m_ndims < 0 || m_ndims > 3, "" );
//  SLIC_ERROR_IF( m_group == AXOM_NULLPTR, "" );
//  SLIC_ERROR_IF( m_group->getNumGroups() != 0, "" );
//  SLIC_ERROR_IF( m_group->getNumViews() != 0, "" );
//
//  m_group->createView( "ndims" )->setScalar( m_ndims );
//  m_group->createView( "type" )->setScalar( m_type );
//  m_group->createView( "block_idx" )->setScalar( m_block_idx );
//  m_group->createView( "part_idx" )->setScalar( m_part_idx );
//
//  IndexType zero = 0;
//  double zero_f = 0.0;
//  m_num_cells = static_cast< IndexType* >(
//    m_group->createView( "num_cells" )
//    ->setScalar( zero )->getVoidPtr() );
//  m_cell_resize_ratio = static_cast< double* >(
//    m_group->createView( "cell_resize_ratio")
//    ->setScalar( zero_f )->getVoidPtr() );
//  m_num_faces = static_cast< IndexType* >(
//    m_group->createView( "num_faces" )
//    ->setScalar( zero )->getVoidPtr() );
//  m_face_resize_ratio = static_cast< double* >(
//    m_group->createView( "face_resize_ratio")
//    ->setScalar( zero_f )->getVoidPtr() );
//  m_num_edges = static_cast< IndexType* >(
//    m_group->createView( "num_edges" )
//    ->setScalar( zero )->getVoidPtr() );
//  m_edge_resize_ratio = static_cast< double* >(
//    m_group->createView( "edge_resize_ratio")
//    ->setScalar( zero_f )->getVoidPtr() );
//  m_num_nodes = static_cast< IndexType* >(
//    m_group->createView( "num_nodes" )
//    ->setScalar( zero )->getVoidPtr() );
//  m_node_resize_ratio = static_cast< double* >(
//    m_group->createView( "node_resize_ratio")
//    ->setScalar( zero_f )->getVoidPtr() );
}
#endif

//------------------------------------------------------------------------------
Mesh::~Mesh()
{

  if ( m_coordinates != AXOM_NULLPTR )
  {
    delete m_coordinates;
    m_coordinates = AXOM_NULLPTR;
  }

// TODO: implement this

//#ifdef MINT_USE_SIDRE
//  if ( m_group != AXOM_NULLPTR )
//  {
//    m_group = AXOM_NULLPTR;
//    m_num_cells = AXOM_NULLPTR;
//    m_cell_capacity = AXOM_NULLPTR;
//    m_cell_resize_ratio = AXOM_NULLPTR;
//    m_num_faces = AXOM_NULLPTR;
//    m_face_capacity = AXOM_NULLPTR;
//    m_face_resize_ratio = AXOM_NULLPTR;
//    m_num_edges = AXOM_NULLPTR;
//    m_edge_capacity = AXOM_NULLPTR;
//    m_edge_resize_ratio = AXOM_NULLPTR;
//    m_num_nodes = AXOM_NULLPTR;
//    m_node_capacity = AXOM_NULLPTR;
//    m_node_resize_ratio = AXOM_NULLPTR;
//    return;
//  }
//#endif
//
//  delete m_num_cells;
//  delete m_cell_capacity;
//  delete m_cell_resize_ratio;
//
//  delete m_num_faces;
//  delete m_face_capacity;
//  delete m_face_resize_ratio;
//
//  delete m_num_edges;
//  delete m_edge_capacity;
//  delete m_edge_resize_ratio;
//
//  delete m_num_nodes;
//  delete m_node_capacity;
//  delete m_node_resize_ratio;
}

//------------------------------------------------------------------------------
void Mesh::getMeshNode( IndexType nodeIdx, double* node ) const
{
  // TODO: implement this
}

//------------------------------------------------------------------------------
void Mesh::getMeshCell( IndexType cellIdx, IndexType* cell ) const
{
  // TODO: implement this
}

//------------------------------------------------------------------------------
int Mesh::getMeshCellType( IndexType cellIdx ) const
{
  // TODO: implement this
  return -1;
}

//------------------------------------------------------------------------------
void Mesh::allocateFieldData( )
{
#ifdef MINT_USE_SIDRE

  if ( hasSidreGroup() )
  {
    sidre::Group* fields_group = ( m_group->hasChildGroup("fields") ?
            m_group->getGroup( "fields") : m_group->createGroup( "fields") );
    SLIC_ASSERT( fields_group != AXOM_NULLPTR );
    SLIC_ASSERT( fields_group->getParent()==m_group );

    for ( int i=0; i < NUM_FIELD_ASSOCIATIONS; ++i )
    {
      m_mesh_fields[ i ] = new mint::FieldData( i, fields_group );
    }
  }
  else
  {
    for ( int i=0; i < NUM_FIELD_ASSOCIATIONS; ++i )
    {
       m_mesh_fields[ i ] = new mint::FieldData( i );
    }
  }

#else

  for ( int i=0; i < NUM_FIELD_ASSOCIATIONS; ++i )
  {
    m_mesh_fields[ i ] = new mint::FieldData( i );
  }

#endif

}

//------------------------------------------------------------------------------
void Mesh::deallocateFieldData( )
{

  for ( int i=0; i < NUM_FIELD_ASSOCIATIONS; ++i )
  {
    SLIC_ASSERT( m_mesh_fields[ i ] != AXOM_NULLPTR );

    delete m_mesh_fields[ i ];
    m_mesh_fields[ i ] = AXOM_NULLPTR;
  }

}

} /* namespace mint */
} /* namespace axom */
