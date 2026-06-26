#!/bin/bash
#   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
#   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
#   Copyright(C) 2025 Ivan Pribec, Leibniz-Rechenzentrum
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
#  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
#  language governing permissions and limitations under the License.

CC=${CC:-mpicc}
$CC -Wall -Wextra -Wno-unused-function -I . -pthread test.c my_timer.c -o test
