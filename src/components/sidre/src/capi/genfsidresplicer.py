#
# Routines to generate splicers for wrappers.
# Used to generate several variations of a routine for Fortran.
# Similar to templates in C++.
#
from __future__ import print_function
import sys

types = (
    ( 'int',    'integer(C_INT)',  'SIDRE_INT_ID'),
    ( 'long',   'integer(C_LONG)', 'SIDRE_LONG_ID'),
    ( 'float',  'real(C_FLOAT)',   'SIDRE_FLOAT_ID'),
    ( 'double', 'real(C_DOUBLE)',  'SIDRE_DOUBLE_ID'),
)

# XXX - only doing 0-d and 1-d for now
maxdims = 1

def XXnum_metabuffers():
    return len(types) * (maxdims + 1) # include scalars
######################################################################

def create_array_view(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        extents_decl = 'extents(1)'
        extents_asgn = 'extents(1) = 1_SIDRE_LENGTH'
    else:
        extents_decl = 'extents(%d)' % d['rank']
        extents_asgn = 'extents = shape(value, kind=SIDRE_LENGTH)'

    return """
! Generated by genfsidresplicer.py
function datagroup_create_array_view_{typename}_{nd}(group, name, value) result(rv)
    use iso_c_binding
    implicit none

    interface
       function SIDRE_create_array_view(group, name, lname, addr, type, rank, extents) &
           result(rv) bind(C,name="SIDRE_create_array_view")
       use iso_c_binding
       import SIDRE_LENGTH
       type(C_PTR), value, intent(IN)     :: group
       character(kind=C_CHAR), intent(IN) :: name(*)
       integer(C_INT), value, intent(IN)  :: lname
       type(C_PTR), value,     intent(IN) :: addr
       integer(C_INT), value, intent(IN)  :: type
       integer(C_INT), value, intent(IN)  :: rank
       integer(SIDRE_LENGTH), intent(IN)  :: extents(*)
       type(C_PTR) rv
       end function SIDRE_create_array_view
    end interface
    external :: SIDRE_C_LOC

    class(datagroup), intent(IN) :: group
    character(*), intent(IN) :: name
    {f_type}, target, intent(IN) :: value{shape}
    integer(C_INT) :: lname
    type(dataview) :: rv
    integer(SIDRE_LENGTH) :: {extents_decl}
    integer(C_INT), parameter :: type = {sidre_type}
    type(C_PTR) addr

    lname = len_trim(name)
    {extents_asgn}
    call SIDRE_C_LOC(value, addr)
    rv%voidptr = SIDRE_create_array_view(group%voidptr, name, lname, addr, type, {rank}, extents)
end function datagroup_create_array_view_{typename}_{nd}""".format(
        extents_decl=extents_decl,
        extents_asgn=extents_asgn, **d)

def print_get_value(d):
    # typename - part of function name
    # nd       - number of dimensions
    # f_type   - fortran type
    # shape     - :,:, to match nd
    if d['rank'] == 0:
        return """
! Generated by genfsidresplicer.py
subroutine dataview_get_value_{typename}_{nd}{suffix}(view, value)
    use iso_c_binding
    implicit none
    class(dataview), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    type(C_PTR) cptr

    cptr = view%get_data_pointer()
    call c_f_pointer(cptr, value)
end subroutine dataview_get_value_{typename}_{nd}{suffix}""".format(**d)

    elif d['rank'] == 1:
        return """
! Generated by genfsidresplicer.py
subroutine dataview_get_value_{typename}_{nd}{suffix}(view, value)
    use iso_c_binding
    implicit none
    class(dataview), intent(IN) :: view
    {f_type}, pointer, intent(OUT) :: value{shape}
    type(C_PTR) cptr
    integer(C_SIZE_T) nelems

    cptr = view%get_data_pointer()
    nelems = view%get_num_elements()
    call c_f_pointer(cptr, value, [ nelems ])
end subroutine dataview_get_value_{typename}_{nd}{suffix}""".format(**d)
    else:
        raise RuntimeError("rank too large in print_get_value")


def type_bound_procedure_part(d):
    return 'procedure :: {stem}_{typename}_{nd}{suffix} => {wrap_class}_{stem}_{typename}_{nd}{suffix}'.format(**d)

def type_bound_procedure_generic(d):
    return '{stem}_{typename}_{nd}{suffix}'.format(**d)

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
        d['nd'] = 'scalar'
        d['shape'] = ''
        d['lower_bound'] = ''
        lines.append(fcn(d))

        for nd in range(1,maxdims+1):
            d['index'] = indx
            indx += 1
            d['rank'] = nd
            d['nd'] = '%dd' % nd
            d['shape'] = '(' + ','.join(shape[:nd]) + ')'
            d['lower_bound'] = '(' + ','.join(lbound[:nd]) + ')'
            lines.append(fcn(d))

def print_lines(printer, fcn, **kwargs):
    """Print output using printer function.
    [Used with cog]
    """
    lines = []
    foreach_value(lines, fcn, **kwargs)
    for line in lines:
        printer(line)

#----------------------------------------------------------------------

def print_switch(printer, calls):
    """Print a switch statement on type and rank.
    Caller must set fileds in d:
      prefix = call or assignment
                  'call foo'
                  'nitems = foo'
      args   = arguments to function, must include parens.
                  '(args)'
                  ''           -- subroutine with no arguments
    """
    d = {}
    printer('  switch(type)')
    printer('  {')
    for typetuple in types:
        d['typename'], f_type, sidre_type = typetuple
        printer('  case %s:' % sidre_type)
        printer('    switch(rank)')
        printer('    {')
        for nd in range(0,maxdims+1):
            if nd == 0:
                d['nd'] = 'scalar'
            else:
                d['nd'] = '%dd' % nd
            printer('    case %d:' % nd)
            for ca in calls:
                d['prefix'] = ca[0]
                d['macro'] = '{prefix}_{typename}_{nd}'.format(**d).upper()
                printer('      ' + ca[1].format(**d) + ';')
            printer('      break;')
        printer('    default:')
        printer('      break;')
        printer('    }')
        printer('    break;')
    printer('  default:')
    printer('    break;')
    printer('  }')

#----------------------------------------------------------------------

def gen_fortran():
    """Generate splicers used by Shroud.
    """
    print('! Generated by genfsidresplicer.py')


    # DataGroup
    lines = []

    generics = []
    extra = dict(
        wrap_class='datagroup',
        stem='create_array_view',
        )
    foreach_value(lines, type_bound_procedure_part, **extra)
    foreach_value(generics, type_bound_procedure_generic, **extra)
    type_bound_procedure_generic_post(lines, generics, 'create_array_view')

    print('! splicer begin class.DataGroup.type_bound_procedure_part')
    for line in lines:
        print(line)
    print('! splicer end class.DataGroup.type_bound_procedure_part')

    print()
    print('------------------------------------------------------------')
    print()

    print('! splicer begin class.DataGroup.additional_functions')
    lines = []
#    foreach_value(lines, create_allocatable_view)
    foreach_value(lines, create_array_view)
    for line in lines:
        print(line)
    print('! splicer end class.DataGroup.additional_functions')


    # DataView
    extra = dict(
        wrap_class='dataview',
        stem='get_value',
        suffix='_ptr',
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
    foreach_value(lines, print_get_value, suffix='_ptr')
    for line in lines:
        print(line)
    print('! splicer end class.DataView.additional_functions')

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
