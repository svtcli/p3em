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

#include <iostream>
#include <thread>
#include <mpi.h>
#include "p3em.hpp"

int main(int argc, char** argv)  {

    int shmRank, shmSize, value;
    int rank, size;
    MPI_Comm    shmComm;
    MPI_Request shmReq;
    MPI_Status  shmStatus;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_split_type(MPI_COMM_WORLD,MPI_COMM_TYPE_SHARED,0,MPI_INFO_NULL,&shmComm);
    MPI_Comm_rank(shmComm, &shmRank);
    MPI_Comm_size(shmComm, &shmSize);

    std::cout << "[r "<<rank<<":"<<shmRank<<"] Before p3em "<< std::endl;
    p3em *xpu = new p3em("../../meters/xpuSmi.sh", shmRank),
         *prf = new p3em("../../meters/perf.sh"  , shmRank);
    std::cout << "[r "<<rank<<":"<<shmRank<<"] After p3em "<< std::endl;

    // Simulate usage
    for (int i = 0; i < 1; ++i) {
        // This is what p3em returns
        value = xpu->getLatestValue();
        auto tmp = prf->getLatestValue();

        std::cout << "[r "<<rank<<":"<<shmRank<<"] Values native (xpu/perf): " << value <<" / " <<tmp <<std::endl;
        value += tmp;
//        MPI_Barrier(MPI_COMM_WORLD); // only for better output, you don't want this

        // You may want an average, instead
        value = static_cast<int>(value *1.0/shmSize);

        MPI_Ibcast(&value,1,MPI_INTEGER, 0, shmComm, &shmReq);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        MPI_Wait(&shmReq,&shmStatus); // barrier here

        std::cout << "[r "<<rank<<":"<<shmRank<<"] Latest value bcast: " << value << std::endl;
        MPI_Barrier(MPI_COMM_WORLD); // only for better output, you don't want this

    }
    MPI_Finalize();
    delete xpu;
    delete prf;
    return 0;
}
