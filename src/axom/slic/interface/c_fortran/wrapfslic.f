! wrapfslic.f
! This file is generated by Shroud 0.12.2. Do not edit.
!
! Copyright (c) 2017-2022, Lawrence Livermore National Security, LLC and
! other Axom Project Developers. See the top-level LICENSE file for details.
!
! SPDX-License-Identifier: (BSD-3-Clause)
!>
!! \file wrapfslic.f
!! \brief Shroud generated wrapper for slic namespace
!<
! splicer begin file_top
! splicer end file_top
module axom_slic
    use iso_c_binding, only : C_INT, C_NULL_PTR, C_PTR
    ! splicer begin module_use
    ! splicer end module_use
    implicit none

    ! splicer begin module_top
    ! splicer end module_top

    !  enum axom::slic::message::Level
    integer(C_INT), parameter :: message_error = 0
    integer(C_INT), parameter :: message_warning = 1
    integer(C_INT), parameter :: message_info = 2
    integer(C_INT), parameter :: message_debug = 3
    integer(C_INT), parameter :: message_num_levels = 4

    type, bind(C) :: SLIC_SHROUD_genericoutputstream_capsule
        type(C_PTR) :: addr = C_NULL_PTR  ! address of C++ memory
        integer(C_INT) :: idtor = 0       ! index of destructor
    end type SLIC_SHROUD_genericoutputstream_capsule

    type SlicGenericOutputStream
        type(SLIC_SHROUD_genericoutputstream_capsule) :: cxxmem
        ! splicer begin class.GenericOutputStream.component_part
        ! splicer end class.GenericOutputStream.component_part
    contains
        procedure :: delete => slic_genericoutputstream_delete
        procedure :: get_instance => slic_genericoutputstream_get_instance
        procedure :: set_instance => slic_genericoutputstream_set_instance
        procedure :: associated => slic_genericoutputstream_associated
        ! splicer begin class.GenericOutputStream.type_bound_procedure_part
        ! splicer end class.GenericOutputStream.type_bound_procedure_part
    end type SlicGenericOutputStream

    interface operator (.eq.)
        module procedure genericoutputstream_eq
    end interface

    interface operator (.ne.)
        module procedure genericoutputstream_ne
    end interface

    interface

        function c_genericoutputstream_ctor_default(stream, SHT_crv) &
                result(SHT_rv) &
                bind(C, name="SLIC_GenericOutputStream_ctor_default")
            use iso_c_binding, only : C_CHAR, C_PTR
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            character(kind=C_CHAR), intent(IN) :: stream(*)
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(OUT) :: SHT_crv
            type(C_PTR) SHT_rv
        end function c_genericoutputstream_ctor_default

        function c_genericoutputstream_ctor_default_bufferify(stream, &
                Lstream, SHT_crv) &
                result(SHT_rv) &
                bind(C, name="SLIC_GenericOutputStream_ctor_default_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT, C_PTR
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            character(kind=C_CHAR), intent(IN) :: stream(*)
            integer(C_INT), value, intent(IN) :: Lstream
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(OUT) :: SHT_crv
            type(C_PTR) SHT_rv
        end function c_genericoutputstream_ctor_default_bufferify

        function c_genericoutputstream_ctor_format(stream, format, &
                SHT_crv) &
                result(SHT_rv) &
                bind(C, name="SLIC_GenericOutputStream_ctor_format")
            use iso_c_binding, only : C_CHAR, C_PTR
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            character(kind=C_CHAR), intent(IN) :: stream(*)
            character(kind=C_CHAR), intent(IN) :: format(*)
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(OUT) :: SHT_crv
            type(C_PTR) SHT_rv
        end function c_genericoutputstream_ctor_format

        function c_genericoutputstream_ctor_format_bufferify(stream, &
                Lstream, format, Lformat, SHT_crv) &
                result(SHT_rv) &
                bind(C, name="SLIC_GenericOutputStream_ctor_format_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT, C_PTR
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            character(kind=C_CHAR), intent(IN) :: stream(*)
            integer(C_INT), value, intent(IN) :: Lstream
            character(kind=C_CHAR), intent(IN) :: format(*)
            integer(C_INT), value, intent(IN) :: Lformat
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(OUT) :: SHT_crv
            type(C_PTR) SHT_rv
        end function c_genericoutputstream_ctor_format_bufferify

        subroutine c_genericoutputstream_delete(self) &
                bind(C, name="SLIC_GenericOutputStream_delete")
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(IN) :: self
        end subroutine c_genericoutputstream_delete

        ! splicer begin class.GenericOutputStream.additional_interfaces
        ! splicer end class.GenericOutputStream.additional_interfaces

        subroutine slic_initialize() &
                bind(C, name="SLIC_initialize")
            implicit none
        end subroutine slic_initialize

        function c_is_initialized() &
                result(SHT_rv) &
                bind(C, name="SLIC_is_initialized")
            use iso_c_binding, only : C_BOOL
            implicit none
            logical(C_BOOL) :: SHT_rv
        end function c_is_initialized

        subroutine c_create_logger(name, imask) &
                bind(C, name="SLIC_create_logger")
            use iso_c_binding, only : C_CHAR
            implicit none
            character(kind=C_CHAR), intent(IN) :: name(*)
            character(kind=C_CHAR), value, intent(IN) :: imask
        end subroutine c_create_logger

        subroutine c_create_logger_bufferify(name, Lname, imask) &
                bind(C, name="SLIC_create_logger_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            character(kind=C_CHAR), intent(IN) :: name(*)
            integer(C_INT), value, intent(IN) :: Lname
            character(kind=C_CHAR), value, intent(IN) :: imask
        end subroutine c_create_logger_bufferify

        function c_activate_logger(name) &
                result(SHT_rv) &
                bind(C, name="SLIC_activate_logger")
            use iso_c_binding, only : C_BOOL, C_CHAR
            implicit none
            character(kind=C_CHAR), intent(IN) :: name(*)
            logical(C_BOOL) :: SHT_rv
        end function c_activate_logger

        function c_activate_logger_bufferify(name, Lname) &
                result(SHT_rv) &
                bind(C, name="SLIC_activate_logger_bufferify")
            use iso_c_binding, only : C_BOOL, C_CHAR, C_INT
            implicit none
            character(kind=C_CHAR), intent(IN) :: name(*)
            integer(C_INT), value, intent(IN) :: Lname
            logical(C_BOOL) :: SHT_rv
        end function c_activate_logger_bufferify

        subroutine c_get_active_logger_name_bufferify(name, Nname) &
                bind(C, name="SLIC_get_active_logger_name_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            character(kind=C_CHAR), intent(OUT) :: name(*)
            integer(C_INT), value, intent(IN) :: Nname
        end subroutine c_get_active_logger_name_bufferify

        function slic_get_logging_msg_level() &
                result(SHT_rv) &
                bind(C, name="SLIC_get_logging_msg_level")
            use iso_c_binding, only : C_INT
            implicit none
            integer(C_INT) :: SHT_rv
        end function slic_get_logging_msg_level

        subroutine slic_set_logging_msg_level(level) &
                bind(C, name="SLIC_set_logging_msg_level")
            use iso_c_binding, only : C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
        end subroutine slic_set_logging_msg_level

        subroutine c_add_stream_to_all_msg_levels(ls) &
                bind(C, name="SLIC_add_stream_to_all_msg_levels")
            import :: SLIC_SHROUD_genericoutputstream_capsule
            implicit none
            type(SLIC_SHROUD_genericoutputstream_capsule), intent(INOUT) :: ls
        end subroutine c_add_stream_to_all_msg_levels

        subroutine c_set_abort_on_error(status) &
                bind(C, name="SLIC_set_abort_on_error")
            use iso_c_binding, only : C_BOOL
            implicit none
            logical(C_BOOL), value, intent(IN) :: status
        end subroutine c_set_abort_on_error

        subroutine slic_enable_abort_on_error() &
                bind(C, name="SLIC_enable_abort_on_error")
            implicit none
        end subroutine slic_enable_abort_on_error

        subroutine slic_disable_abort_on_error() &
                bind(C, name="SLIC_disable_abort_on_error")
            implicit none
        end subroutine slic_disable_abort_on_error

        function c_is_abort_on_errors_enabled() &
                result(SHT_rv) &
                bind(C, name="SLIC_is_abort_on_errors_enabled")
            use iso_c_binding, only : C_BOOL
            implicit none
            logical(C_BOOL) :: SHT_rv
        end function c_is_abort_on_errors_enabled

        subroutine c_set_abort_on_warning(status) &
                bind(C, name="SLIC_set_abort_on_warning")
            use iso_c_binding, only : C_BOOL
            implicit none
            logical(C_BOOL), value, intent(IN) :: status
        end subroutine c_set_abort_on_warning

        subroutine slic_enable_abort_on_warning() &
                bind(C, name="SLIC_enable_abort_on_warning")
            implicit none
        end subroutine slic_enable_abort_on_warning

        subroutine slic_disable_abort_on_warning() &
                bind(C, name="SLIC_disable_abort_on_warning")
            implicit none
        end subroutine slic_disable_abort_on_warning

        function c_is_abort_on_warnings_enabled() &
                result(SHT_rv) &
                bind(C, name="SLIC_is_abort_on_warnings_enabled")
            use iso_c_binding, only : C_BOOL
            implicit none
            logical(C_BOOL) :: SHT_rv
        end function c_is_abort_on_warnings_enabled

        subroutine c_log_message_file_line(level, message, fileName, &
                line) &
                bind(C, name="SLIC_log_message_file_line")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: line
        end subroutine c_log_message_file_line

        subroutine c_log_message_file_line_bufferify(level, message, &
                Lmessage, fileName, LfileName, line) &
                bind(C, name="SLIC_log_message_file_line_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            integer(C_INT), value, intent(IN) :: Lmessage
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: LfileName
            integer(C_INT), value, intent(IN) :: line
        end subroutine c_log_message_file_line_bufferify

        subroutine c_log_message_file_line_filter(level, message, &
                fileName, line, filter_duplicates) &
                bind(C, name="SLIC_log_message_file_line_filter")
            use iso_c_binding, only : C_BOOL, C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: line
            logical(C_BOOL), value, intent(IN) :: filter_duplicates
        end subroutine c_log_message_file_line_filter

        subroutine c_log_message_file_line_filter_bufferify(level, &
                message, Lmessage, fileName, LfileName, line, &
                filter_duplicates) &
                bind(C, name="SLIC_log_message_file_line_filter_bufferify")
            use iso_c_binding, only : C_BOOL, C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            integer(C_INT), value, intent(IN) :: Lmessage
            character(kind=C_CHAR), intent(IN) :: fileName(*)
            integer(C_INT), value, intent(IN) :: LfileName
            integer(C_INT), value, intent(IN) :: line
            logical(C_BOOL), value, intent(IN) :: filter_duplicates
        end subroutine c_log_message_file_line_filter_bufferify

        subroutine c_log_message(level, message) &
                bind(C, name="SLIC_log_message")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
        end subroutine c_log_message

        subroutine c_log_message_bufferify(level, message, Lmessage) &
                bind(C, name="SLIC_log_message_bufferify")
            use iso_c_binding, only : C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            integer(C_INT), value, intent(IN) :: Lmessage
        end subroutine c_log_message_bufferify

        subroutine c_log_message_filter(level, message, &
                filter_duplicates) &
                bind(C, name="SLIC_log_message_filter")
            use iso_c_binding, only : C_BOOL, C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            logical(C_BOOL), value, intent(IN) :: filter_duplicates
        end subroutine c_log_message_filter

        subroutine c_log_message_filter_bufferify(level, message, &
                Lmessage, filter_duplicates) &
                bind(C, name="SLIC_log_message_filter_bufferify")
            use iso_c_binding, only : C_BOOL, C_CHAR, C_INT
            implicit none
            integer(C_INT), value, intent(IN) :: level
            character(kind=C_CHAR), intent(IN) :: message(*)
            integer(C_INT), value, intent(IN) :: Lmessage
            logical(C_BOOL), value, intent(IN) :: filter_duplicates
        end subroutine c_log_message_filter_bufferify

        subroutine slic_finalize() &
                bind(C, name="SLIC_finalize")
            implicit none
        end subroutine slic_finalize

        ! splicer begin additional_interfaces
        ! splicer end additional_interfaces
    end interface

    interface SlicGenericOutputStream
        module procedure slic_genericoutputstream_ctor_default
        module procedure slic_genericoutputstream_ctor_format
    end interface SlicGenericOutputStream

    interface log_message
        module procedure slic_log_message_file_line
        module procedure slic_log_message_file_line_filter
        module procedure slic_log_message
        module procedure slic_log_message_filter
    end interface log_message

contains

    function slic_genericoutputstream_ctor_default(stream) &
            result(SHT_rv)
        use iso_c_binding, only : C_INT, C_PTR
        character(len=*), intent(IN) :: stream
        type(SlicGenericOutputStream) :: SHT_rv
        ! splicer begin class.GenericOutputStream.method.ctor_default
        type(C_PTR) :: SHT_prv
        SHT_prv = c_genericoutputstream_ctor_default_bufferify(stream, &
            len_trim(stream, kind=C_INT), SHT_rv%cxxmem)
        ! splicer end class.GenericOutputStream.method.ctor_default
    end function slic_genericoutputstream_ctor_default

    function slic_genericoutputstream_ctor_format(stream, format) &
            result(SHT_rv)
        use iso_c_binding, only : C_INT, C_PTR
        character(len=*), intent(IN) :: stream
        character(len=*), intent(IN) :: format
        type(SlicGenericOutputStream) :: SHT_rv
        ! splicer begin class.GenericOutputStream.method.ctor_format
        type(C_PTR) :: SHT_prv
        SHT_prv = c_genericoutputstream_ctor_format_bufferify(stream, &
            len_trim(stream, kind=C_INT), format, &
            len_trim(format, kind=C_INT), SHT_rv%cxxmem)
        ! splicer end class.GenericOutputStream.method.ctor_format
    end function slic_genericoutputstream_ctor_format

    subroutine slic_genericoutputstream_delete(obj)
        class(SlicGenericOutputStream) :: obj
        ! splicer begin class.GenericOutputStream.method.delete
        call c_genericoutputstream_delete(obj%cxxmem)
        ! splicer end class.GenericOutputStream.method.delete
    end subroutine slic_genericoutputstream_delete

    ! Return pointer to C++ memory.
    function slic_genericoutputstream_get_instance(obj) result (cxxptr)
        use iso_c_binding, only: C_PTR
        class(SlicGenericOutputStream), intent(IN) :: obj
        type(C_PTR) :: cxxptr
        cxxptr = obj%cxxmem%addr
    end function slic_genericoutputstream_get_instance

    subroutine slic_genericoutputstream_set_instance(obj, cxxmem)
        use iso_c_binding, only: C_PTR
        class(SlicGenericOutputStream), intent(INOUT) :: obj
        type(C_PTR), intent(IN) :: cxxmem
        obj%cxxmem%addr = cxxmem
        obj%cxxmem%idtor = 0
    end subroutine slic_genericoutputstream_set_instance

    function slic_genericoutputstream_associated(obj) result (rv)
        use iso_c_binding, only: c_associated
        class(SlicGenericOutputStream), intent(IN) :: obj
        logical rv
        rv = c_associated(obj%cxxmem%addr)
    end function slic_genericoutputstream_associated

    ! splicer begin class.GenericOutputStream.additional_functions
    ! splicer end class.GenericOutputStream.additional_functions

    function slic_is_initialized() &
            result(SHT_rv)
        use iso_c_binding, only : C_BOOL
        logical :: SHT_rv
        ! splicer begin function.is_initialized
        SHT_rv = c_is_initialized()
        ! splicer end function.is_initialized
    end function slic_is_initialized

    subroutine slic_create_logger(name, imask)
        use iso_c_binding, only : C_INT
        character(len=*), intent(IN) :: name
        character, value, intent(IN) :: imask
        ! splicer begin function.create_logger
        call c_create_logger_bufferify(name, len_trim(name, kind=C_INT), &
            imask)
        ! splicer end function.create_logger
    end subroutine slic_create_logger

    function slic_activate_logger(name) &
            result(SHT_rv)
        use iso_c_binding, only : C_BOOL, C_INT
        character(len=*), intent(IN) :: name
        logical :: SHT_rv
        ! splicer begin function.activate_logger
        SHT_rv = c_activate_logger_bufferify(name, &
            len_trim(name, kind=C_INT))
        ! splicer end function.activate_logger
    end function slic_activate_logger

    subroutine slic_get_active_logger_name(name)
        use iso_c_binding, only : C_INT
        character(len=*), intent(OUT) :: name
        ! splicer begin function.get_active_logger_name
        call c_get_active_logger_name_bufferify(name, &
            len(name, kind=C_INT))
        ! splicer end function.get_active_logger_name
    end subroutine slic_get_active_logger_name

    subroutine slic_add_stream_to_all_msg_levels(ls)
        type(SlicGenericOutputStream), intent(INOUT) :: ls
        ! splicer begin function.add_stream_to_all_msg_levels
        call c_add_stream_to_all_msg_levels(ls%cxxmem)
        ! splicer end function.add_stream_to_all_msg_levels
    end subroutine slic_add_stream_to_all_msg_levels

    subroutine slic_set_abort_on_error(status)
        use iso_c_binding, only : C_BOOL
        logical, value, intent(IN) :: status
        ! splicer begin function.set_abort_on_error
        logical(C_BOOL) SH_status
        SH_status = status  ! coerce to C_BOOL
        call c_set_abort_on_error(SH_status)
        ! splicer end function.set_abort_on_error
    end subroutine slic_set_abort_on_error

    function slic_is_abort_on_errors_enabled() &
            result(SHT_rv)
        use iso_c_binding, only : C_BOOL
        logical :: SHT_rv
        ! splicer begin function.is_abort_on_errors_enabled
        SHT_rv = c_is_abort_on_errors_enabled()
        ! splicer end function.is_abort_on_errors_enabled
    end function slic_is_abort_on_errors_enabled

    subroutine slic_set_abort_on_warning(status)
        use iso_c_binding, only : C_BOOL
        logical, value, intent(IN) :: status
        ! splicer begin function.set_abort_on_warning
        logical(C_BOOL) SH_status
        SH_status = status  ! coerce to C_BOOL
        call c_set_abort_on_warning(SH_status)
        ! splicer end function.set_abort_on_warning
    end subroutine slic_set_abort_on_warning

    function slic_is_abort_on_warnings_enabled() &
            result(SHT_rv)
        use iso_c_binding, only : C_BOOL
        logical :: SHT_rv
        ! splicer begin function.is_abort_on_warnings_enabled
        SHT_rv = c_is_abort_on_warnings_enabled()
        ! splicer end function.is_abort_on_warnings_enabled
    end function slic_is_abort_on_warnings_enabled

    subroutine slic_log_message_file_line(level, message, fileName, &
            line)
        use iso_c_binding, only : C_INT
        integer(C_INT), value, intent(IN) :: level
        character(len=*), intent(IN) :: message
        character(len=*), intent(IN) :: fileName
        integer(C_INT), value, intent(IN) :: line
        ! splicer begin function.log_message_file_line
        call c_log_message_file_line_bufferify(level, message, &
            len_trim(message, kind=C_INT), fileName, &
            len_trim(fileName, kind=C_INT), line)
        ! splicer end function.log_message_file_line
    end subroutine slic_log_message_file_line

    subroutine slic_log_message_file_line_filter(level, message, &
            fileName, line, filter_duplicates)
        use iso_c_binding, only : C_BOOL, C_INT
        integer(C_INT), value, intent(IN) :: level
        character(len=*), intent(IN) :: message
        character(len=*), intent(IN) :: fileName
        integer(C_INT), value, intent(IN) :: line
        logical, value, intent(IN) :: filter_duplicates
        ! splicer begin function.log_message_file_line_filter
        logical(C_BOOL) SH_filter_duplicates
        SH_filter_duplicates = filter_duplicates  ! coerce to C_BOOL
        call c_log_message_file_line_filter_bufferify(level, message, &
            len_trim(message, kind=C_INT), fileName, &
            len_trim(fileName, kind=C_INT), line, SH_filter_duplicates)
        ! splicer end function.log_message_file_line_filter
    end subroutine slic_log_message_file_line_filter

    subroutine slic_log_message(level, message)
        use iso_c_binding, only : C_INT
        integer(C_INT), value, intent(IN) :: level
        character(len=*), intent(IN) :: message
        ! splicer begin function.log_message
        call c_log_message_bufferify(level, message, &
            len_trim(message, kind=C_INT))
        ! splicer end function.log_message
    end subroutine slic_log_message

    subroutine slic_log_message_filter(level, message, &
            filter_duplicates)
        use iso_c_binding, only : C_BOOL, C_INT
        integer(C_INT), value, intent(IN) :: level
        character(len=*), intent(IN) :: message
        logical, value, intent(IN) :: filter_duplicates
        ! splicer begin function.log_message_filter
        logical(C_BOOL) SH_filter_duplicates
        SH_filter_duplicates = filter_duplicates  ! coerce to C_BOOL
        call c_log_message_filter_bufferify(level, message, &
            len_trim(message, kind=C_INT), SH_filter_duplicates)
        ! splicer end function.log_message_filter
    end subroutine slic_log_message_filter

    ! splicer begin additional_functions
    ! splicer end additional_functions

    function genericoutputstream_eq(a,b) result (rv)
        use iso_c_binding, only: c_associated
        type(SlicGenericOutputStream), intent(IN) ::a,b
        logical :: rv
        if (c_associated(a%cxxmem%addr, b%cxxmem%addr)) then
            rv = .true.
        else
            rv = .false.
        endif
    end function genericoutputstream_eq

    function genericoutputstream_ne(a,b) result (rv)
        use iso_c_binding, only: c_associated
        type(SlicGenericOutputStream), intent(IN) ::a,b
        logical :: rv
        if (.not. c_associated(a%cxxmem%addr, b%cxxmem%addr)) then
            rv = .true.
        else
            rv = .false.
        endif
    end function genericoutputstream_ne

end module axom_slic
