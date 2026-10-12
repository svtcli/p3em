#!/bin/bash

#- Time resolution (try it)
SEC=${1:-0.1}
MSEC=$(echo $SEC | awk '{print $1 * 1000}')

#- Nvidia GPUs via nvidia-smi
nvidia-smi -lms $MSEC --query-gpu=power.draw --format=csv,nounits,noheader | awk -v t=$SEC '{sum+=$1+0; printf "%d\n", sum*t}'

#- Alternative syntax
#nvidia-smi -lms $MSEC -q -d POWER | awk -v sec=$SEC  '/Inst/ {if(NR%35==29){s+=$5*sec;  print s}}'

