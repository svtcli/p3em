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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdatomic.h>
#include <sys/time.h>


#define P3EM_MAX_NAME_LEN 256

// Determine the local MPI rank from common environment variables (C++ style).
static inline int p3em_get_local_rank(void) {
    const char *env_vars[] = {
        "OMPI_COMM_WORLD_LOCAL_RANK",
        "MV2_COMM_WORLD_LOCAL_RANK",
        "MPI_LOCALRANKID",
        "SLURM_LOCALID",
        "PMI_LOCAL_RANK"
    };
    for (size_t i = 0; i < sizeof(env_vars)/sizeof(env_vars[0]); ++i) {
        const char *v = getenv(env_vars[i]);
        if (v) return atoi(v);
    }
    return 0;
}

typedef struct {
  _Atomic double latestValue;
  pid_t scriptPid;
  int pipefd[2];
  FILE* stream;
  char buffer[256];
  char prName[P3EM_MAX_NAME_LEN];
  pthread_t monitorThread;
  int should_stop;  // Flag to signal thread termination
  int initialized;  // Track initialization state
  int shmRank;
  // moved global state
  int enabled;
  const char *cpu_path;
  const char *gpu_path;
} p3em_t;

// Global environment variables (managed internally)
/* Global state moved into p3em_t context */
// Function declarations (static definitions follow)
static void* p3em_launchScriptAndMonitor(void* arg);
static double p3em_getLatestValue(p3em_t* p3em);
static inline double p3em_now(const p3em_t *ctx);

// Initialise monitoring from environment variables.
// Creates monitors for each defined env var (P3EM_CPU, P3EM_GPU).
// Returns 0 on success, -1 on allocation failure.
static int p3em_init(p3em_t *ctx) {
    const char *en = getenv("P3EM_ENABLED");
    ctx->enabled = en && strcmp(en, "1") == 0;
    if (!ctx->enabled) return 0;
    ctx->cpu_path = getenv("P3EM_CPU");
    ctx->gpu_path = getenv("P3EM_GPU");
    if (!ctx->cpu_path && !ctx->gpu_path) {
        fprintf(stderr, "[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined.\n");
        exit(EXIT_FAILURE);
    }
    // Prefer CPU monitor; if not present, use GPU
    const char *path = ctx->cpu_path ? ctx->cpu_path : ctx->gpu_path;
    atomic_init(&ctx->latestValue, -42.0);
    ctx->scriptPid = -420;
    ctx->stream = NULL;
    ctx->should_stop = 0;
    ctx->initialized = 1;
    ctx->shmRank = p3em_get_local_rank();
    strcpy(ctx->prName, path);
    if (ctx->shmRank == 0) {
        ctx->initialized = !pthread_create(&ctx->monitorThread, NULL, p3em_launchScriptAndMonitor, ctx);
        if (!ctx->initialized) return -1;
        while (p3em_getLatestValue(ctx) <= 0) usleep(1000);
    }
    return 0;
}

// Shut down all monitors that were created
static void p3em_cleanup(p3em_t *ctx) {
    if (ctx->initialized && ctx->shmRank == 0) {
        ctx->should_stop = 1;
        if (ctx->scriptPid > 0) {
            killpg(ctx->scriptPid, SIGKILL);
            waitpid(ctx->scriptPid, NULL, 0);
        }
        pthread_join(ctx->monitorThread, NULL);
        ctx->initialized = 0;
    }
}

static double p3em_getLatestValue(p3em_t* p3em) {
   if(p3em->shmRank){ return 0.0 ;} // Non-0 ranks always return 0
   return p3em ? atomic_load(&p3em->latestValue) : -42.0;
 }

// Return current time: wall‑clock if disabled, summed CPU+GPU values if enabled
static inline double p3em_now(const p3em_t *ctx) {
    if (!ctx->enabled) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
    }
    return p3em_getLatestValue((p3em_t*)ctx);
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
    double value;
    if (sscanf(p3em->buffer, "%lf", &value) == 1) atomic_store(&p3em->latestValue, value);
  }
  if (p3em->stream) {
    fclose(p3em->stream);
    p3em->stream = NULL;
  }
  return NULL;
}

#endif
