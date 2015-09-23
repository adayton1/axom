#
# Routines to generate splicers for wrappers.
# Used to generate several variations of a routine for Fortran.
# Similar to templates in C++.
#
from __future__ import print_function
import sys

types = (
    ( 'int',    'integer(C_INT)',  'ATK_C_INT_T'),
    ( 'long',   'integer(C_LONG)', 'ATK_C_LONG_T'),
    ( 'float',  'real(C_FLOAT)',   'ATK_C_FLOAT_T'),
    ( 'double', 'real(C_DOUBLE)',  'ATK_C_DOUBLE_T'),
)

######################################################################

def print_register_allocatable(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == -1:
        raise NotImplementedError
        return """
! Generated by genfsidresplicer.py
function datagroup_register_allocatable_{typename}_{nd}(group, name, value) result(rv)
    use iso_c_binding
    implicit none
    class(datagroup), intent(IN) :: group
    character(*), intent(IN) :: name
    {f_type}, allocatable, intent(IN) :: value{shape}
    type(dataview) :: rv
    rv%voidptr = C_NULL_PTR
end subroutine datagroup_register_allocatable_{typename}_{nd}""".format(**d)

    elif d['rank'] == 0:
        return """
! Generated by genfsidresplicer.py
function datagroup_register_allocatable_{typename}_{nd}(group, name, value) result(rv)
    use iso_c_binding
    implicit none

    interface
       function ATK_register_allocatable_{typename}_{nd}(group, name, lname, array, atk_type, rank) result(rv)
       use iso_c_binding
       type(C_PTR), value, intent(IN)    :: group
       character(*), intent(IN)          :: name
       integer(C_INT), value, intent(IN) :: lname
       {f_type}, allocatable, intent(IN) :: array{shape}
       integer(C_INT), value, intent(IN) :: atk_type
       integer(C_INT), value, intent(IN) :: rank
       type(C_PTR) rv
       end function ATK_register_allocatable_{typename}_{nd}
    end interface

    class(datagroup), intent(IN) :: group
    character(*), intent(IN) :: name
    {f_type}, allocatable, intent(IN) :: value{shape}
    integer(C_INT) :: atk_type = {atk_type}
    integer(C_INT) :: ndim = {rank}
    integer(C_INT) :: lname
    type(dataview) :: rv

    lname = len_trim(name)
    rv%voidptr = ATK_register_allocatable_{typename}_{nd}(group%voidptr, name, lname, value, atk_type, ndim)
end function datagroup_register_allocatable_{typename}_{nd}""".format(**d)

    elif d['rank'] == 1:
        return """
! Generated by genfsidresplicer.py
function datagroup_register_allocatable_{typename}_{nd}(group, name, value) result(rv)
    use iso_c_binding
    implicit none

    interface
       function ATK_register_allocatable_{typename}_{nd}(group, name, lname, array, atk_type, rank) result(rv)
       use iso_c_binding
       type(C_PTR), value, intent(IN)    :: group
       character(*), intent(IN)          :: name
       integer(C_INT), value, intent(IN) :: lname
       {f_type}, allocatable, intent(IN) :: array{shape}
       integer(C_INT), value, intent(IN) :: atk_type
       integer(C_INT), value, intent(IN) :: rank
       type(C_PTR) rv
       end function ATK_register_allocatable_{typename}_{nd}
    end interface

    class(datagroup), intent(IN) :: group
    character(*), intent(IN) :: name
    integer(C_INT) :: atk_type = {atk_type}
    integer(C_INT) :: ndim = {rank}
    {f_type}, allocatable, intent(IN) :: value{shape}
    integer(C_INT) :: lname
    type(dataview) :: rv

    lname = len_trim(name)
    rv%voidptr = ATK_register_allocatable_{typename}_{nd}(group%voidptr, name, lname, value, atk_type, ndim)
end function datagroup_register_allocatable_{typename}_{nd}""".format(**d)
    else:
        raise RuntimeError("rank too large in print_register_allocatable")


def print_get_value(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == -1:
        return """
! Generated by genfsidresplicer.py
subroutine dataview_get_value_{typename}_{nd}(view, value)
    use iso_c_binding
    implicit none
    class(dataview), intent(IN) :: view
    {f_type}, intent(OUT) :: value
    {f_type}, pointer :: value_ptr
    type(C_PTR) cptr

    cptr = view%get_data_pointer()
    call c_f_pointer(cptr, value_ptr)
    value = value_ptr
end subroutine dataview_get_value_{typename}_{nd}""".format(**d)

    elif d['rank'] == 0:
        return """
! Generated by genfsidresplicer.py
subroutine dataview_get_value_{typename}_{nd}(view, value)
    use iso_c_binding
    implicit none
    class(dataview), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    type(C_PTR) cptr

    cptr = view%get_data_pointer()
    call c_f_pointer(cptr, value)
end subroutine dataview_get_value_{typename}_{nd}""".format(**d)

    elif d['rank'] == 1:
        return """
! Generated by genfsidresplicer.py
subroutine dataview_get_value_{typename}_{nd}(view, value)
    use iso_c_binding
    implicit none
    class(dataview), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    type(C_PTR) cptr
    integer(C_SIZE_T) nelems

    cptr = view%get_data_pointer()
    nelems = view%get_number_of_elements()
    call c_f_pointer(cptr, value, [ nelems ])
end subroutine dataview_get_value_{typename}_{nd}""".format(**d)
    else:
        raise RuntimeError("rank too large in print_get_value")


def type_bound_procedure_part(d):
    return 'procedure :: {stem}_{typename}_{nd} => {wrap_class}_{stem}_{typename}_{nd}'.format(**d)

def type_bound_procedure_generic(d):
    return '{stem}_{typename}_{nd}'.format(**d)

def type_bound_procedure_generic_post(lines, generics, stem):
    lines.append('generic :: %s => &' % stem)
    for gen in generics[:-1]:
        lines.append('    ' + gen + ',  &')
    lines.append('    ' + generics[-1])


def foreach_value(lines, fcn, **kwargs):
    """ Call fcn once for each type, appending to lines.
    kwargs - additional values for format dictionary.
    """
    shape = []
    for nd in range(2):
        shape.append(':')
    d = {}
    d.update(kwargs)
    for typetuple in types:
        d['typename'], d['f_type'], d['atk_type'] = typetuple

        # scalar values
        # XXX - generic does not distinguish between pointer and non-pointer
#        d['rank'] = -1
#        d['nd'] = 'scalar'
#        d['shape'] = ''
#        lines.append(fcn(d))

        # scalar pointers
        d['rank'] = 0
        d['nd'] = 'scalar_ptr'
        d['shape'] = ''
        lines.append(fcn(d))

        for nd in range(1,2):   # XXX - only doing 0-d and 1-d for now
            d['rank'] = nd
            d['nd'] = '%dd_ptr' % nd
            d['shape'] = '(' + ','.join(shape[:nd]) + ')'
            lines.append(fcn(d))

def gen_fortran():
    print('! Generated by genfsidresplicer.py')

    # DataGroup
    extra = dict(
        wrap_class='datagroup',
        stem='register_allocatable',
        )
    lines = []
    foreach_value(lines, type_bound_procedure_part, **extra)
    generics = []
    foreach_value(generics, type_bound_procedure_generic, **extra)
    type_bound_procedure_generic_post(lines, generics, 'register_allocatable')

    print('! splicer begin class.DataGroup.type_bound_procedure_part')
    for line in lines:
        print(line)
    print('! splicer end class.DataGroup.type_bound_procedure_part')

    print()
    print('------------------------------------------------------------')
    print()

    print('! splicer begin class.DataGroup.additional_functions')
    lines = []
    foreach_value(lines, print_register_allocatable)
    for line in lines:
        print(line)
    print('! splicer end class.DataGroup.additional_functions')


    # DataView
    extra = dict(
        wrap_class='dataview',
        stem='get_value',
        )
    lines = []
    foreach_value(lines, type_bound_procedure_part, **extra)
    generics = []
    foreach_value(generics, type_bound_procedure_generic, **extra)
    type_bound_procedure_generic_post(lines, generics, 'get_value')

    print('! splicer begin class.DataView.type_bound_procedure_part')
    for line in lines:
        print(line)
    print('! splicer end class.DataView.type_bound_procedure_part')

    print()
    print('------------------------------------------------------------')
    print()

    print('! splicer begin class.DataView.additional_functions')
    lines = []
    foreach_value(lines, print_get_value)
    for line in lines:
        print(line)
    print('! splicer end class.DataView.additional_functions')


######################################################################

def print_atk_register_allocatable(d):
    """Write C++ routine to accept Fortran allocatable.
    """
# XXX - need cmake macro to mangle name portably
    return """
// Fortran callable routine.
// Needed for each type-kind-rank to get address of allocatable array.
void *atk_register_allocatable_{typename}_{nd}_(
    DataGroup *group,
    char *name, int lname,
    void *array, int atk_type, int rank)
{{
    return register_allocatable(group, std::string(name, lname), array, atk_type, rank); 
}}
""".format(**d)

def gen_allocatable():
    print("""//
// SidreAllocatable.cpp - Routines used by Fortran interface
// Generated by genfsidresplicer.py
//   DO NOT EDIT
//
#include <cstddef>
#include "common/CommonTypes.hpp"
#include "SidreWrapperHelpers.hpp"

extern "C" {
namespace asctoolkit {
namespace sidre {
""")

    lines = []
    foreach_value(lines, print_atk_register_allocatable)
    for line in lines:
        print(line)

    print("""
}  // namespace asctoolkit
}  // namespace sidre
}  // extern "C"
""")


######################################################################

if __name__ == '__main__':
    try:
        cmd = sys.argv[1]
    except IndexError:
        raise RuntimeError("Missing command line argument")

    if cmd == 'fortran':
        gen_fortran()
    elif cmd == 'allocatable':
        gen_allocatable()
    else:
        raise RuntimeError("Unknown command")
