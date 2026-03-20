#ifndef P3EMCHRONO
#define P3EMCHRONO

#include <chrono>
#include <cstdlib>
#include <optional>
#include <string>
#include <iostream>
#include "p3em.hpp"

namespace p3emChrono {
  class system_clock {
  private:
    static inline bool enabled = false;
    static inline std::optional<p3em> cpu, gpu;
    static inline std::optional<int> shmRank;

    static bool check_enabled() {

      const char *env_nbl = "P3EM_ENABLED", *env_gpu = "P3EM_GPU", *env_cpu = "P3EM_CPU";
      const char *str_nbl = std::getenv(env_nbl), *str_gpu = std::getenv(env_gpu), *str_cpu = std::getenv(env_cpu);

      if (!str_nbl || std::string(str_nbl) != "1") {
        return false;
      } // Check if TIME_HACK_ENABLED is set to "1"

      if (!str_gpu && !str_cpu) {
        std::cerr << "[p3emHack] disabled: neither " << env_gpu << " nor " << env_cpu << " defined" << std::endl;
        return false;
      } // Check if at least one of GPU or CPU is defined

      return true;
    }

    static int mpi_local_rank_on_node() {
      for (const char* env_var : {
        "OMPI_COMM_WORLD_LOCAL_RANK",  // OpenMPI
        "MV2_COMM_WORLD_LOCAL_RANK",   // MVAPICH2
        "MPI_LOCALRANKID",             // MPICH
        "SLURM_LOCALID",               // SLURM
        "PMI_LOCAL_RANK",              // PMI
      }) {
        const char* str = std::getenv(env_var);
        if (str) { return std::stoi(str); }
      }
      return 0;
    } // Adapted from Kokkos::Impl

  public:
    static void init() {
      enabled = check_enabled();
      if (enabled) {
        shmRank.emplace(mpi_local_rank_on_node());
        std::cout<<"shmRank " << shmRank.value()<<std::endl;
        const char* str_gpu = std::getenv("P3EM_GPU");
        const char* str_cpu = std::getenv("P3EM_CPU");

        if (str_gpu) {
          std::cerr << "[p3emHack] GPU monitoring enabled: " << str_gpu << std::endl;
          gpu.emplace(std::string(str_gpu), shmRank.value());
        }

        if (str_cpu) {
          std::cerr << "[p3emHack] CPU monitoring enabled: " << str_cpu << std::endl;
          cpu.emplace(std::string(str_cpu), shmRank.value());
        }
      }
    }

    static std::chrono::system_clock::time_point now() {
      if (!enabled) { return std::chrono::system_clock::now(); }

      long long total_value = 0;
      if (gpu.has_value()) { total_value += gpu->getLatestValue(); }
      if (cpu.has_value()) { total_value += cpu->getLatestValue(); }

      return std::chrono::system_clock::time_point(
        std::chrono::seconds(total_value)
      );
    }
  };

}

#endif
