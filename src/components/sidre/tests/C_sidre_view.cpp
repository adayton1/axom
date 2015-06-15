/*
 * Copyright (c) 2015, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * All rights reserved.
 *
 * This source code cannot be distributed without permission and
 * further review from Lawrence Livermore National Laboratory.
 */

#include "gtest/gtest.h"

#include "sidre/sidre.h"

//------------------------------------------------------------------------------

TEST(C_sidre_view,create_views)
{
  ATK_datastore * ds   = ATK_datastore_new();
  ATK_datagroup * root = ATK_datastore_get_root(ds);

  ATK_dataview * dv_0 = ATK_datagroup_create_view_and_buffer_simple(root, "field0");
  ATK_dataview * dv_1 = ATK_datagroup_create_view_and_buffer_simple(root, "field1");


  ATK_databuffer * db_0 = ATK_dataview_get_buffer(dv_0);
  ATK_databuffer * db_1 = ATK_dataview_get_buffer(dv_1);

  EXPECT_EQ(ATK_databuffer_get_index(db_0), 0);
  EXPECT_EQ(ATK_databuffer_get_index(db_1), 1);
  ATK_datastore_delete(ds);
}

//------------------------------------------------------------------------------

TEST(C_sidre_view,int_buffer_from_view)
{
  ATK_datastore * ds = ATK_datastore_new();
  ATK_datagroup * root = ATK_datastore_get_root(ds);

  ATK_dataview * dv = ATK_datagroup_create_view_and_buffer_simple(root, "u0");

  ATK_dataview_allocate(dv, ATK_C_INT_T, 10);
  int * data_ptr = (int *) ATK_dataview_get_data_buffer(dv);

  for(int i=0 ; i<10 ; i++) {
    data_ptr[i] = i*i;
  }

#ifdef XXX
  dv->getNode().print_detailed();
#endif

  EXPECT_EQ(ATK_dataview_get_total_bytes(dv), sizeof(int) * 10);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,int_buffer_from_view_conduit_value)
{
  ATK_datastore * ds = ATK_datastore_new();
  ATK_datagroup * root = ATK_datastore_get_root(ds);

  ATK_dataview * dv = ATK_datagroup_create_view_and_buffer_from_type(root, "u0", ATK_C_INT_T, 10);
  int * data_ptr = (int *) ATK_dataview_get_data_buffer(dv);

  for(int i=0 ; i<10 ; i++) {
    data_ptr[i] = i*i;
  }

#ifdef XXX
  dv->getNode().print_detailed();
#endif

  EXPECT_EQ(ATK_dataview_get_total_bytes(dv), sizeof(int) * 10);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,int_array_multi_view)
{
  ATK_datastore * ds = ATK_datastore_new();
  ATK_datagroup * root = ATK_datastore_get_root(ds);
  ATK_databuffer * dbuff = ATK_datastore_create_buffer(ds);

  ATK_databuffer_declare(dbuff, ATK_C_INT_T, 10);
  ATK_databuffer_allocate_existing(dbuff);
  int * data_ptr = (int *) ATK_databuffer_get_data(dbuff);

  for(int i=0 ; i<10 ; i++) {
    data_ptr[i] = i;
  }

#ifdef XXX
  dbuff->getNode().print_detailed();
#endif

  EXPECT_EQ(ATK_databuffer_get_total_bytes(dbuff), sizeof(int) * 10);


  ATK_dataview * dv_e = ATK_datagroup_create_view(root, "even", dbuff);
  ATK_dataview * dv_o = ATK_datagroup_create_view(root, "odd", dbuff);
  EXPECT_TRUE(dv_e != NULL);
  EXPECT_TRUE(dv_o != NULL);

#ifdef XXX
  dv_e->apply(DataType::uint32(5,0,8));

  dv_o->apply(DataType::uint32(5,4,8));

  dv_e->getNode().print_detailed();
  dv_o->getNode().print_detailed();

  uint32_array dv_e_ptr = dv_e->getNode().as_uint32_array();
  uint32_array dv_o_ptr = dv_o->getNode().as_uint32_array();
  for(int i=0 ; i<5 ; i++)
  {
    std::cout << "idx:" <<  i
              << " e:" << dv_e_ptr[i]
              << " o:" << dv_o_ptr[i]
              << " em:" << dv_e_ptr[i]  % 2
              << " om:" << dv_o_ptr[i]  % 2
              << std::endl;

    EXPECT_EQ(dv_e_ptr[i] % 2, 0u);
    EXPECT_EQ(dv_o_ptr[i] % 2, 1u);
  }
#endif
  ATK_datastore_print(ds);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,init_int_array_multi_view)
{
  ATK_datastore * ds = ATK_datastore_new();
  ATK_datagroup * root = ATK_datastore_get_root(ds);
  ATK_databuffer * dbuff = ATK_datastore_create_buffer(ds);

  ATK_databuffer_allocate_from_type(dbuff, ATK_C_INT_T, 10);
  int * data_ptr = (int *) ATK_databuffer_get_data(dbuff);

  for(int i=0 ; i<10 ; i++) {
    data_ptr[i] = i;
  }

#ifdef XXX
  dbuff->getNode().print_detailed();
#endif
  EXPECT_EQ(ATK_databuffer_get_total_bytes(dbuff), sizeof(int) * 10);


  ATK_dataview * dv_e = ATK_datagroup_create_view(root, "even",dbuff);
  ATK_dataview * dv_o = ATK_datagroup_create_view(root, "odd",dbuff);
  EXPECT_TRUE(dv_e != NULL);
  EXPECT_TRUE(dv_o != NULL);

#ifdef XXX
  // uint32(num_elems, offset, stride)
  dv_e->apply(DataType::uint32(5,0,8));


  // uint32(num_elems, offset, stride)
  dv_o->apply(DataType::uint32(5,4,8));


  dv_e->getNode().print_detailed();
  dv_o->getNode().print_detailed();

  uint32_array dv_e_ptr = dv_e->getNode().as_uint32_array();
  uint32_array dv_o_ptr = dv_o->getNode().as_uint32_array();
  for(int i=0 ; i<5 ; i++)
  {
    std::cout << "idx:" <<  i
              << " e:" << dv_e_ptr[i]
              << " o:" << dv_o_ptr[i]
              << " em:" << dv_e_ptr[i]  % 2
              << " om:" << dv_o_ptr[i]  % 2
              << std::endl;

    EXPECT_EQ(dv_e_ptr[i] % 2, 0u);
    EXPECT_EQ(dv_o_ptr[i] % 2, 1u);
  }
#endif

  ATK_datastore_print(ds);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,int_array_multi_view_resize)
{
  ///
  /// This example creates a 4 * 10 buffer of ints,
  /// and 4 views that point the 4 sections of 10 ints
  ///
  /// We then create a new buffer to support 4*12 ints
  /// and 4 views that point into them
  ///
  /// after this we use the old buffers to copy the values
  /// into the new views
  ///

  // create our main data store
  ATK_datastore * ds = ATK_datastore_new();
  // get access to our root data Group
  ATK_datagroup * root = ATK_datastore_get_root(ds);

  // create a group to hold the "old" or data we want to copy
  ATK_datagroup * r_old = ATK_datagroup_create_group(root, "r_old");
  // create a view to hold the base buffer
  ATK_dataview * base_old = ATK_datagroup_create_view_and_buffer_simple(r_old, "base_data");

  // alloc our buffer
  // we will create 4 sub views of this array
  ATK_dataview_allocate(base_old, ATK_C_INT_T, 40);
  int * data_ptr = (int *) ATK_dataview_get_data_buffer(base_old);


  // init the buff with values that align with the
  // 4 subsections.
  for(int i=0 ; i<10 ; i++) {
    data_ptr[i] = 1;
  }
  for(int i=10 ; i<20 ; i++) {
    data_ptr[i] = 2;
  }
  for(int i=20 ; i<30 ; i++) {
    data_ptr[i] = 3;
  }
  for(int i=30 ; i<40 ; i++) {
    data_ptr[i] = 4;
  }

#ifdef XXX
  /// setup our 4 views
  ATK_databuffer * buff_old = ATK_dataview_get_buffer(base_old);
  buff_old->getNode().print();
  ATK_dataview * r0_old = ATK_dataview_create_view(r_old, "r0",buff_old);
  ATK_dataview * r1_old = ATK_dataview_create_view(r_old, "r1",buff_old);
  ATK_dataview * r2_old = ATK_dataview_create_view(r_old, "r2",buff_old);
  ATK_dataview * r3_old = ATK_dataview_create_view(r_old, "r3",buff_old);

  // each view is offset by 10 * the # of bytes in a uint32
  // uint32(num_elems, offset)
  index_t offset =0;
  r0_old->apply(r0_old, DataType::uint32(10,offset));

  offset += sizeof(int) * 10;
  r1_old->apply(r1_old, DataType::uint32(10,offset));

  offset += sizeof(int) * 10;
  r2_old->apply(r2_old, DataType::uint32(10,offset));

  offset += sizeof(int) * 10;
  r3_old->apply(r3_old, DataType::uint32(10,offset));

  /// check that our views actually point to the expected data
  //
  uint32 * r0_ptr = r0_old->getNode().as_uint32_ptr();
  for(int i=0 ; i<10 ; i++)
  {
    EXPECT_EQ(r0_ptr[i], 1u);
    // check pointer relation
    EXPECT_EQ(&r0_ptr[i], &data_ptr[i]);
  }

  uint32 * r3_ptr = r3_old->getNode().as_uint32_ptr();
  for(int i=0 ; i<10 ; i++)
  {
    EXPECT_EQ(r3_ptr[i], 4u);
    // check pointer relation
    EXPECT_EQ(&r3_ptr[i], &data_ptr[i+30]);
  }

  // create a group to hold the "old" or data we want to copy into
  ATK_datagroup * r_new = ATK_datagroup_create_group(root, "r_new");
  // create a view to hold the base buffer
  ATK_dataview * base_new = ATK_datagroup_create_view_and_buffer_simple(r_new, "base_data");

  // alloc our buffer
  // create a buffer to hold larger subarrays
  base_new->allocate(base_new, DataType::uint32(4 * 12));
  int* base_new_data = (int *) ATK_databuffer_det_data(base_new);
  for (int i = 0; i < 4 * 12; ++i) 
  {
     base_new_data[i] = 0;
  } 

  ATK_databuffer * buff_new = ATK_dataview_get_buffer(base_new);
  buff_new->getNode().print();

  // create the 4 sub views of this array
  ATK_dataview * r0_new = ATK_datagroup_create_view(r_new, "r0",buff_new);
  ATK_dataview * r1_new = ATK_datagroup_create_view(r_new, "r1",buff_new);
  ATK_dataview * r2_new = ATK_datagroup_create_view(r_new, "r2",buff_new);
  ATK_dataview * r3_new = ATK_datagroup_create_view(r_new, "r3",buff_new);

  // apply views to r0,r1,r2,r3
  // each view is offset by 12 * the # of bytes in a uint32

  // uint32(num_elems, offset)
  offset =0;
  r0_new->apply(DataType::uint32(12,offset));

  offset += sizeof(int) * 12;
  r1_new->apply(DataType::uint32(12,offset));

  offset += sizeof(int) * 12;
  r2_new->apply(DataType::uint32(12,offset));

  offset += sizeof(int) * 12;
  r3_new->apply(DataType::uint32(12,offset));

  /// update r2 as an example first
  buff_new->getNode().print();
  r2_new->getNode().print();

  /// copy the subset of value
  r2_new->getNode().update(r2_old->getNode());
  r2_new->getNode().print();
  buff_new->getNode().print();


  /// check pointer values
  int * r2_new_ptr = (int *) ATK_dataview_get_data_buffer(r2_new);

  for(int i=0 ; i<10 ; i++)
  {
    EXPECT_EQ(r2_new_ptr[i], 3);
  }

  for(int i=10 ; i<12 ; i++)
  {
    EXPECT_EQ(r2_new_ptr[i], 0);     // assumes zero-ed alloc
  }


  /// update the other views
  r0_new->getNode().update(r0_old->getNode());
  r1_new->getNode().update(r1_old->getNode());
  r3_new->getNode().update(r3_old->getNode());

  buff_new->getNode().print();
#endif

  ATK_datastore_print(ds);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,int_array_realloc)
{
  ///
  /// info
  ///

  // create our main data store
  ATK_datastore * ds = ATK_datastore_new();
  // get access to our root data Group
  ATK_datagroup * root = ATK_datastore_get_root(ds);

  // create a view to hold the base buffer
  ATK_dataview * a1 = ATK_datagroup_create_view_and_buffer_from_type(root, "a1", ATK_C_FLOAT_T, 5);
  ATK_dataview * a2 = ATK_datagroup_create_view_and_buffer_from_type(root, "a2", ATK_C_INT_T, 5);

  float * a1_ptr = (float *) ATK_dataview_get_data_buffer(a1);
  int * a2_ptr = (int *)  ATK_dataview_get_data_buffer(a2);

  for(int i=0 ; i<5 ; i++)
  {
    a1_ptr[i] =  5.0;
    a2_ptr[i] = -5;
  }

  EXPECT_EQ(ATK_dataview_get_total_bytes(a1), sizeof(float)*5);
  EXPECT_EQ(ATK_dataview_get_total_bytes(a2), sizeof(int)*5);


  ATK_dataview_reallocate(a1, ATK_C_FLOAT_T, 10);
  ATK_dataview_reallocate(a2, ATK_C_INT_T, 15);

  a1_ptr = (float *) ATK_dataview_get_data_buffer(a1);
  a2_ptr = (int *) ATK_dataview_get_data_buffer(a2);

  for(int i=0 ; i<5 ; i++)
  {
    EXPECT_EQ(a1_ptr[i],5.0);
    EXPECT_EQ(a2_ptr[i],-5);
  }

  for(int i=5 ; i<10 ; i++)
  {
    a1_ptr[i] = 10.0;
    a2_ptr[i] = -10;
  }

  for(int i=10 ; i<15 ; i++)
  {
    a2_ptr[i] = -15;
  }

  EXPECT_EQ(ATK_dataview_get_total_bytes(a1), sizeof(float)*10);
  EXPECT_EQ(ATK_dataview_get_total_bytes(a2), sizeof(int)*15);


  ATK_datastore_print(ds);
  ATK_datastore_delete(ds);

}

//------------------------------------------------------------------------------

TEST(C_sidre_view,simple_opaque)
{
  // create our main data store
  ATK_datastore * ds = ATK_datastore_new();
  // get access to our root data Group
  ATK_datagroup * root = ATK_datastore_get_root(ds);
  int * src_data = (int *) malloc(sizeof(int));

  src_data[0] = 42;

  void * src_ptr = (void *)src_data;

  ATK_dataview * opq_view = ATK_datagroup_create_opaque_view(root, "my_opaque",src_ptr);

  // we shouldn't have any buffers
  EXPECT_EQ(ATK_datastore_get_num_buffers(ds), 0u);

  EXPECT_TRUE(ATK_dataview_is_opaque(opq_view));

  void * opq_ptr = ATK_dataview_get_opaque(opq_view);

  int * out_data = (int *)opq_ptr;
  EXPECT_EQ(opq_ptr,src_ptr);
  EXPECT_EQ(out_data[0],42);

  ATK_datastore_print(ds);
  ATK_datastore_delete(ds);
  free(src_data);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
#include "slic/UnitTestLogger.hpp"
using asctoolkit::slic::UnitTestLogger;

int main(int argc, char* argv[])
{
   int result = 0;

   ::testing::InitGoogleTest(&argc, argv);

   UnitTestLogger logger;  // create & initialize test logger,
                       // finalized when exiting main scope

   result = RUN_ALL_TESTS();

   return result;
}
