#!/bin/bash

#- hwmon device number (default: all)
x=("${@:-*}")
MPATH=()
for val in "${x[@]}"; do
  MPATH+=("/sys/class/hwmon/hwmon${val}/device/power1_average")
done

#- Time duration
SEC=$(cat ${MPATH}_interval | awk '{print $1/1000}')  #| awk '(NR==1){print $1/1000}'

#- Read in loop and process with awk
while true; do
  cat ${MPATH[@]}
  sleep $SEC
done | awk -v sec=$SEC '{sum += ($1*sec/1.0E6); print sum}'
