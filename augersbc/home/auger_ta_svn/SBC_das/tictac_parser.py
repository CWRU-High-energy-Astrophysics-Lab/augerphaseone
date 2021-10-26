import os
import time
import math
import subprocess as sp

def utcFromString(x):
  split_x = x.split(',')
  int_x = map(int,split_x)
  right_order = [2,1,0,3,4,5]
  int_x = [int_x[i] for i in right_order]
  return int_x

secsInWeek = 604800 
secsInDay = 86400 
gpsEpoch = (1980, 1, 6, 0, 0, 0)  # (year, month, day, hh, mm, ss)

tictac_file = 'TA_SD_AUG_4_18_00.txt'
timestamp_file = 'TA_LOCAL.txt'

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

N = 749995494 # Precision oscillator frequency

def computeMicro(n1,n2):
  if n2>n1:
    micsec = float(n2-n1) / N
    micsec_str = "%.6f" %micsec
    return micsec_str.split('.')[1]
  else:
    micsec = float(N-n1+n2) / N
    micsec_str = "%.6f" %micsec
    return micsec_str.split('.')[1]

def write_timestamp(x,i):
  x3 = x[i] #ith entry is TESTevent
  x3 = int(x3.split(' ')[-1]) #Format to int
  #Get GPSevent indices
  gpsevt = [j for j, s in enumerate(x) if "GPSevent" in s]
  #Get UTC time indices
  utctime = [j for j, s in enumerate(x) if ",2016," in s]
  gps_index = gpsevt[-1]
  utc_index = utctime[-1]
  if gps_index > utc_index:
    x2 = x[gps_index]
    x2 = int(x2.split(' ')[-1]) #This entry will be GPS 1 PPS count
    x1 = x[utc_index]
    utc_list = utcFromString(x1) #Format string as ymdhmmss list
    gps_sec = gpsFromUTC(*utc_list) #Find GPS second from list
    gps_sec += 1 #Since this PPS is for next time string
    Tmicro = computeMicro(x3,x2)
  else:
    x2 = x[utc_index]
    utc_list = utcFromString(x2)
    gps_sec = gpsFromUTC(*utc_list)
    x1 = x[gps_index]
    x1 = int(x1.split(' ')[-1])
    Tmicro = computeMicro(x1,x3)
  with open(timestamp_file,'a') as F:
    F.write("%i.%s\n" %(gps_sec,Tmicro))
  return None

# Main loop
tail_old = ""
while True:
  tail_call = sp.Popen(['tail','-n','8',tictac_file],stdout=sp.PIPE,stderr=sp.PIPE)
  tail_new, err = tail_call.communicate()
  if tail_new != tail_old and "TEST" in tail_new:
    tail_list = tail_new.split('\n')
    #First find number of TESTevents
    test_ind = [j for j, s in enumerate(tail_list) if "TEST" in s]
    if len(test_ind) < 2:
      write_timestamp(tail_list,test_ind[0])
    # More than one TESTevent, needs to be handled
    else:
      for m in test_ind:
        write_timestamp(tail_list,m)
    tail_old = tail_new
  else:
    time.sleep(0.500)
