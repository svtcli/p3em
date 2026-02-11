/*
module load ear
icx -o ear ear.c -lear -learld -lear.ompi -lear_api  -I$EAR_INSTALL_PATH/include -L$EAR_INSTALL_PATH/lib
export LD_LIBRARY_PATH=$EAR_INSTALL_PATH/lib:$LD_LIBRARY_PATH
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ear.h>
int main(int argc, char *argv[]) {
  unsigned long e0, e1, ed, t0, t1, td;
  unsigned long interval_ms = (argc > 1) ? atoi(argv[1]) : 100;
  if (ear_connect() != EAR_SUCCESS) {
    fprintf(stderr, "Error: Failed to connect to EAR\n");
    return 1;
  }
  if (ear_energy(&e0, &t0) != EAR_SUCCESS) {
    fprintf(stderr, "Error: Failed to read initial energy\n");
    return 1;
  }
  while (1) {
    usleep(interval_ms * 1000);
    if (ear_energy(&e1, &t1) == EAR_SUCCESS) {
      ear_energy_diff(e0,e1,&ed,t0,t1,&td);
      printf("%lu\n", ed);
    }
  }
  return 0;
}
