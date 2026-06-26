#include "my_timer.h"
#include "p3em.h"
#include <stdio.h>
#include <stdlib.h>

void my_timer_init(my_timer *t) {
    if (t->initialized) return;
    t->initialized = 1;
    if (p3em_init_ctx(&t->p3em) != 0) {
        fprintf(stderr, "[my_timer] p3em init failed\n");
        exit(EXIT_FAILURE);
    }
}

void my_timer_cleanup(my_timer *t) {
    if (!t->initialized) return;
    p3em_cleanup_ctx(&t->p3em);
    t->initialized = 0;
}

void my_timer_start(my_timer *t) {
    t->startSec = p3em_now(&t->p3em);
}

double my_timer_elapsed(const my_timer *t) {
    return p3em_now(&t->p3em) - t->startSec;
}
