#!/bin/sh
#Small script that compresses daily data files
#August 8 2016
#Sean Quinn
#spq@case.edu

cd /home/pi/data2

# This runs at 00:00:00 UTC, use -1 day to write the correct name
datestr=$(date +%Y_%m_%d -d "-1 day")
t2file=$datestr.T2
sulogfile=$datestr.sulog
#Save T2 time stamps in 24 hour period to new file
cat T2_list.out > $t2file
#Compress file
gzip $t2file
#Cleanup
> T2_list.out
> T2_PARSED.out
mv $t2file.gz south/t2/

cat su_log.txt > $sulogfile
gzip $sulogfile
> su_log.txt
mv $sulogfile.gz south/su_log/

t3file=$datestr.T3
cat T3.out > $t3file
gzip $t3file
> T3.out
mv $t3file.gz south/t3/

# Save 24 hour TA global trigger to new file
taglobalfile=$datestr.TAG
kill $(cat tag_pid)
cat TA_GLOBAL.txt > $taglobalfile
cat /dev/ttyUSB0 > TA_GLOBAL.txt &
echo $! > tag_pid
gzip $taglobalfile
mv $taglobalfile.gz ta/

# Save 24 hour TA local trigger to new file
talocalfile=$datestr.TAL
cat TA_LOCAL_NEW.txt > $talocalfile
gzip $talocalfile
> TA_LOCAL_NEW.txt
mv $talocalfile.gz ta/
