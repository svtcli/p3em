// SPDX-License-Identifier: Apache-2.0
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

static inline int p3em_get_local_rank(void) {
    const char *env_vars[] = {"OMPI_COMM_WORLD_LOCAL_RANK","MV2_COMM_WORLD_LOCAL_RANK","MPI_LOCALRANKID","SLURM_LOCALID","PMI_LOCAL_RANK"};
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
  int should_stop;
  int initialized;
  int shmRank;
  int enabled;
  const char *cpu_path;
  const char *gpu_path;
} p3em_t;

// API prototypes
int p3em_init_ctx(p3em_t *ctx);
void p3em_cleanup_ctx(p3em_t *ctx);
double p3em_getLatestValue_internal(p3em_t *p3em);
int p3em_getLatestValue(void *handle);
void* p3em_launchScriptAndMonitor(void* arg);
static inline double p3em_now(const p3em_t *ctx) {
    if (!ctx->enabled) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
    }
    return p3em_getLatestValue_internal((p3em_t*)ctx);
}

#endif // P3EM_H
