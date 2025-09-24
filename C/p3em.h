//   Copyright(C) 2025 Salvatore Cielo, LRZ
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.

#ifndef p3em
#define p3em

#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <signal.h>

std::atomic<int> latestValue{-42};
pid_t scriptPid=-420;

void launchScriptAndMonitor() {
    int pipefd[2];
    pipe(pipefd);
    scriptPid = fork();
    if (scriptPid == 0) {    // Child process
      setpgid(0, 0); // Create new process group
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      execlp("stdbuf", "stdbuf", "-oL", "-eL", "../p3em.sh", nullptr);
      exit(1); // If exec fails
    }

    // Parent process: read from pipe
    close(pipefd[1]);
    FILE* stream = fdopen(pipefd[0], "r");
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stream)) {
        std::istringstream iss(buffer);
        std::string word;
        while (iss >> word); // get last word
        try {
            latestValue = std::stoi(word);
        } catch (...) {}
    }
    fclose(stream);
}

int getLatestValue() {
    return latestValue.load();
}

#endif
