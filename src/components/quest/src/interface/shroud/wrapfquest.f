! wrapfquest.f
! This is generated code, do not edit
!
! Copyright (c) 2015, Lawrence Livermore National Security, LLC.
! Produced at the Lawrence Livermore National Laboratory.
!
! All rights reserved.
!
! This source code cannot be distributed without permission and
! further review from Lawrence Livermore National Laboratory.
!
!>
!! \file wrapfquest.f
!! \brief Shroud generated wrapper for QUEST library
!<
module quest_mod
    use fstr_mod
    ! splicer begin module_use
    ! splicer end module_use
    implicit none
    
    
    interface
        
        subroutine c_initialize(mpicomm, fileName, ndims, maxElements, maxLevels) &
                bind(C, name="QUEST_initialize")
            use iso_c_binding
            implicit none
            integer(C_INT), value, intent(IN) :: mpicomm
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: ndims
            integer(C_INT), value, intent(IN) :: maxElements
            integer(C_INT), value, intent(IN) :: maxLevels
        end subroutine c_initialize
        
        subroutine c_initialize_bufferify(mpicomm, fileName, LfileName, ndims, maxElements, maxLevels) &
                bind(C, name="QUEST_initialize_bufferify")
            use iso_c_binding
            implicit none
            integer(C_INT), value, intent(IN) :: mpicomm
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: LfileName
            integer(C_INT), value, intent(IN) :: ndims
            integer(C_INT), value, intent(IN) :: maxElements
            integer(C_INT), value, intent(IN) :: maxLevels
        end subroutine c_initialize_bufferify
        
        subroutine quest_finalize() &
                bind(C, name="QUEST_finalize")
            use iso_c_binding
            implicit none
        end subroutine quest_finalize
        
        function quest_distance(x, y, z) &
                result(rv) &
                bind(C, name="QUEST_distance")
            use iso_c_binding
            implicit none
            real(C_DOUBLE), value, intent(IN) :: x
            real(C_DOUBLE), value, intent(IN) :: y
            real(C_DOUBLE), value, intent(IN) :: z
            real(C_DOUBLE) :: rv
        end function quest_distance
        
        function quest_inside(x, y, z) &
                result(rv) &
                bind(C, name="QUEST_inside")
            use iso_c_binding
            implicit none
            real(C_DOUBLE), value, intent(IN) :: x
            real(C_DOUBLE), value, intent(IN) :: y
            real(C_DOUBLE), value, intent(IN) :: z
            integer(C_INT) :: rv
        end function quest_inside
        
        ! splicer begin additional_interfaces
        ! splicer end additional_interfaces
    end interface

contains
    
    subroutine quest_initialize(mpicomm, fileName, ndims, maxElements, maxLevels)
        use iso_c_binding
        implicit none
        integer(C_INT), value, intent(IN) :: mpicomm
        character(*), intent(IN) :: fileName
        integer(C_INT), value, intent(IN) :: ndims
        integer(C_INT), value, intent(IN) :: maxElements
        integer(C_INT), value, intent(IN) :: maxLevels
        ! splicer begin initialize
        call c_initialize_bufferify(  &
            mpicomm,  &
            fileName,  &
            len_trim(fileName, kind=C_INT),  &
            ndims,  &
            maxElements,  &
            maxLevels)
        ! splicer end initialize
    end subroutine quest_initialize
    
    ! splicer begin additional_functions
    ! splicer end additional_functions

end module quest_mod
