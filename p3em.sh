#!/bin/bash
#   Copyright(C) 2025 Salvatore Cielo, LRZ
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
#  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
#  language governing permissions and limitations under the License.

SEC=${1:-0.01}
MSEC=$(echo "scale=0; 1000 * $SEC /1 " | bc -l)

#while true ; do date "+%s" ; sleep $SEC ; done # TEST

#perf stat -a -e power/energy-pkg/ --log-fd 1 -Sr 0  sleep $SEC | awk '/Jou/ {gsub(/,/, ".", $0); sum += $1+0; print sum}'

unbuffer xpu-smi dump -m 8 --ims $MSEC --file /dev/stdout 2>/dev/null | awk '{s += $3+0} (NR+2) % 4 == 0 {printf "%d\n", s; s = 0}'

#nvidia-smi -lms $MSEC --query-gpu=power.draw --format=csv,nounits,noheader | awk -v t=$SEC '{sum+=$1+0; printf "%d\n" sum/t}'

#- AMD is not great in general as it doesn't have a daemon mode.
#-- AMD w rocm-smi, a bit faster: latency is about 0.5 sec
#Z=$(rocm-smi --showenergycounter --csv | awk -F"," '{sum+=$3+0}END{printf "%d\n",sum/1E6}');
#while true; do rocm-smi --showenergycounter --csv | awk -F"," -v z=$Z '{sum+=($3+0)}END{printf "%d\n",sum/1E6-z+5000}' ; done

#-- AMD w amd-smi, a bit simpler but slower: about 0.7 sec
#Z=$(amd-smi metric -E --csv | awk -F"," '{sum+=$2+0}END{printf "%d\n",sum}');
#while true; do amd-smi metric -E --csv | awk -F"," -v z=$Z '{sum+=($2+0)}END{printf "%d\n",sum-z+4000}' ; done
