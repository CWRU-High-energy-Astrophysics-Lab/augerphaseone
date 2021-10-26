#!/bin/sh

# South data
rsync -ah -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data/new_sbc/south/ augta@129.22.134.40:/home/augta/data/south/
# North data
rsync -ah -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data/new_sbc/north/t2/ augta@129.22.134.40:/home/augta/data/north/t2/
rsync -ah -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data/new_sbc/north/t3/ augta@129.22.134.40:/home/augta/data/north/t3/
# TA data
rsync -ah -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data/new_sbc/ta/ augta@129.22.134.40:/home/augta/data/ta/
# Coincidence data
rsync -ah -e "ssh -i /home/pi/.ssh/id_rsa" /home/pi/data/new_sbc/coincidence/ augta@129.22.134.40:/home/augta/data/coincidence/
