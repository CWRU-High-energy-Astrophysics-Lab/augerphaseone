import subprocess as sp

# Start pm but force Linux to line buffer the stdout
# (Required for real time T3 decisions)
proc = sp.Popen(['stdbuf','-oL','-i0','./pm'],stdout=sp.PIPE,stdin=sp.PIPE)
# Initialize flags and counters
t2_flag = 0
t2_flag_micros = 0
t2_cntr = 0
num_t2 = 99
# File to save Auger North T2
t2_file = 'AN_T2.txt'
# Main loop
while True:
	line = proc.stdout.readline() # Blocking read of pipe
	if 'NbT2' in line: # 
		splt_str = line.split(',')
		colon_str = splt_str[1].split(':')
		gps_sec = colon_str[1].replace(' ','')
		colon_str = splt_str[0].split(':')
		num_t2 = int(colon_str[1])
		t2_flag = 1 #Indicate a T2 message received, expect microseconds next
		t2_list = [0] * (num_t2 - 1)
		continue
	elif t2_flag == 1 and t2_flag_micros == 0:
		if 'Micros' in line: # List of microseconds follows this message
			t2_flag_micros = 1
			continue
		else:
			continue
	elif t2_flag * t2_flag_micros == 1 and len(line) > 1:
			tm_stmp = gps_sec + '.' + line
			t2_list[t2_cntr] = tm_stmp
			t2_cntr += 1
			# print tm_stmp # Diagnostic printing
			if t2_cntr == num_t2 - 1:
				t2_flag = 0 #End of T2 list
				t2_flag_micros = 0
				t2_cntr = 0
				with open(t2_file,'a') as F: # Append T2 list to file
					for t2 in t2_list:
						F.write(t2)
	else:
		continue
