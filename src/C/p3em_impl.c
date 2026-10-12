#include "p3em.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdatomic.h>

static void p3em_reader_stop(p3em_reader_t *r);
static void* p3em_launchScriptAndMonitor(void* arg);

static int p3em_reader_start(p3em_reader_t *r, const char *path) {
    atomic_init(&r->latestValue, -42.0);
    r->scriptPid = -420;
    r->stream = NULL;
    r->should_stop = 0;
    atomic_init(&r->done, 0);
    snprintf(r->prName, sizeof(r->prName), "%s", path);
    r->initialized = !pthread_create(&r->monitorThread, NULL, p3em_launchScriptAndMonitor, r);
    if (!r->initialized) return -1;
    while (atomic_load(&r->latestValue) < 0) {
        if (atomic_load(&r->done)) {
            fprintf(stderr, "[p3em] ERROR: meter script exited before producing a value: %s\n", path);
            p3em_reader_stop(r);
            return -1;
        }
        usleep(1000);
    }
    return 0;
}

static void p3em_reader_stop(p3em_reader_t *r) {
    if (!r->initialized) return;
    r->should_stop = 1;
    if (r->scriptPid > 0) {
        killpg(r->scriptPid, SIGKILL);
        waitpid(r->scriptPid, NULL, 0);
    }
    pthread_join(r->monitorThread, NULL);
    r->initialized = 0;
}

static int p3em_init_impl(p3em_t *ctx) {
    const char *en = getenv("P3EM_ENABLED");
    ctx->enabled = en && strcmp(en, "1") == 0;
    ctx->cpu.initialized = 0;
    ctx->gpu.initialized = 0;
    if (!ctx->enabled) return 0;
    ctx->cpu_path = getenv("P3EM_CPU");
    ctx->gpu_path = getenv("P3EM_GPU");
    if (!ctx->cpu_path && !ctx->gpu_path) {
        fprintf(stderr, "[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined.\n");
        exit(EXIT_FAILURE);
    }
    ctx->shmRank = p3em_get_local_rank();
    if (ctx->shmRank) return 0;
    if (ctx->gpu_path) {
        fprintf(stderr, "[p3em] GPU monitoring enabled: %s\n", ctx->gpu_path);
        if (p3em_reader_start(&ctx->gpu, ctx->gpu_path) != 0) return -1;
    }
    if (ctx->cpu_path) {
        fprintf(stderr, "[p3em] CPU monitoring enabled: %s\n", ctx->cpu_path);
        if (p3em_reader_start(&ctx->cpu, ctx->cpu_path) != 0) {
            p3em_reader_stop(&ctx->gpu);
            return -1;
        }
    }
    return 0;
}

static void p3em_cleanup_impl(p3em_t *ctx) {
    p3em_reader_stop(&ctx->cpu);
    p3em_reader_stop(&ctx->gpu);
}

double p3em_getLatestValue_internal(p3em_t *p3em) {
    if (!p3em) return -42.0;
    if (p3em->shmRank) return 0.0;
    double total = 0.0;
    if (p3em->cpu.initialized) total += atomic_load(&p3em->cpu.latestValue);
    if (p3em->gpu.initialized) total += atomic_load(&p3em->gpu.latestValue);
    return total;
}

static void* p3em_launchScriptAndMonitor(void* arg) {
    p3em_reader_t* r = (p3em_reader_t*)arg;
    if (pipe(r->pipefd) == -1) { atomic_store(&r->done, 1); return NULL; }
    r->scriptPid = fork();
    if (r->scriptPid == 0) {
        setpgid(0, 0);
        dup2(r->pipefd[1], STDOUT_FILENO);
        close(r->pipefd[0]);
        close(r->pipefd[1]);
        execlp("stdbuf", "stdbuf", "-oL", "-eL", r->prName, NULL);
        exit(1);
    }
    close(r->pipefd[1]);
    r->stream = fdopen(r->pipefd[0], "r");
    if (!r->stream) { atomic_store(&r->done, 1); return NULL; }
    while (!r->should_stop && fgets(r->buffer, sizeof(r->buffer), r->stream)) {
        double value;
        if (sscanf(r->buffer, "%lf", &value) == 1) atomic_store(&r->latestValue, value);
    }
    if (r->stream) {
        fclose(r->stream);
        r->stream = NULL;
    }
    atomic_store(&r->done, 1);
    return NULL;
}

int p3em_init_ctx(p3em_t *ctx) {
    return p3em_init_impl(ctx);
}

void p3em_cleanup_ctx(p3em_t *ctx) {
    p3em_cleanup_impl(ctx);
}
