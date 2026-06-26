//   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
//   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
//   Copyright(C) 2025 Ivan Pribec, Leibniz-Rechenzentrum
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.

#include <stdio.h>
#include <stdlib.h>
//#include "p3em.h"
#include <mpi.h>

#include "my_timer.h"


int main(int argc, char* argv[]) {
  int shmRank, shmSize;
  double elapsed;
  MPI_Comm shmComm;
  MPI_Request shmReq;
  MPI_Status shmStatus;

  MPI_Init(&argc, &argv);
  MPI_Comm_split_type(MPI_COMM_WORLD,MPI_COMM_TYPE_SHARED,0,MPI_INFO_NULL,&shmComm);
  MPI_Comm_rank(shmComm, &shmRank);
  MPI_Comm_size(shmComm, &shmSize);

  my_timer timer; my_timer_init(&timer);

  my_timer_start(&timer);
  for (int i = 0; i < 10; ++i) {
    usleep(300000);
    elapsed = my_timer_elapsed(&timer);
    double avg = elapsed / (double)shmSize;
    MPI_Ibcast(&avg, 1, MPI_DOUBLE, 0, shmComm, &shmReq);
    MPI_Wait(&shmReq, &shmStatus);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("[NodeRank %i] Elapsed: %f\n", shmRank, elapsed);
    printf("[NodeRank %i] Latest value bcast: %f\n", shmRank, avg);
  }

  my_timer_cleanup(&timer);
  MPI_Finalize();
  return 0;
}
