import math
import time
import subprocess as sp

def utcFromString(x):
  split_x = x.split(',') # Break up position/date message
  split_x = split_x[0:6] # Only need d,m,y,h,min,s. Ignore northing, easting, alt
  int_x = map(int,split_x)
  right_order = [2,1,0,3,4,5]
  int_x = [int_x[i] for i in right_order]
  return int_x

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

N = 749995494 # Precision oscillator frequency

def computeMicro(n1,n2):
  if n2>n1:
    micsec = float(n2-n1) / N
    micsec_str = "%.6f" %micsec
    return int(micsec_str.split('.')[1])
  else:
    micsec = float(2**32-n1+n2) / N
    micsec_str = "%.6f" %micsec
    return int(micsec_str.split('.')[1])

dev_id = ''
p = sp.Popen(['stdbuf','-oL','cat',dev_id],stdout=sp.PIPE)
# Main loop
int_list = [0] * 150 # Micros buffer. Should only need ~70, but 
                     # allocate more just in case
ii = 0
gps_n = 0
gps_sec = 0
while True:
  msg = p.stdout.readline()
  if "GPS" in msg:
    if ii > 0:
      for jj in range(ii):
        print "%i.%06d" %(gps_sec,int_list[jj])
      ii = 0
    gps_n = int(msg.split(' ')[-1])
    continue
  elif ",2016," in msg:
    utc_list = utcFromString(msg)
    gps_sec = gpsFromUTC(*utc_list)
    continue
  elif "TEST" in msg:
    microint = int(msg.split(' ')[-1])
    microsec = computeMicro(gps_n,microint)
    int_list[ii] = microsec
    ii += 1
    continue
  else:
    continue