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
   integer(c_int) :: val
   integer :: i

   ! Create handle (pass C string "../p3em.sh\0")
   handle = handle_to_p3em("../p3em.sh" // c_null_char)  ! wrong order

   do i = 1, 10
      val = p3em_getLatestValue(handle)
      print *, "Latest value:", val
      call usleep(30000)  ! 30 ms
   end do

   call free_handle(handle)

end program
