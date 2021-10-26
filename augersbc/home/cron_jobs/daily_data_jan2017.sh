#!/bin/sh
#Small script that compresses daily data files
#Jan 11 2017
#Sean Quinn
#spq@case.edu

cd /home/pi/data/new_sbc

# This runs at 00:00:00 UTC, use -1 day to write the correct name
datestr=$(date +%Y_%m_%d -d "-1 day")
t2file=$datestr.T2
sulogfile=$datestr.sulog
#Save raw T2 time stamps in 24 hour period to new file
cp T2_list.out $t2file
#Move new file to archive folder
mv $t2file south/t2/
#Cleanup
> T2_list.out
> T2_PARSED.out

# Save 24 hour cdas_su_emu log to new file
cp su_log.txt $sulogfile
mv $sulogfile south/sulog/
> su_log.txt

# Save 24 hour T3 events to new file
t3file=$datestr.T3
cp T3.out $t3file
mv $t3file south/t3/
> T3.out

# Save 24 hour TA global trigger to new file
taglobalfile=$datestr.TAG
cp TA_GLOBAL.txt $taglobalfile
mv $taglobalfile ta/
> TA_GLOBAL.txt

# Save 24 hour TA local trigger to new file
talocalfile=$datestr.TAL
cp TA_LOCAL.txt $talocalfile
mv $talocalfile ta/
> TA_LOCAL.txt

# Save 24 hour TAG+AS+AN coincidence file
cglob=$datestr.CTAG
cp TAG_AS_COINCIDENCE.txt $cglob
mv $cglob coincidence/
> TAG_AS_COINCIDENCE.txt

# Save 24 hour TAL+AS+AN coincidence file
cloc=$datestr.CTAL
cp TAL_AS_COINCIDENCE.txt $cloc
mv $cloc coincidence/
> TAL_AS_COINCIDENCE.txt

#Compress files
#---
#South
gzip south/t2/$t2file
gzip south/sulog/$sulogfile
gzip south/t3/$t3file
#TA
gzip ta/$taglobalfile
gzip ta/$talocalfile
#Coincidence
gzip coincidence/$cglob
gzip coincidence/$cloc
#---
