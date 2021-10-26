import subprocess as sp
import mpmath as mpm
import time

# Set decimal precision for mpm
mpm.mp.dps = 22 # This means mpm numbers are 22*3.33 bit, precise
                # enough for comparison of 17 digit floats

#  sp.Popen(['./cl','T','%i' %gps_sec,'%s' %Tmicro,'30'])
#  with open(logfile,'a') as F:
#    F.write("Sending T3 #%i: %i.%s\n" %(j,gps_sec,Tmicro))
#  return None

AS_T2_FILE = "T2_PARSED.out"
TA_LOC_FILE = "TA_LOCAL_NEW_PARSED.txt"
CNC_FILE = "TAL_AS_COINCIDENCE.txt"

# Main loop
t3_num = 0
p1 = sp.Popen(['tail','-f',TA_LOC_FILE],stdout=sp.PIPE)
t3_mark = 0.
while True:
  tal_str = p1.stdout.readline().replace('\n','')
  #print tal_str
  p2 = sp.Popen(['tail','-n','50',AS_T2_FILE],stdout=sp.PIPE)
  as_t2_lst = p2.stdout.read(-1).split('\n')[:-1]
  # Check local trigger for coincidence
  x1 = mpm.mpf(tal_str)
  for asl_str in as_t2_lst:
    x2 = mpm.mpf(asl_str)
    diff = abs(float(x1-x2))
    if diff < 80e-6 and time.time() > t3_mark + 150.:
      #print diff
      #print tal_str + " " + asl_str
      with open(CNC_FILE,'a') as F:
        F.write("%s %s %.6f\n" %(tal_str,asl_str,diff))
      south_sec = asl_str.split('.')[0]
      south_micro = asl_str.split('.')[1]
      # Send T3 to south
      sp.Popen(['./cl','T','%s' %south_sec,'%s' %south_micro,'20'])
      t3_mark = time.time()
    else:
      continue
