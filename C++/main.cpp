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

    MPI_Init(&argc, &argv);
    int world_size;  // Total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int rank;  // Current process ID
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    p3em myem("../p3em.sh", rank);
    // Simulate usage
    for (int i = 0; i < 10; ++i) {
        std::cout << "[Rank "<<rank<<"] Latest value: " << myem.getLatestValue() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}
