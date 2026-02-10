#!/bin/bash

#- hwmon device number (default: all)
x=${1:-"*"}
MPATH=/sys/class/hwmon/hwmon${x}/device/power1_instant

#- Time duration
SEC=${2:-0.1}

#- Read in loop and process with awk
while true; do
  cat $MPATH
  sleep $SEC
done | awk -v sec=$SEC '{sum += ($1*sec/1.0E6); print sum}'
