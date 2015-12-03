#
# Copyright (c) 2015, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# All rights reserved.
#
# This source code cannot be distributed without permission and
# further review from Lawrence Livermore National Laboratory.
# 

# API coverage tests
# Each test should be documented with the interface functions being tested

import unittest
import sidre
from sidre import InvalidIndex, nameIsValid

class SidreGroup(unittest.TestCase):
#------------------------------------------------------------------------------
# getName()
#------------------------------------------------------------------------------
    def test_get_name(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        group = root.createGroup("test")

        self.assertTrue(group.getName() == "test" )

        group2 = root.getGroup("foo")
        self.assertTrue(group2 == None)

        ds.delete()

#------------------------------------------------------------------------------
# getParent()
#------------------------------------------------------------------------------
    def test_get_parent(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        parent = root.createGroup("parent")
        child = parent.createGroup("child")

        self.assertTrue( child.getParent() == parent )

        ds.delete()

#------------------------------------------------------------------------------
# Verify getDatastore()
#------------------------------------------------------------------------------
    def test_get_datastore(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        group = root.createGroup("parent")

        self.assertTrue( group.getDataStore() == ds )

        const_ds = group.getDataStore()
        self.assertTrue( const_ds == ds )

        ds.delete()

#------------------------------------------------------------------------------
# Verify hasGroup()
#------------------------------------------------------------------------------
    def test_has_group(self):
        ds = sidre.DataStore()
        root = ds.getRoot()

        parent = root.createGroup("parent")
        child = parent.createGroup("child")
        self.assertTrue( child.getParent() == parent )

        self.assertTrue( parent.hasGroup("child") )

        ds.delete()

#------------------------------------------------------------------------------
# Verify hasView()
#------------------------------------------------------------------------------
    def XXXtest_has_view(self):
        ds = sidre.DataStore()
        root = ds.getRoot()

        parent = root.createGroup("parent")
        view = parent.createViewAndBuffer("view")

        self.assertTrue( view.getOwningGroup() == parent )

        self.assertTrue( parent.hasView("view") )

        ds.delete()

#------------------------------------------------------------------------------
# Verify getViewName(), getViewIndex()
#------------------------------------------------------------------------------
    def test_get_view_name_index(self):
        ds = sidre.DataStore()
        root = ds.getRoot()

        parent = root.createGroup("parent")
        view1 = parent.createViewAndBuffer("view1")
        view2 = parent.createViewAndBuffer("view2")

        self.assertEqual(parent.getNumViews(), 2)

        idx1 = parent.getViewIndex("view1")
        idx2 = parent.getViewIndex("view2")

        name1 = parent.getViewName(idx1)
        name2 = parent.getViewName(idx2)

        self.assertEqual(name1, "view1")
        self.assertEqual(view1.getName(), name1)

        self.assertEqual(name2, "view2")
        self.assertEqual(view2.getName(), name2)

        idx3 = parent.getViewIndex("view3")
        self.assertTrue(idx3 == InvalidIndex)

        name3 = parent.getViewName(idx3)
        self.assertTrue(name3 is None)
        self.assertFalse(nameIsValid(name3))

        ds.delete()

#------------------------------------------------------------------------------
# Verify getGroupName(), getGroupIndex()
#------------------------------------------------------------------------------
    def test_get_group_name_index(self):
        ds = sidre.DataStore()
        root = ds.getRoot()

        parent = root.createGroup("parent")
        group1 = parent.createGroup("group1")
        group2 = parent.createGroup("group2")

        self.assertEqual(parent.getNumGroups(), 2)

        idx1 = parent.getGroupIndex("group1")
        idx2 = parent.getGroupIndex("group2")

        name1 = parent.getGroupName(idx1)
        name2 = parent.getGroupName(idx2)

        self.assertEqual(name1, "group1")
        self.assertEqual(group1.getName(), name1)

        self.assertEqual(name2, "group2")
        self.assertEqual(group2.getName(), name2)

        idx3 = parent.getGroupIndex("group3")
        self.assertTrue(idx3 == InvalidIndex)

        name3 = parent.getGroupName(idx3)
        self.assertTrue(name3 is None)
        self.assertFalse(nameIsValid(name3))

        ds.delete()

#------------------------------------------------------------------------------
# createViewAndBuffer()
# destroyViewAndBuffer()
# hasView()
#------------------------------------------------------------------------------
    def test_create_destroy_has_viewbuffer(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        group = root.createGroup("parent")

        view = group.createViewAndBuffer("view")
        self.assertTrue( group.getParent() == root )
        self.assertTrue( view.hasBuffer() )

        self.assertTrue( group.hasView("view") )

        group.destroyViewAndBuffer("view")

        self.assertFalse( group.hasView("view") )

        ds.delete()

#------------------------------------------------------------------------------
# createGroup()
# destroyGroup()
# hasGroup()
#------------------------------------------------------------------------------
    def test_create_destroy_has_group(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        group = root.createGroup("group")
        self.assertTrue( group.getParent() == root )

        self.assertTrue( root.hasGroup("group") )


        root.destroyGroup("group")
        self.assertFalse( root.hasGroup("group") )

        ds.delete()

#------------------------------------------------------------------------------
    def test_group_name_collisions(self):
        ds = sidre.DataStore()
        flds = ds.getRoot().createGroup("fields")
        flds.createViewAndBuffer("a")

        self.assertTrue(flds.hasView("a"))

        ds.delete()

#------------------------------------------------------------------------------
    def XXXtest_view_copy_move(self):
        ds = sidre.DataStore()
        flds = ds.getRoot().createGroup("fields")

        flds.createViewAndBuffer("i0").allocate(DataType.c_int())
        flds.createViewAndBuffer("f0").allocate(DataType.c_float())
        flds.createViewAndBuffer("d0").allocate(DataType.c_double())

        flds.getView("i0").setValue(1)
        flds.getView("f0").setValue(100.0)
        flds.getView("d0").setValue(3000.0)

        self.assertTrue(flds.hasView("i0"))
        self.assertTrue(flds.hasView("f0"))
        self.assertTrue(flds.hasView("d0"))

        # test moving a view from flds to sub
        flds.createGroup("sub").moveView(flds.getView("d0"))
        flds.print_json()
        self.assertFalse(flds.hasView("d0"))
        self.assertTrue(flds.hasGroup("sub"))
        self.assertTrue(flds.getGroup("sub").hasView("d0"))

        # check the data value
        d0_data =  flds.getGroup("sub").getView("d0").getValue()
        EXPECT_NEAR(d0_data[0],3000.0,1e-12)

        # test copying a view from flds to sub
        flds.getGroup("sub").copyView(flds.getView("i0"))

        flds.print_json()

        self.assertTrue(flds.hasView("i0"))
        self.assertTrue(flds.getGroup("sub").hasView("i0"))

        # we expect the actual data  pointers to be the same
        self.assertEqual(flds.getView("i0").getDataPointer(),
                         flds.getGroup("sub").getView("i0").getDataPointer())

        ds.delete()

#------------------------------------------------------------------------------
    def XXXtest_groups_move_copy(self):
        ds = sidre.DataStore()
        flds = ds.getRoot().createGroup("fields")

        ga = flds.createGroup("a")
        gb = flds.createGroup("b")
        gc = flds.createGroup("c")

        ga.createViewAndBuffer("i0").allocate(DataType.c_int())
        gb.createViewAndBuffer("f0").allocate(DataType.c_float())
        gc.createViewAndBuffer("d0").allocate(DataType.c_double())

        ga.getView("i0").setValue(1)
        gb.getView("f0").setValue(100.0)
        gc.getView("d0").setValue(3000.0)

        # check that all sub groups exist
        self.assertTrue(flds.hasGroup("a"))
        self.assertTrue(flds.hasGroup("b"))
        self.assertTrue(flds.hasGroup("c"))

        #move "b" to a child of "sub"
        flds.createGroup("sub").moveGroup(gb)

        flds.print_json()

        self.assertTrue(flds.hasGroup("a"))
        self.assertTrue(flds.hasGroup("sub"))
        self.assertTrue(flds.hasGroup("c"))

        self.assertEqual(flds.getGroup("sub").getGroup("b"),gb)

        ds.delete()

#------------------------------------------------------------------------------
    def XXXtest_create_destroy_view_and_buffer(self):
        ds = sidre.DataStore()
        grp = ds.getRoot().createGroup("grp")

        viewName1 = "viewBuffer1"
        viewName2 = "viewBuffer2"

        view1 = grp.createViewAndBuffer(viewName1)
        view2 = grp.createViewAndBuffer(viewName2)

        self.assertTrue(grp.hasView(viewName1))
        self.assertEqual( grp.getView(viewName1), view1 )

        self.assertTrue(grp.hasView(viewName2))
        self.assertEqual( grp.getView(viewName2), view2 )

        bufferId1 = view1.getBuffer().getIndex()

        grp.destroyViewAndBuffer(viewName1)


        self.assertFalse(grp.hasView(viewName1))
        self.assertEqual(ds.getNumBuffers(), 1)

        buffer1 = ds.getBuffer(bufferId1)
        self.assertTrue( buffer1 == ATK_NULLPTR )

        ds.delete()

#------------------------------------------------------------------------------
    def XXXtest_create_destroy_alloc_view_and_buffer(self):
        ds = sidre.DataStore()
        grp = ds.getRoot().createGroup("grp")

        viewName1 = "viewBuffer1"
        viewName2 = "viewBuffer2"

        # use create + alloc convenience methods
        # this one is the DataType & method
        view1 = grp.createViewAndBuffer(viewName1,DataType.c_int(10))
  # this one is the Schema & method
#--        Schema s
#--        s.set(DataType.c_double(10))
#--        view2 = grp.createViewAndBuffer(viewName2, s)

        self.assertTrue(grp.hasView(viewName1))
        self.assertEqual( grp.getView(viewName1), view1 )

        self.assertTrue(grp.hasView(viewName2))
        self.assertEqual( grp.getView(viewName2), view2 )


        v1_vals = view1.getValue()  # int
        v2_vals = view2.getValue()  # double

        for i in range(10):
            v1_vals[i] = i
            v2_vals[i] = i * 3.1415
        

        self.assertEqual(view1.getNumberOfElements(), 10)
        self.assertEqual(view2.getNumberOfElements(), 10)
        self.assertEqual(view1.getTotalBytes(), 10 * sizeof(int))
        self.assertEqual(view2.getTotalBytes(), 10 * sizeof(double))

        grp.destroyViewAndBuffer(viewName1)
        grp.destroyViewAndBuffer(viewName2)

        ds.delete()

#------------------------------------------------------------------------------
    def XXXtest_create_view_of_buffer_with_schema(self):
        ds = sidre.DataStore()
        root = ds.getRoot()
        # use create + alloc convenience methods
        # this one is the DataType & method
        base =  root.createViewAndBuffer("base", DataType.c_int(10))
        base_vals = base.getValue() # int
        for i in range(10):
            if i < 5:
                base_vals[i] = 10
            else:
                base_vals[i] = 20

        base_buff = base.getBuffer()
        # create two views into this buffer
        # view for the first 5 values
        root.createView("sub_a", base_buff, DataType.c_int(5))
        # view for the second 5 values
        #  (schema call path case)
#--        Schema s(DataType.c_int(5,5*sizeof(int)))
#--        root.createView("sub_b",base_buff,s)

        sub_a_vals = root.getView("sub_a").getValue()
        sub_b_vals = root.getView("sub_b").getValue()

        for i in range(5):
            self.assertEqual(sub_a_vals[i], 10)
            self.assertEqual(sub_b_vals[i], 20)

        ds.delete()


#------------------------------------------------------------------------------
    def XXXtest_save_restore_simple(self):
        ds = sidre.DataStore()
        flds = ds.getRoot().createGroup("fields")

        ga = flds.createGroup("a")

        ga.createViewAndBuffer("i0").allocate(DataType.c_int())

        ga.getView("i0").setValue(1)

        self.assertTrue(ds.getRoot().hasGroup("fields"))
        self.assertTrue(ds.getRoot().getGroup("fields").hasGroup("a"))
        self.assertTrue(ds.getRoot().getGroup("fields").getGroup("a").hasView("i0"))


        ds.getRoot().save("out_sidre_group_save_restore_simple","conduit")

        ds.print_json()

        ds2 = sidre.DataStore()

        ds2.getRoot().load("out_sidre_group_save_restore_simple","conduit")

        ds2.print_json()

        flds = ds2.getRoot().getGroup("fields")
        # check that all sub groups exist
        self.assertTrue(flds.hasGroup("a"))
        testvalue = flds.getGroup("a").getView("i0").getValue()
        self.assertEqual(testvalue,1)

        ds2.print_json()

        ds.delete()
        ds2.delete()

#------------------------------------------------------------------------------
    def XXXtest_save_restore_complex(self):
        ds = sidre.DataStore()
        flds = ds.getRoot().createGroup("fields")

        ga = flds.createGroup("a")
        gb = flds.createGroup("b")
        gc = flds.createGroup("c")

        ga.createViewAndBuffer("i0").allocate(DataType.c_int())
        gb.createViewAndBuffer("f0").allocate(DataType.c_float())
        gc.createViewAndBuffer("d0").allocate(DataType.c_double())

        ga.getView("i0").setValue(1)
        # Be careful on floats.  If you just hand it 100.0, the compiler will assume you want a double.
        # Either cast the value to float, or be explicit on the template argument.
        gb.getView("f0").setValue( 100.0 )
        # this would have worked equally well also.
        # gb.getView("f0").setValue<float>(100.0)
        gc.getView("d0").setValue(3000.00)

        # check that all sub groups exist
        self.assertTrue(flds.hasGroup("a"))
        self.assertTrue(flds.hasGroup("b"))
        self.assertTrue(flds.hasGroup("c"))

        ds.print_json()

        ds.getRoot().save("out_sidre_group_save_restore_complex","conduit")

        ds2 = sidre.DataStore()


        ds2.getRoot().load("out_sidre_group_save_restore_complex","conduit")

        flds = ds2.getRoot().getGroup("fields")
        # check that all sub groups exist
        self.assertTrue(flds.hasGroup("a"))
        self.assertTrue(flds.hasGroup("b"))
        self.assertTrue(flds.hasGroup("c"))

        self.assertEqual(flds.getGroup("a").getView("i0").getValue<int>(),1)
        EXPECT_NEAR(flds.getGroup("b").getView("f0").getValue<float>(),100.0,  1e-12)
        EXPECT_NEAR(flds.getGroup("c").getView("d0").getValue<double>(),3000.0, 1e-12)

        ds2.print_json()

        ds.delete()
        ds2.delete()




if __name__ == '__main__':
    unittest.main()
