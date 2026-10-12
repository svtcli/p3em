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

#- Time resolution (try it)
SEC=${1:-0.1}
MSEC=$(echo $SEC | awk '{print $1 * 1000}')

#- No. of logical cores per node
CPN=$(lscpu | grep CPU\(s\) | awk '{sum+=$2+0}(NR==1){print sum}')

#- module or other activation
module load likwid > /dev/null

#- EXPERIMENTAL, but likely perfect
unbuffer likwid-perfctr -g PWR_PKG_ENERGY:PWR0 -t ${MSEC}ms | \
  awk -v cpn=$CPN '(NR>7) {for(i=5;i<5+cpn;i++) total+=$i} {print total}'
