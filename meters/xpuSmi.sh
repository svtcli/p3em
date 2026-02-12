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

##- Get GPUs per node
GPN=$(( 1 + $(xpu-smi discovery --dump 1 2>/dev/null | tail -n 1 )))

##- Zero it (optional)
Z=$(unbuffer xpu-smi dump -m 8 -n 1 --file /dev/stdout 2>/dev/null | awk -v gpn=$GPN '(NR>1){sum+=$3+0}END{print sum}')

unbuffer xpu-smi dump -m 8 --ims $MSEC --file /dev/stdout 2>/dev/null | \
  awk -v gpn=$GPN -v z=$Z '{s+=$3+0}((NR+2)%gpn)==0 {printf "%d\n", s-z; s=0}'
