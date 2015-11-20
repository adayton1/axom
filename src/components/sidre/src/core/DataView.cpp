/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * All rights reserved.
 *
 * This source code cannot be distributed without permission and
 * further review from Lawrence Livermore National Laboratory.
 */

/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Implementation file for DataView class.
 *
 ******************************************************************************
 */


// Associated header file
#include "DataView.hpp"

// Other toolkit project headers
#include "common/CommonTypes.hpp"
#include "slic/slic.hpp"

// SiDRe project headers
#include "DataBuffer.hpp"
#include "DataGroup.hpp"
#include "DataStore.hpp"


namespace asctoolkit
{
namespace sidre
{


/*
 *************************************************************************
 *
 * Declare data view with type and number of elements.
 *
 *************************************************************************
 */
DataView * DataView::declare(TypeID type, SidreLength numelems)
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call declare on an opaque view");
  SLIC_ASSERT_MSG(numelems >= 0, "Must declare number of elements >= 0");

  if ( !isOpaque() && numelems >= 0 ) 
  {
    DataType dtype = conduit::DataType::default_dtype(type);
    dtype.set_number_of_elements(numelems);

    m_schema.set(dtype);
    m_is_applied = false;
  }
  return this;
}

/*
 *************************************************************************
 *
 * Declare data view with a Conduit data type object.
 *
 *************************************************************************
 */
DataView * DataView::declare(const DataType& dtype)
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call declare on an opaque view");

  if ( !isOpaque() ) 
  {
    m_schema.set(dtype);
    m_is_applied = false;
  }
  return this;
}

/*
 *************************************************************************
 *
 * Declare data view with a Conduit schema object.
 *
 *************************************************************************
 */
DataView * DataView::declare(const Schema& schema)
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call declare on an opaque view");

  if ( !isOpaque() ) 
  {
    m_schema.set(schema);
    m_is_applied = false;
  }
  return this;
}

/*
 *************************************************************************
 *
 * Allocate data for view, previously declared.
 *
 *************************************************************************
 */
DataView * DataView::allocate()
{
  SLIC_ASSERT( allocationIsValid() );

  if ( allocationIsValid() ) 
  {
    if ( m_data_buffer == ATK_NULLPTR ) 
    {
       m_data_buffer = m_owning_group->getDataStore()->createBuffer();
       m_data_buffer->attachView(this);
    }

    if ( m_data_buffer->getNumViews() == 1 )
    {
      TypeID type = static_cast<TypeID>(m_schema.dtype().id());
      SidreLength numelems = m_schema.dtype().number_of_elements();
      m_data_buffer->allocate(type, numelems);
      apply();  
    }
  } 

  return this;
}

/*
 *************************************************************************
 *
 * Allocate data for view with type and number of elements.
 *
 *************************************************************************
 */
DataView * DataView::allocate( TypeID type, SidreLength numelems)
{
  SLIC_ASSERT( allocationIsValid() ); 
  SLIC_ASSERT_MSG(numelems >= 0, "Must allocate number of elements >= 0");

  if ( allocationIsValid() && numelems >= 0 )
  {
    declare(type, numelems);
    allocate();
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Allocate data for view described by a Conduit data type object.
 *
 *************************************************************************
 */
DataView * DataView::allocate(const DataType& dtype)
{
  SLIC_ASSERT( allocationIsValid() );

  if ( allocationIsValid() )
  {
    declare(dtype);
    allocate();
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Allocate data for view described by a Conduit schema object.
 *
 *************************************************************************
 */
DataView * DataView::allocate(const Schema& schema)
{
  SLIC_ASSERT( allocationIsValid() ); 

  if ( allocationIsValid() )
  {
    declare(schema);
    allocate();
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Reallocate data for view to given number of elements.
 *
 *************************************************************************
 */
DataView * DataView::reallocate(SidreLength numelems)
{
  SLIC_ASSERT( allocationIsValid() ); 
  SLIC_ASSERT_MSG(numelems >= 0, "Must re-allocate number of elements >= 0");

  if ( allocationIsValid() && numelems >= 0 )
  {
    // preserve current type
    TypeID vtype = static_cast<TypeID>(m_schema.dtype().id());
    declare(vtype, numelems);
    m_data_buffer->reallocate(numelems);
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Reallocate data for view using a Conduit data type object.
 *
 *************************************************************************
 */
DataView * DataView::reallocate(const DataType& dtype)
{
  SLIC_ASSERT( allocationIsValid() ); 

  if ( allocationIsValid() )
  {
    TypeID type = static_cast<TypeID>(dtype.id());
    TypeID view_type = static_cast<TypeID>(m_schema.dtype().id());
    SLIC_ASSERT_MSG( type == view_type,
		     "Attempting to reallocate with a different type");
    if (type == view_type)
    {
      declare(dtype);
      SidreLength numelems = dtype.number_of_elements();
      m_data_buffer->reallocate(numelems);
      apply();
    }
  }
  return this;
}

/*
 *************************************************************************
 *
 * Reallocate data for view using a Conduit schema object.
 *
 *************************************************************************
 */
DataView * DataView::reallocate(const Schema& schema)
{
  SLIC_ASSERT( allocationIsValid() ); 

  if ( allocationIsValid() )
  {
    TypeID type = static_cast<TypeID>(schema.dtype().id());
    TypeID view_type = static_cast<TypeID>(m_schema.dtype().id());
    SLIC_ASSERT_MSG( type == view_type,
		     "Attempting to reallocate with a different type");
    if (type == view_type)
    {
      declare(schema);
      SidreLength numelems = schema.dtype().number_of_elements();
      m_data_buffer->reallocate(numelems);
      apply();
    }
  }
  return this;
}

/*
 *************************************************************************
 *
 * Apply a previously declared data view to data held in the buffer.
 *
 *************************************************************************
 */
DataView * DataView::apply()
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call apply() on an opaque view");
 
  if ( !isOpaque() )
  { 
    m_node.set_external(m_schema, m_data_buffer->getData());
    m_is_applied = true;
  }
  return this;
}

/*
 *************************************************************************
 *
 * Apply a Consuit data type description to data held in the buffer.
 *
 *************************************************************************
 */
DataView * DataView::apply(const DataType &dtype)
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call apply() on an opaque view");

  if ( !isOpaque() )
  {
    declare(dtype);
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Apply a Conduit Schema to data held in the buffer.
 *
 *************************************************************************
 */
DataView * DataView::apply(const Schema& schema)
{
  SLIC_ASSERT_MSG( !isOpaque(),
                  "Cannot call apply() on an opaque view");
 
  if ( !isOpaque() )
  { 
    declare(schema);
    apply();
  }
  return this;
}

/*
 *************************************************************************
 *
 * Return void* pointer to buffer data.
 *
 *************************************************************************
 */
void * DataView::getDataPointer() const
{
  if ( isOpaque() ) {
      return (void *)(getNode().as_uint64());
  } else {
      return m_data_buffer->getData();
  }
}

/*
 *************************************************************************
 *
 * Return void* pointer to opaque data.
 *
 *************************************************************************
 */
void * DataView::getOpaque() const
{
  if ( isOpaque() ) 
  {
    return (void *)(getNode().as_uint64());
  } 
  else 
  {
    return ATK_NULLPTR; 
  }
}


/*
 *************************************************************************
 *
 * Copy data view description to given Conduit node.
 *
 *************************************************************************
 */
void DataView::info(Node &n) const
{
  n["name"] = m_name;
  n["schema"] = m_schema.to_json();
  n["node"] = m_node.to_json();
  n["is_opaque"] = m_is_opaque;
  n["is_applied"] = m_is_applied;
}


/*
 *************************************************************************
 *
 * Print JSON description of data view to stdout.
 *
 *************************************************************************
 */
void DataView::print() const
{
  print(std::cout);
}

/*
 *************************************************************************
 *
 * Print JSON description of data view to an  ostream.
 *
 *************************************************************************
 */
void DataView::print(std::ostream& os) const
{
  Node n;
  info(n);
  n.to_json_stream(os);
}

/*
 *************************************************************************
 *
 * PRIVATE ctor for DataView not associated with any data. 
 *
 *************************************************************************
 */
DataView::DataView( const std::string& name,
                    DataGroup * const owning_group)
  :   m_name(name),
  m_owning_group(owning_group),
  m_data_buffer(ATK_NULLPTR),
  m_schema(),
  m_node(),
  m_is_opaque(false),
  m_is_applied(false)
{}

/*
 *************************************************************************
 *
 * PRIVATE ctor for DataView associated with DataBuffer.
 *
 *************************************************************************
 */
DataView::DataView( const std::string& name,
                    DataGroup * const owning_group,
                    DataBuffer * const data_buffer)
  :   m_name(name),
  m_owning_group(owning_group),
  m_data_buffer(data_buffer),
  m_schema(),
  m_node(),
  m_is_opaque(false),
  m_is_applied(false)
{}

/*
 *************************************************************************
 *
 * PRIVATE ctor for DataView associated with opaque data.
 *
 *************************************************************************
 */
DataView::DataView( const std::string& name,
                    DataGroup * const owning_group,
                    void * opaque_ptr)
  : m_name(name),
  m_owning_group(owning_group),
  m_data_buffer(ATK_NULLPTR),
  m_schema(),
  m_node(),
  m_is_opaque(true),
  m_is_applied(false)
{
  // todo, conduit should provide a check for if uint64 is a
  // good enough type to rep void *
  m_node.set((conduit::uint64)opaque_ptr);
}

/*
 *************************************************************************
 *
 * PRIVATE dtor.
 *
 *************************************************************************
 */
DataView::~DataView()
{
  if (m_data_buffer != ATK_NULLPTR)
  {
    m_data_buffer->detachView(this);
  }
}

/*
 *************************************************************************
 *
 * PRIVATE method to check whether allocation on view is a valid operation.
 *
 *************************************************************************
 */
bool DataView::allocationIsValid() const
{
   return ( !isOpaque() && (m_data_buffer == ATK_NULLPTR || 
                            m_data_buffer->getNumViews() == 1) );
}


} /* end namespace sidre */
} /* end namespace asctoolkit */
