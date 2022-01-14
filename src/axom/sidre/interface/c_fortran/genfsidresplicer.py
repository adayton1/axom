# Copyright (c) 2017-2022, Lawrence Livermore National Security, LLC and
# other Axom Project Developers. See the top-level LICENSE file for details.
#
# SPDX-License-Identifier: (BSD-3-Clause)

#
# Routines to generate splicers for wrappers.
# Used to generate several variations of a routine for Fortran.
# Similar to templates in C++.
#
from __future__ import print_function
import sys

# types to use for generic routines
types = (
    ( 'int',    'integer(C_INT)',  'SIDRE_INT_ID'),
    ( 'long',   'integer(C_LONG)', 'SIDRE_LONG_ID'),
    ( 'float',  'real(C_FLOAT)',   'SIDRE_FLOAT_ID'),
    ( 'double', 'real(C_DOUBLE)',  'SIDRE_DOUBLE_ID'),
)

# maximum number of dimensions of generic routines
maxdims = 4

def XXnum_metabuffers():
    return len(types) * (maxdims + 1) # include scalars
######################################################################

def group_get_scalar(d):
    """Create methods on Group to get a scalar.
    """
    return """
! Generated by genfsidresplicer.py
subroutine group_get_scalar_{typename}(grp, name, value)
    use iso_c_binding
    class(SidreGroup), intent(IN) :: grp
    character(*), intent(IN) :: name
    {f_type}, intent(OUT) :: value
    integer(C_INT) :: lname
    type(SIDRE_SHROUD_view_capsule) view
    type(C_PTR) viewptr

    lname = len_trim(name)
    viewptr = c_group_get_view_from_name_bufferify(grp%cxxmem, name, lname, view)
    value = c_view_get_data_{typename}(view)
end subroutine group_get_scalar_{typename}""".format(**d)

def group_set_scalar(d):
    """Create methods on Group to set a scalar.
    """
    return """
! Generated by genfsidresplicer.py
subroutine group_set_scalar_{typename}(grp, name, value)
    use iso_c_binding
    class(SidreGroup), intent(IN) :: grp
    character(*), intent(IN) :: name
    {f_type}, intent(IN) :: value
    integer(C_INT) :: lname
    type(SIDRE_SHROUD_view_capsule) view
    type(C_PTR) viewptr

    lname = len_trim(name)
    viewptr = c_group_get_view_from_name_bufferify(grp%cxxmem, name, lname, view)
    call c_view_set_scalar_{typename}(view, value)
end subroutine group_set_scalar_{typename}""".format(**d)

def group_create_array_view(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        extents_decl = 'extents(1)'
        extents_asgn = 'extents(1) = 1_SIDRE_IndexType'
    else:
        extents_decl = 'extents(%d)' % d['rank']
        extents_asgn = 'extents = shape(value, kind=SIDRE_IndexType)'

    return """
! Generated by genfsidresplicer.py
function group_create_array_view_{typename}{nd}(grp, name, value) result(rv)
    use iso_c_binding
    implicit none

    class(SidreGroup), intent(IN) :: grp
    character(*), intent(IN) :: name
    {f_type}, target, intent(IN) :: value{shape}
    integer(C_INT) :: lname
    type(SidreView) :: rv
    integer(SIDRE_IndexType) :: {extents_decl}
    integer(C_INT), parameter :: type = {sidre_type}
    type(C_PTR) addr, viewptr

    lname = len_trim(name)
#ifdef USE_C_LOC_WITH_ASSUMED_SHAPE
    addr = c_loc(value)
#else
    call SIDRE_C_LOC(value{lower_bound}, addr)
#endif
    {extents_asgn}
    viewptr = c_group_create_view_external_bufferify( &
        grp%cxxmem, name, lname, addr, rv%cxxmem)
    call c_view_apply_type_shape(rv%cxxmem, type, {rank}, extents)
end function group_create_array_view_{typename}{nd}""".format(
        extents_decl=extents_decl,
        extents_asgn=extents_asgn, **d)

def group_set_array_data_ptr(d):
    """
    call view%set_external_data_ptr
    hide c_loc call and add target attribute
    """
    # XXX - should this check the type/shape of value against the view?
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        extents_decl = 'extents(1)'
        extents_asgn = 'extents(1) = 1_SIDRE_IndexType'
    else:
        extents_decl = 'extents(%d)' % d['rank']
        extents_asgn = 'extents = shape(value, kind=SIDRE_IndexType)'

    return """
! Generated by genfsidresplicer.py
! This function does nothing if view name does not exist in group.
subroutine group_set_array_data_ptr_{typename}{nd}(grp, name, value)
    use iso_c_binding
    implicit none

    class(SidreGroup), intent(IN) :: grp
    character(len=*), intent(IN) :: name
    {f_type}, target, intent(IN) :: value{shape}
    integer(C_INT) :: lname
    type(SIDRE_SHROUD_view_capsule) view
!    integer(SIDRE_IndexType) :: {extents_decl}
!    integer(C_INT), parameter :: type = {sidre_type}
    type(C_PTR) addr, viewptr

    lname = len_trim(name)
!    {extents_asgn}
    viewptr = c_group_get_view_from_name_bufferify(grp%cxxmem, name, lname, view)
    if (c_associated(view%addr)) then
#ifdef USE_C_LOC_WITH_ASSUMED_SHAPE
        addr = c_loc(value)
#else
        call SIDRE_C_LOC(value{lower_bound}, addr)
#endif
        call c_view_set_external_data_ptr_only(view, addr)
!        call c_view_apply_type_shape(rv%cxxmem, type, {rank}, extents)
    endif
end subroutine group_set_array_data_ptr_{typename}{nd}""".format(
        extents_decl=extents_decl,
        extents_asgn=extents_asgn, **d)

def view_set_array_data_ptr(d):
    """
    call view%set_external_data_ptr
    hide c_loc call and add target attribute
    """
    # XXX - should this check the type/shape of value against the view?
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        extents_decl = 'extents(1)'
        extents_asgn = 'extents(1) = 1_SIDRE_IndexType'
    else:
        extents_decl = 'extents(%d)' % d['rank']
        extents_asgn = 'extents = shape(value, kind=SIDRE_IndexType)'

    return """
! Generated by genfsidresplicer.py
subroutine view_set_array_data_ptr_{typename}{nd}(view, value)
    use iso_c_binding
    implicit none

    class(SidreView), intent(IN) :: view
    {f_type}, target, intent(IN) :: value{shape}
!    integer(SIDRE_IndexType) :: {extents_decl}
!    integer(C_INT), parameter :: type = {sidre_type}
    type(C_PTR) addr

!    lname = len_trim(name)
!    {extents_asgn}
#ifdef USE_C_LOC_WITH_ASSUMED_SHAPE
    addr = c_loc(value)
#else
    call SIDRE_C_LOC(value{lower_bound}, addr)
#endif
    call c_view_set_external_data_ptr_only(view%cxxmem, addr)
!    call c_view_apply_type_shape(rv%cxxmem, type, {rank}, extents)
end subroutine view_set_array_data_ptr_{typename}{nd}""".format(
        extents_decl=extents_decl,
        extents_asgn=extents_asgn, **d)

def print_get_data(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        return """
! Generated by genfsidresplicer.py
subroutine view_get_data_{typename}{nd}{suffix}(view, value)
    use iso_c_binding
    implicit none
    class(SidreView), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    {f_type}, pointer :: tmp(:)
    type(C_PTR) cptr
    integer(SIDRE_IndexType) :: offset

    cptr = view%get_void_ptr()
    if (c_associated(cptr)) then
      offset = view%get_offset()
      if (offset > 0) then
        call c_f_pointer(cptr, tmp, [offset+1])   ! +1 to convert 0-based offset to 1-based index
        cptr = c_loc(tmp(offset+1))               ! Emulates pointer arithmetic
      endif
      
      call c_f_pointer(cptr, value)
    else
      nullify(value)
    endif
end subroutine view_get_data_{typename}{nd}{suffix}""".format(**d)

    else:
        return """
! Generated by genfsidresplicer.py
subroutine view_get_data_{typename}{nd}{suffix}(view, value)
    use iso_c_binding
    implicit none
    class(SidreView), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    {f_type}, pointer :: tmp(:)
    type(C_PTR) cptr
    integer rank
    integer(SIDRE_IndexType) extents({rank})
    integer(SIDRE_IndexType) :: offset

    cptr = view%get_void_ptr()
    if (c_associated(cptr)) then
      offset = view%get_offset()
      if (offset > 0) then
        call c_f_pointer(cptr, tmp, [offset+1])   ! +1 to convert 0-based offset to 1-based index
        cptr = c_loc(tmp(offset+1))               ! Emulates pointer arithmetic
      endif

      rank = view%get_shape({rank}, extents)
      call c_f_pointer(cptr, value, extents)
    else
      nullify(value)
    endif
end subroutine view_get_data_{typename}{nd}{suffix}""".format(**d)


class AddMethods(object):
    """Create lines necessary to add generic methods to a derived type.
    Loops over types and rank.

    procedure :: {stem}_{typename}{nd}{suffix} => {wrap_class}_{stem}_{typename}{nd}{suffix}
    generic :: {stem} => &
         gen1, &
         genn
    """
    def __init__(self, wrap_class):
        self.wrap_class = wrap_class
        self.lines = []
        self.methods = []

    @staticmethod
    def type_bound_procedure_part(d):
        return 'procedure :: {stem}_{typename}{nd}{suffix} => {wrap_class}_{stem}_{typename}{nd}{suffix}'.format(**d)

    @staticmethod
    def type_bound_procedure_generic(d):
        return '{stem}_{typename}{nd}{suffix}'.format(**d)

    def add_method(self, stem, fcn, scalar=False, **kwargs):
        self.methods.append((stem, fcn, scalar, kwargs))

    def gen_type_bound(self):
        lines = []
        for stem, fcn, scalar, kwargs in self.methods:
            generics = []
            extra = dict(
                wrap_class=self.wrap_class,
                stem=stem,
                )
            extra.update(kwargs)
            foreach_type(lines, AddMethods.type_bound_procedure_part, scalar=scalar, **extra)
            foreach_type(generics, AddMethods.type_bound_procedure_generic, scalar=scalar, **extra)
            
            lines.append('generic :: {stem} => &'.format(stem=stem))
            for gen in generics[:-1]:
                lines.append('    ' + gen + ',  &')
            lines.append('    ' + generics[-1])
        return lines

    def gen_body(self):
        lines = []
        for stem, fcn, scalar, kwargs in self.methods:
            foreach_type(lines, fcn, scalar=scalar, **kwargs)
        return lines


def foreach_type(lines, fcn, scalar=False, **kwargs):
    """ Call fcn once for each type and rank, appending to lines.
    kwargs - additional values for format dictionary.
    """
    shape = []
    lbound = []
    for nd in range(maxdims + 1):
        shape.append(':')
        lbound.append('lbound(value,%d)' % (nd+1))
    d = dict(
        suffix=''     # suffix of function name
    )
    d.update(kwargs)
    indx = 0
    for typetuple in types:
        d['typename'], d['f_type'], d['sidre_type'] = typetuple

        # scalar values
        # XXX - generic does not distinguish between pointer and non-pointer
#        d['rank'] = -1
#        d['nd'] = 'scalar'
#        d['shape'] = ''
#        lines.append(fcn(d))

        # scalar pointers
        d['index'] = indx
        indx += 1
        d['rank'] = 0
        d['shape'] = ''
        d['lower_bound'] = ''

        if scalar:
            d['nd'] = ''
            lines.append(fcn(d))
        else:
            d['nd'] = '_scalar'
            lines.append(fcn(d))
            for nd in range(1,maxdims+1):
                d['index'] = indx
                indx += 1
                d['rank'] = nd
                d['nd'] = '_%dd' % nd
                d['shape'] = '(' + ','.join(shape[:nd]) + ')'
                d['lower_bound'] = '(' + ','.join(lbound[:nd]) + ')'
                lines.append(fcn(d))

#----------------------------------------------------------------------

def group_string():
    """Text for functions with get and set strings for a group.

    get_string  =>   grp->getView(name)->getString()
    set_string  =>   grp->getView(name)->setString()
    """
    return """
subroutine group_get_string(grp, name, value)
    use iso_c_binding
    class(SidreGroup), intent(IN) :: grp
    character(*), intent(IN) :: name
    character(*), intent(OUT) :: value
    integer(C_INT) :: lname
    type(SIDRE_SHROUD_view_capsule) view
    type(C_PTR) viewptr

    lname = len_trim(name)
    viewptr = c_group_get_view_from_name_bufferify(grp%cxxmem, name, lname, view)
    call c_view_get_string_bufferify(view, value, len(value, kind=C_INT))
end subroutine group_get_string

subroutine group_set_string(grp, name, value)
    use iso_c_binding
    class(SidreGroup), intent(IN) :: grp
    character(*), intent(IN) :: name
    character(*), intent(IN) :: value
    integer(C_INT) :: lname
    type(SIDRE_SHROUD_view_capsule) view
    type(C_PTR) viewptr

    lname = len_trim(name)
    viewptr = c_group_get_view_from_name_bufferify(grp%cxxmem, name, lname, view)
    call c_view_set_string_bufferify(view, value, len_trim(value, kind=C_INT))
end subroutine group_set_string
"""


#----------------------------------------------------------------------

def gen_fortran():
    """Generate splicers used by Shroud.
    """
    print('! Generated by genfsidresplicer.py')

    # Group
    t = AddMethods('group')
    t.add_method('get_scalar', group_get_scalar, True)
    t.add_method('set_scalar', group_set_scalar, True)
    t.add_method('create_array_view', group_create_array_view)
    t.add_method('set_array_data_ptr', group_set_array_data_ptr)

    print('! splicer begin class.Group.type_bound_procedure_part')
    for line in t.gen_type_bound():
        print(line)
    print('procedure :: get_string => group_get_string')
    print('procedure :: set_string => group_set_string')
    print('! splicer end class.Group.type_bound_procedure_part')

    print()
    print('------------------------------------------------------------')
    print()

    print('! splicer begin class.Group.additional_functions')
    for line in t.gen_body():
        print(line)
    print(group_string())
    print('! splicer end class.Group.additional_functions')


    # View
    t = AddMethods('view')
    t.add_method('get_data', print_get_data, suffix='_ptr')
    t.add_method('set_array_data_ptr', view_set_array_data_ptr)

    print('! splicer begin class.View.type_bound_procedure_part')
    for line in t.gen_type_bound():
        print(line)
    print('! splicer end class.View.type_bound_procedure_part')

    print()
    print('------------------------------------------------------------')
    print()

    print('! splicer begin class.View.additional_functions')
    for line in t.gen_body():
        print(line)
    print('! splicer end class.View.additional_functions')

######################################################################

if __name__ == '__main__':
    try:
        cmd = sys.argv[1]
    except IndexError:
        raise RuntimeError("Missing command line argument")

    if cmd == 'fortran':
        # fortran splicers
        gen_fortran()
    elif cmd == 'test':
        AllocateAllocatable(print)
    else:
        raise RuntimeError("Unknown command")
