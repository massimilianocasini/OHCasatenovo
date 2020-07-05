#!/bin/bash
wert=`cat /sys/bus/w1/devices/$1/w1_slave | tail -n1 | cut -d '=' -f2`
wert2=`echo "scale=3; $wert/1000" | bc`
echo $wert2
