#!/bin/sh

# South data
rsync -avh --bwlimit=256 --log-file="/home/pi/cron_jobs/rsync/$(date "+%Y.%m.%d-%H.%M.%S")" -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data2/south/ augerta@192.168.254.223:/home/augerta/disk/cosmo
# TA data
rsync -avh --bwlimit=256 --log-file="/home/pi/cron_jobs/rsync/$(date "+%Y.%m.%d-%H.%M.%S")" -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data2/ta/ augerta@192.168.254.223:/home/augerta/disk/ta
