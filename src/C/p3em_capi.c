#include "p3em.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*
 * Compatibility C API for the Fortran interface.
 * The original C API used environment variables; this wrapper
 * accepts a script path directly and creates a p3em_t context.
 */

int p3em_init(void **handle, const char *script) {
    (void)script; // script argument kept for ABI compatibility, ignored
    if (!handle) return -1;
    p3em_t *ctx = malloc(sizeof(p3em_t));
    if (!ctx) return -1;
    // Let the regular C init logic handle env‑vars and script selection
    if (p3em_init_ctx(ctx) != 0) {
        free(ctx);
        return -1;
    }
    *handle = ctx;
    return 0;
}

int p3em_getLatestValue(void *handle) {
    // Return wall‑clock seconds when the context is disabled
    return (int) p3em_now((p3em_t*)handle);
}

void p3em_cleanup(void *handle) {
    if (!handle) return;
    p3em_cleanup_ctx((p3em_t*)handle);
    free(handle);
}
