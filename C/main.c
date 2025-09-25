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

#include <stdio.h>
#include <sys/wait.h>
#include "p3em.h"

int main() {

  p3em_t *global_monitor;

  if (p3em_init(&global_monitor, "../p3em.sh") != 0) {
    printf("Failed to initialize p3em\n");
    return 1;
  }
  // Simulate usage
  for (int i = 0; i < 10; ++i) {
    printf("Latest value: %d\n", p3em_getLatestValue(global_monitor));
    usleep(30000);
  }

  p3em_cleanup(global_monitor);
  return 0;
}
