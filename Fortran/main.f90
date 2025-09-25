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

program main
   use iso_c_binding
   implicit none

   include "p3em.fi"

   interface
      subroutine usleep(usec) bind(C, name="usleep")
         import :: c_int
         integer(c_int), value :: usec
      end subroutine
   end interface

   type(c_ptr) :: handle
   integer(c_int) :: val, istat
   integer :: i

   istat = p3em_init(handle,"../p3em.sh"//c_null_char)
   do i = 1, 10
      val = p3em_getLatestValue(handle)
      print *, "Latest value:", val
      call usleep(30000)  ! 30 ms
   end do

   call p3em_cleanup(handle)

end program
