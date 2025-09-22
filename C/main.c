#include <stdio.h>
#include <sys/wait.h>
#include "p3em.h"

static p3em_t global_monitor;

int main() {
  if (p3em_init(&global_monitor, "../p3em.sh") != 0) {
    printf("Failed to initialize p3em\n");
    return 1;
  }
  // Simulate usage
  for (int i = 0; i < 1000; ++i) {
    printf("Latest value: %d\n", p3em_getLatestValue(&global_monitor));
    usleep(30000);
  }

  p3em_cleanup(&global_monitor);
  return 0;
}
