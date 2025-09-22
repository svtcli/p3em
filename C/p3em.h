//   Copyright(C) 2025 Salvatore Cielo, LRZ
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
//  License. You may obtain a copy of the License at    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.

#ifndef P3EM_H
#define P3EM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdatomic.h>

#define P3EM_MAX_NAME_LEN 256

typedef struct {
  atomic_int latestValue;
  pid_t scriptPid;
  int pipefd[2];
  FILE* stream;
  char buffer[256];
  char prName[P3EM_MAX_NAME_LEN];
  pthread_t monitorThread;
  int should_stop;  // Flag to signal thread termination
  int initialized;  // Track initialization state
} p3em_t;

// Function declarations
int   p3em_init(p3em_t* p3em, const char* name);
void  p3em_cleanup(p3em_t* p3em);
int   p3em_getLatestValue(p3em_t* p3em);
void* p3em_launchScriptAndMonitor(void* arg); // Internal

// Implementation
int p3em_init(p3em_t* p3em, const char* name) {
  if (!p3em || !name) return -1;
  // Check name length
  if (strlen(name) >= P3EM_MAX_NAME_LEN) return -1;
  // Initialize members
  atomic_init(&p3em->latestValue, -42);
  p3em->scriptPid = -420;
  p3em->stream = NULL;
  p3em->should_stop = 0;
  p3em->initialized = 0;
  strcpy(p3em->prName, name);  // Copy the name
  // Start monitoring thread
  p3em->initialized = pthread_create(&p3em->monitorThread,NULL,p3em_launchScriptAndMonitor,p3em);
  if(p3em->initialized != 0) return -1;
  // Wait for first read; small delay to prevent busy waiting
  while(p3em_getLatestValue(p3em)<=0) usleep(1000);
  return 0;
}

void p3em_cleanup(p3em_t* p3em) {
  if (!p3em || !p3em->initialized) return;
  p3em->should_stop = 1;  // Signal thread to stop
  if (p3em->scriptPid > 0) {   // Kill the script process group
    killpg(p3em->scriptPid, SIGKILL);
    waitpid(p3em->scriptPid, NULL, 0); // Clean up zombie process
  }
  pthread_join(p3em->monitorThread, NULL);   // Wait for thread to finish
  p3em->initialized = 0;
}

int p3em_getLatestValue(p3em_t* p3em) {
  return p3em ? atomic_load(&p3em->latestValue) : -42;
}

void* p3em_launchScriptAndMonitor(void* arg) {
  p3em_t* p3em = (p3em_t*)arg;
  if (pipe(p3em->pipefd) == -1) return NULL;
  p3em->scriptPid = fork();

  if (p3em->scriptPid == 0) {    // Child process
    setpgid(0, 0); // Create new process group
    dup2(p3em->pipefd[1], STDOUT_FILENO);
    close(p3em->pipefd[0]);
    close(p3em->pipefd[1]);
    execlp("stdbuf", "stdbuf", "-oL", "-eL", p3em->prName, NULL);
    exit(1); // If exec fails
  }
  // Parent process: read from pipe
  close(p3em->pipefd[1]);
  p3em->stream = fdopen(p3em->pipefd[0], "r");
  if (!p3em->stream) { return NULL; }

  while (!p3em->should_stop && fgets(p3em->buffer, sizeof(p3em->buffer), p3em->stream)) {
    int value;
    if (sscanf(p3em->buffer, "%d", &value) == 1) atomic_store(&p3em->latestValue, value);
  }
  if (p3em->stream) {
    fclose(p3em->stream);
    p3em->stream = NULL;
  }
  return NULL;
}

#endif
