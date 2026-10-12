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
#include <iomanip>

class P3emTimer {
    double startSec;
public:
    P3emTimer() {
        p3em::init();
        startSec = p3em::now();
    }
    double elapsed() const { return p3em::now() - startSec; }
};

int main(int argc, char** argv) {
    int shmRank, shmSize, rank, size;
    double value;
    MPI_Comm shmComm;
    MPI_Request shmReq;
    MPI_Status shmStatus;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &shmComm);
    MPI_Comm_rank(shmComm, &shmRank);
    MPI_Comm_size(shmComm, &shmSize);

    P3emTimer timer;

    // Simulate usage
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(230));
        double elapsed = timer.elapsed();
        std::cout << "[r " << rank << ":" << shmRank << "] Elapsed seconds: "
                  << std::fixed << std::setprecision(6) << elapsed << std::endl;
        // Average among node tasks
        double avg = elapsed / static_cast<double>(shmSize);
        MPI_Ibcast(&avg, 1, MPI_DOUBLE, 0, shmComm, &shmReq);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        MPI_Wait(&shmReq, &shmStatus);
        MPI_Barrier(MPI_COMM_WORLD);
        std::cout << "[r " << rank << ":" << shmRank << "] Latest value bcast: "
                  << std::fixed << std::setprecision(6) << avg << std::endl;
    }
    MPI_Finalize();
    return 0;
}
