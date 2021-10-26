import math
import time
import subprocess as sp

T2_FILE = 'T2_list.out'
T2_OUTFILE = 'T2_PARSED.out'

p = sp.Popen(['tail','-f',T2_FILE],stdout=sp.PIPE)
# Main loop
micro_list = []
gps_sec = ""
while True:
  line = p.stdout.readline()
  if "Sec,nt2,scaler:" in line:
    # Get GPS second
    gps_sec = line.split(' ')[1]
    micro_list = []
  elif " 1:" in line or " 9:" in line and len(gps_sec) > 0:
    line = line.replace('\n','')
    micro_list.append(line.split(':')[1])
  elif "--" in line and len(gps_sec) > 0:
    # End of T2 list, write results
    with open(T2_OUTFILE,'a') as F:
      for member in micro_list:
        F.write("%s.%06d\n" %(gps_sec,int(member)))
    gps_sec = ""
  else:
  # Unknown message, skip
    continue
