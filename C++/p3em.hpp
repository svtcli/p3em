//   Copyright(C) 2025 Salvatore Cielo, LRZ
//   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
//   Copyright(C) 2025 Ivan Pribec, LRZ
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.

#ifndef P3EM_H  // Use uppercase and _H suffix
#define P3EM_H
#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>

class p3em {
private:
  std::atomic<int> latestValue{-42};
  pid_t scriptPid = -420;
  int pipefd[2];  // Declare as member
  FILE* stream = nullptr;  // Initialize to nullptr, set later
  char buffer[256];
  std::string prName;
  std::thread monitorThread;

  void launchScriptAndMonitor() {
    pipe(pipefd);
    scriptPid = fork();
    if (scriptPid == 0) {    // Child process
      setpgid(0, 0); // Create new process group
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      execlp("stdbuf", "stdbuf", "-oL", "-eL", prName.c_str(), nullptr);  // Use prName.c_str()
      exit(1); // If exec fails
    }
    // Parent process: read from pipe
    close(pipefd[1]);
    stream = fdopen(pipefd[0], "r");  // Initialize stream here

    while (fgets(buffer, sizeof(buffer), stream)) {
      std::istringstream iss(buffer);
      std::string word;
      while (iss >> word); // get last word
      try {
          latestValue = std::stoi(word);
      } catch (...) {}
    }
    if (stream) fclose(stream);
  }

public:
  p3em(const std::string& name) : prName(name) {
    // Start thread after all members are initialized
    monitorThread = std::thread(&p3em::launchScriptAndMonitor, this);
    while(getLatestValue()<=0){ // Wait for 1st read
     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  ~p3em() {
    if (scriptPid > 0)            killpg(scriptPid, SIGKILL);
    if (monitorThread.joinable()) monitorThread.join();
  }

  int getLatestValue() {
    return latestValue.load();
  }
};

#endif
