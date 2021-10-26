#!/bin/sh

cat /dev/ttyUSB0 >> TA_GLOBAL.txt &
echo $! > tag_pid
