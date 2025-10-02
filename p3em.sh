#!/bin/bash
#   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
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
#- GPUs per node (for xpu-smi)
GPN=4
#- logical cores per node (for likwid)
CPN=$(lscpu | grep CPU\(s\) | awk '{sum+=$2+0}(NR==1){print sum}')

#- Debugging test, prints time in seconds
#while true ; do date "+%s" ; sleep $SEC ; done

#- CPU measuring via perf
unbuffer perf stat -a -e power/energy-pkg/ -I $MSEC | awk '{sum+=$2+0; print sum}' # Perhaps add  -S to perf stat?

#- CPU via likwid. EXPERIMENTAL, likely perfect
#unbuffer likwid-perfctr -g PWR_PKG_ENERGY:PWR0 -t ${MSEC}ms | awk -v cpn=$CPN '(NR>7) {for(i=5;i<5+cpn;i++) total+=$i} {print total}'

#- Intel GPUs via xpu-smi
#unbuffer xpu-smi dump -m 8 --ims $MSEC --file /dev/stdout 2>/dev/null | awk -v gpn=$GPN '{s+=$3+0}((NR+2)%gpn)==0 {print s%1E6; s=0}'

#- Nvidia GPUs via nvidia-smi
#nvidia-smi -lms $MSEC --query-gpu=power.draw --format=csv,nounits,noheader | awk -v t=$SEC '{sum+=$1+0; printf "%d\n", sum/t}'

#- AMD smis are not great in general as lack a daemon mode.
#-- AMD w rocm-smi, a bit faster: latency is about 0.5 sec
#Z=$(rocm-smi --showenergycounter --csv | awk -F"," '{sum+=$3+0}END{printf "%d\n",sum/1E6}');
#while true; do rocm-smi --showenergycounter --csv | awk -F"," -v z=$Z '{sum+=($3+0)}END{printf "%d\n",sum/1E6-z+5000}' ; done

#-- AMD w amd-smi, a bit simpler but slower: about 0.7 sec
#Z=$(amd-smi metric -E --csv | awk -F"," '{sum+=$2+0}END{printf "%d\n",sum}');
#while true; do amd-smi metric -E --csv | awk -F"," -v z=$Z '{sum+=($2+0)}END{printf "%d\n",sum-z+4000}' ; done
