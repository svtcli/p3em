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

#ifndef P3EM_H
#define P3EM_H
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <vector>

class p3em {
private:
  std::atomic<int> latestValue{-42};
  pid_t scriptPid = -420;
  int pipefd[2];
  FILE* stream = nullptr;
  char buffer[256];
  const int shmRank;
  std::string prName;
  std::thread monitorThread;

  void launchScriptAndMonitor() {
    pipe(pipefd);
    scriptPid = fork();
    if (scriptPid == 0) {
      setpgid(0, 0);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      execlp("stdbuf", "stdbuf", "-oL", "-eL", prName.c_str(), nullptr);
      exit(1);
    }
    close(pipefd[1]);
    stream = fdopen(pipefd[0], "r");

    while (fgets(buffer, sizeof(buffer), stream)) {
      std::istringstream iss(buffer);
      std::string word;
      while (iss >> word);
      try {
          latestValue = std::stoi(word);
      } catch (...) {}
    }
    if (stream) fclose(stream);
  }

  static int mpi_local_rank_on_node() {
    for (const char* env_var : {
      "OMPI_COMM_WORLD_LOCAL_RANK",
      "MV2_COMM_WORLD_LOCAL_RANK",
      "MPI_LOCALRANKID",
      "SLURM_LOCALID",
      "PMI_LOCAL_RANK",
    }) {
      const char* str = std::getenv(env_var);
      if (str) { return std::stoi(str); }
    }
    return 0;
  }

  static bool check_enabled() {
    const char *str = std::getenv("P3EM_ENABLED");
    return str && std::string(str) == "1";
  }

public:
  static inline bool enabled = false;
  static std::optional<p3em> cpuReader;
  static std::optional<p3em> gpuReader;

  p3em(const std::string& name, const int localRank=0) : prName(name), shmRank(localRank) {
     std::cout << "p3em init'd" << std::endl;
    if (!shmRank) {
      monitorThread = std::thread(&p3em::launchScriptAndMonitor, this);
      while (getLatestValue() < 0) {
       std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  ~p3em() {
    if (!shmRank) {
      if (scriptPid > 0)            killpg(scriptPid, SIGKILL);
      if (monitorThread.joinable()) monitorThread.join();
   }
  }

  int getLatestValue() const {
    return shmRank ? 0 : latestValue.load();
  }

  static void init() {
    enabled = check_enabled();
    if (enabled) {
      int rank = mpi_local_rank_on_node();
       const char* str_gpu = std::getenv("P3EM_GPU");
       const char* str_cpu = std::getenv("P3EM_CPU");

       // Abort if enabled but no device specified
       if (!str_cpu && !str_gpu) {
           std::cerr << "[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined." << std::endl;
           std::exit(EXIT_FAILURE);
       }

       if (str_gpu) {
        std::cerr << "[p3em] GPU monitoring enabled: " << str_gpu << std::endl;
        gpuReader.emplace(std::string(str_gpu), rank);
      }

      if (str_cpu) {
        std::cerr << "[p3em] CPU monitoring enabled: " << str_cpu << std::endl;
        cpuReader.emplace(std::string(str_cpu), rank);
      }
    }
  }

template<class T = double>
static T now() {
    if (!enabled) {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return static_cast<T>(tv.tv_sec) + static_cast<T>(tv.tv_usec) / static_cast<T>(1e6);
    }

    T total = T(0);
    if (cpuReader.has_value()) total += static_cast<T>(cpuReader->getLatestValue());
    if (gpuReader.has_value()) total += static_cast<T>(gpuReader->getLatestValue());
    return total;
  }
};

inline std::optional<p3em> p3em::cpuReader;
inline std::optional<p3em> p3em::gpuReader;

namespace p3emChrono {
  class system_clock {
  public:
    static void init() { p3em::init(); }

    template<typename Clock = std::chrono::system_clock>
    static typename Clock::time_point now() {
      if (!p3em::enabled) return Clock::now();
        return typename Clock::time_point(
          std::chrono::duration_cast<typename Clock::duration>(std::chrono::duration<double>(p3em::now()))
        );
    }
  };
}

#endif
