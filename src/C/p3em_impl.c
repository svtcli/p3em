#include "p3em.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdatomic.h>
#include <sys/time.h>

static int p3em_init_impl(p3em_t *ctx) {
    const char *en = getenv("P3EM_ENABLED");
    ctx->enabled = en && strcmp(en, "1") == 0;
    if (!ctx->enabled) return 0;
    ctx->cpu_path = getenv("P3EM_CPU");
    ctx->gpu_path = getenv("P3EM_GPU");
    if (!ctx->cpu_path && !ctx->gpu_path) {
        fprintf(stderr, "[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined.\n");
        exit(EXIT_FAILURE);
    }
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
        while (p3em_getLatestValue_internal(ctx) <= 0) usleep(1000);
    }
    return 0;
}

static void p3em_cleanup_impl(p3em_t *ctx) {
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

double p3em_getLatestValue_internal(p3em_t *p3em) {
    if (p3em->shmRank) return 0.0;
    return p3em ? atomic_load(&p3em->latestValue) : -42.0;
}

void* p3em_launchScriptAndMonitor(void* arg) {
    p3em_t* p3em = (p3em_t*)arg;
    if (pipe(p3em->pipefd) == -1) return NULL;
    p3em->scriptPid = fork();
    if (p3em->scriptPid == 0) {
        setpgid(0, 0);
        dup2(p3em->pipefd[1], STDOUT_FILENO);
        close(p3em->pipefd[0]);
        close(p3em->pipefd[1]);
        execlp("stdbuf", "stdbuf", "-oL", "-eL", p3em->prName, NULL);
        exit(1);
    }
    close(p3em->pipefd[1]);
    p3em->stream = fdopen(p3em->pipefd[0], "r");
    if (!p3em->stream) return NULL;
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

int p3em_init_ctx(p3em_t *ctx) {
    return p3em_init_impl(ctx);
}

void p3em_cleanup_ctx(p3em_t *ctx) {
    p3em_cleanup_impl(ctx);
}
