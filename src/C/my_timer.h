#ifndef MY_TIMER_H
#define MY_TIMER_H

#include "p3em.h"
#include <stdbool.h>

typedef struct {
    int initialized;
    double startSec;
    p3em_t p3em;
} my_timer;

void my_timer_init(my_timer *t);
void my_timer_cleanup(my_timer *t);
void my_timer_start(my_timer *t);
double my_timer_elapsed(const my_timer *t);

#endif
