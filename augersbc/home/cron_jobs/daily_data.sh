#!/bin/sh
#Small script that compresses daily data files
#October 10 2016
#Sean Quinn
#spq@case.edu

cd /home/pi/data/new_sbc

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

sleep 2s

# Save 24 hour cdas_su_emu log to new file
cat su_log.txt > $sulogfile
gzip $sulogfile
> su_log.txt
mv $sulogfile.gz south/su_log/

sleep 2s

# Save 24 hour T3 events to new file
t3file=$datestr.T3
cat T3.out > $t3file
gzip $t3file
> T3.out
mv $t3file.gz south/t3/

sleep 2s

# Save 24 hour TA global trigger to new file
taglobalfile=$datestr.TAG
cat TA_GLOBAL.txt > $taglobalfile
gzip $taglobalfile
> TA_GLOBAL.txt
mv $taglobalfile.gz ta/

sleep 2s

# Save 24 hour TA local trigger to new file
talocalfile=$datestr.TAL
cat TA_LOCAL.txt > $talocalfile
gzip $talocalfile
> TA_LOCAL.txt
mv $talocalfile.gz ta/

sleep 2s

# Save 24 hour AN T2 to new file
#ant2file=$datestr.T2
#cat north/AN_T2.txt > north/t2/$ant2file
#gzip north/t2/$ant2file
#> north/AN_T2.txt

sleep 2s

# Save 24 hour AN T3 to new archive
#tar -caf north/$datestr.tar.gz north/t3_*
#rm t3_*
#mv north/$datestr.tar.gz north/t3/

sleep 2s

# Save 24 hour TAG+AS+AN coincidence file
fname=$datestr.CTAG
cat TAG_AS_COINCIDENCE.txt > $fname
gzip $fname
mv $fname.gz coincidence
> TAG_AS_COINCIDENCE.txt

sleep 2s

# Save 24 hour TAL+AS+AN coincidence file
fname=$datestr.CTAL
cat TAL_AS_COINCIDENCE.txt > $fname
gzip $fname
mv $fname.gz coincidence
> TAL_AS_COINCIDENCE.txt
