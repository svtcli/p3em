!   Copyright(C) 2025 Ivan Pribec, LRZ
!   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
!   Copyright(C) 2025 Salvatore Cielo, LRZ
!
!  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
!  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
!
!  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
!  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
!  language governing permissions and limitations under the License.

module p3em_mod
use, intrinsic :: iso_fortran_env, only: error_unit
use, intrinsic :: iso_c_binding

implicit none
private

public :: p3em_t

type :: p3em_t
    type(c_ptr), private :: handle = c_null_ptr
contains
    procedure, non_overridable, public :: get_energy
    final :: cleanup
end type

include "p3em.fi"

contains

    integer function get_energy(p3em)
        class(p3em_t), intent(inout) :: p3em
        if (.not. c_associated(p3em%handle)) then
            block
                integer(c_int) :: stat
                stat = p3em_init(p3em%handle,"../p3em.sh"//c_null_char)
                if (stat /= 0) then
                    write(error_unit,*) "p3em_mod: Warning: p3em_t encounted an error during initialization."
                end if
            end block
        end if
        get_energy = p3em_getlatestvalue(p3em%handle)
    end function

    subroutine cleanup(p3em)
        type(p3em_t), intent(inout) :: p3em
        if (c_associated(p3em%handle)) then
            call p3em_cleanup(p3em%handle)
        end if
    end subroutine

end module

program main
use p3em_mod, only: p3em_t
implicit none

interface
  subroutine usleep(usec) bind(C, name="usleep")
     use, intrinsic :: iso_c_binding, only: c_int
     integer(c_int), value :: usec
  end subroutine
end interface

! Use a block to trigger the finalizer!
block
    type(p3em_t) :: em
    integer :: k, val

    do k = 1, 10
        call usleep(3000)
        val = em%get_energy()
        print *, "Energy consumed: ", val
    end do

    ! Finalizer called here
end block

end program
