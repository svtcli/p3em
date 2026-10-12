#!/bin/bash
#
# List and reads once all the available hwmon devices.
# Use it to know what's available, then pass the desired
# device to average.sh or instant.sh.
#
for x in {0..1200}; do # 1200 is just a very large number
  MPATH=/sys/class/hwmon/hwmon${x}/device/
  if [ -f $MPATH/power1_oem_info ]; then
    if [ -f $MPATH/power1_average ]; then
      echo -n Device_${x}_Average: $(cat $MPATH/power1_oem_info | awk '{gsub(/ /,"_",$0); print $0}')
      echo " " $(cat $MPATH/power1_average  | awk '{print ($1/1.0E6)}')W
    fi
    if [ -f $MPATH/power1_inst* ]; then
      echo -n Device_${x}_Instant: $(cat $MPATH/power1_oem_info | awk '{gsub(/ /,"_",$0); print $0}')
      echo " " $(cat $MPATH/power1_instant  | awk '{print ($1/1.0E6)}')W
    fi
  fi
done
