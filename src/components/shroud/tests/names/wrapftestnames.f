! wrapftestnames.f
! This is generated code, do not edit
module testnames_mod
    use fstr_mod
    implicit none
    
    
    interface
        
        subroutine yyy_tes_function1() &
                bind(C, name="YYY_TES_function1")
            use iso_c_binding
            implicit none
        end subroutine yyy_tes_function1
        
        subroutine f_c_name_special() &
                bind(C, name="c_name_special")
            use iso_c_binding
            implicit none
        end subroutine f_c_name_special
        
        ! splicer begin additional_interfaces
        ! splicer end additional_interfaces
    end interface

contains
    
    subroutine function1()
        use iso_c_binding
        implicit none
        ! splicer begin function1
        call yyy_tes_function1()
        ! splicer end function1
    end subroutine function1
    
    subroutine f_name_special()
        use iso_c_binding
        implicit none
        ! splicer begin function2
        call f_c_name_special()
        ! splicer end function2
    end subroutine f_name_special
    
    ! splicer begin additional_functions
    ! splicer end additional_functions

end module testnames_mod
