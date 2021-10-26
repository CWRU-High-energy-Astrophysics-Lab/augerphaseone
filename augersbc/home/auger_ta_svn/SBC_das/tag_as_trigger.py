import subprocess as sp
import mpmath as mpm
import time
import math

secsInWeek = 604800 
secsInDay = 86400 
gpsEpoch = (1980, 1, 6, 0, 0, 0)  # (year, month, day, hh, mm, ss)

def gpsFromUTC(year, month, day, hour, minute, sec, leapSecs=17): 
	"""converts UTC to GPS second

	Original function can be found at: http://software.ligo.org/docs/glue/frames.html

	GPS time is basically measured in (atomic) seconds since  
	January 6, 1980, 00:00:00.0  (the GPS Epoch) 

	The GPS week starts on Saturday midnight (Sunday morning), and runs 
	for 604800 seconds.  

	Currently, GPS time is 17 seconds ahead of UTC
	While GPS SVs transmit this difference and the date when another leap 
	second takes effect, the use of leap seconds cannot be predicted.  This 
	routine is precise until the next leap second is introduced and has to be 
	updated after that.

	SOW = Seconds of Week 
	SOD = Seconds of Day 

	Note:  Python represents time in integer seconds, fractions are lost!!! 
	""" 
	secFract = sec % 1 
	epochTuple = gpsEpoch + (-1, -1, 0) 
	t0 = time.mktime(epochTuple) 
	t = time.mktime((year, month, day, hour, minute, sec, -1, -1, 0))  
	# Note: time.mktime strictly works in localtime and to yield UTC, it should be 
	#       corrected with time.timezone 
	#       However, since we use the difference, this correction is unnecessary. 
	# Warning:  trouble if daylight savings flag is set to -1 or 1 !!! 
	t = t + leapSecs
	tdiff = t - t0
	gpsSOW = (tdiff % secsInWeek)  + secFract
	gpsWeek = int(math.floor(tdiff/secsInWeek))
	gpsDay = int(math.floor(gpsSOW/secsInDay))
	gpsSOD = (gpsSOW % secsInDay)
	gps_tuple = (gpsWeek, gpsSOW, gpsDay, gpsSOD)
	return int(gps_tuple[0] * secsInWeek + gps_tuple[1])

# Set decimal precision for mpm
mpm.mp.dps = 22 # This means mpm numbers are 22*3.33 bit, precise
								# enough for comparison of 17 digit floats

AS_T2_FILE = "T2_PARSED.out"
TA_GLOBAL_FILE = "TA_GLOBAL.txt"
CNC_FILE = "TAG_AS_COINCIDENCE.txt"

# Main loop
t3_num = 0
p1 = sp.Popen(['tail','-f',TA_GLOBAL_FILE],stdout=sp.PIPE)
t3_mark = 0.
while True:
	tag_str = p1.stdout.readline().replace('\n','')
	p2 = sp.Popen(['tail','-n','50',AS_T2_FILE],stdout=sp.PIPE)
	as_t2_lst = p2.stdout.read(-1).split('\n')[:-1]
	# Check global trigger for coincidence
	if "## " in tag_str:
		tag_str_list = tag_str.split(' ')
		yr_mo_day = tag_str_list[1]
		yr = "20" + yr_mo_day[:2]
		yr = int(yr)
		mo = int(yr_mo_day[2:4])
		day = int(yr_mo_day[4:])
		hr_min_sec = tag_str_list[2]
		hr = int(hr_min_sec[:2])
		minu = int(hr_min_sec[2:4])
		sec = int(hr_min_sec[4:])
		gps_sec = gpsFromUTC(yr,mo,day,hr,minu,sec)
		# Second value is offset from TA SD time stamp
		# First find second value over 10 minute period reported in TAG
		sec1 = (minu % 10) * 60
		# Add reported TAG seconds
		sec1 += sec
		# Find counter value reported by TA SD
		sd_counter = int(tag_str_list[3])
		offset = sec1-sd_counter
		if offset > 0:
			gps_sec -= offset
		else:
			gps_sec += offset
		gps_micro = tag_str_list[4]
		tag_str2 = str(gps_sec) + "." + gps_micro
	else:
		continue
	x1 = mpm.mpf(tag_str2)
	for asl_str in as_t2_lst:
		x2 = mpm.mpf(asl_str)
		diff = abs(float(x1-x2))
		if diff < 100e-6 and time.time() > t3_mark + 150.:
			#print diff
			#print tal_str + " " + asl_str
			with open(CNC_FILE,'a') as F:
				F.write("%s %s %.8f\n" %(tag_str,asl_str,diff))
			south_sec = asl_str.split('.')[0]
			south_micro = asl_str.split('.')[1]
			# Send T3 to south
			sp.Popen(['./cl','T','%s' %south_sec,'%s' %south_micro,'20'])
			t3_mark = time.time()
		else:
			continue
