#!/bin/bash

#- hwmon device number (default: all)
x=("${@:-*}")
MPATH=()
for val in "${x[@]}"; do
  MPATH+=("/sys/class/hwmon/hwmon${val}/device/power1_instant")
done

#- Time duration
SEC=${2:-0.1}

#- Read in loop and process with awk
while true; do
  cat ${MPATH[@]}
  sleep $SEC
done | awk -v sec=$SEC '{sum += ($1*sec/1.0E6); print sum}'
