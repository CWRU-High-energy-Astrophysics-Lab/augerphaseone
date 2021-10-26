# Copyright (c) Case Western Reserve University 2015
# This software is distributed under Apache License 2.0
# Consult the file LICENSE.txt
# Author: Sean Quinn spq@case.edu
# June 14 2015

import binascii
import bz2
import struct
import os
import matplotlib.pyplot as plt
import numpy as np
import subprocess

"""Small Python3 script that parses the master T3.out list. The file contains 
all the relevant T3 data: Event ID, GPS time, Trigger type, and FADC traces for the
3 PMTs. Ricardo Sato has written a program "x2" which recovers the trace data
only from a decompressed T3 message. This script performs the following function
1.) Isolates individual events in the master list
2.) Converts ASCII hex data to raw binary
2.) Decompresses (bz2 format) T3 data message (still raw binary)
3.) Creates a folder for individual events with following name format
    GPSTIME_MICROSECOND
    Example: 1117640005_616863
4.) Places output of x2 program, which is an ASCII text file containing the
    PMT traces (presumably mV levels for dynode/anode?)
"""

def save_calib(bytes):
	f = open('calib_info.txt','w')
	calsize = struct.unpack('>I',bytes[8212:8216])[0]
	if calsize == 0:
		calsize = struct.unpack('>I',bytes[8216:8220])[0]
		cal_vals = ['cal_size','ver','tube_mask','gps_start','gps_end',
						'NbT1','NbT2','Evolution','Base','SigmaBase','VemPeak','Rate',
						'NbTDA','DA','SigmaDA','VemCharge','NbTOT']
		f.write('Version {}\n'.format(struct.unpack('>H',bytes[8220:8222])[0]))
		f.write('TubeMask {}\n'.format(struct.unpack('>H',bytes[8222:8224])[0]))
		f.write('StartSecond {}\n'.format(struct.unpack('>I',bytes[8224:8228])[0]))
		f.write('EndSecond {}\n'.format(struct.unpack('>I',bytes[8228:8232])[0]))
		f.write('NbT1 {}\n'.format(struct.unpack('>H',bytes[8232:8234])[0]))
		f.write('NbT2 {}\n'.format(struct.unpack('>H',bytes[8234:8236])[0]))
		evol = [0,0,0]
		for i in range(3):
			evol[i] = struct.unpack('>H',bytes[8236+2*i:8236+2*(i+1)])[0]
		f.write('Evolution {0} {1} {2}\n'.format(*evol))
		#Finish at 8242
		base = [0,0,0,0,0,0]
		for i in range(6):
			base[i] = struct.unpack('>H',bytes[8242+2*i:8242+2*(i+1)])[0]
		f.write('Base {0} {1} {2} {3} {4} {5}\n'.format(*base))
		#Finish at 8254
		sig_base = [0,0,0,0,0,0]
		for i in range(6):
			sig_base[i] = struct.unpack('>H',bytes[8254+2*i:8254+2*(i+1)])[0]
		f.write('SigmaBase {0} {1} {2} {3} {4} {5}\n'.format(*sig_base))
		#Finish at 8266
		block = ['VemPeak','Rate','NbTDA','DA','SigmaDA','VemCharge']
		for i in range(6):
			vals = [0,0,0]
			for j in range(3):
				vals[j]=struct.unpack('>H',bytes[8266+2*(3*i+j):8266+2*(3*i+j+1)])[0]
			f.write(block[i]+' '+'{0} {1} {2}\n'.format(*vals))
		#Finish at 8302
		f.write('NbTOT {}\n'.format(struct.unpack('>H',bytes[8302:8304])[0]))
		if calsize == 104:
			block = ['DADt','SigmaDADt','DAChi2']
			for i in range(3):
				vals = [0,0,0]
				for j in range(3):
					vals[j]=struct.unpack('>H',bytes[8266+2*(3*i+j):8266+2*(3*i+j+1)])[0]
			f.write(block[i]+' '+'{0} {1} {2}\n'.format(*vals))
	else:
		f.write("BAD COMPRESS\n")
		f.close()
	f.close()
	return calsize

def save_mon(bytes,si):
	#si is fondly known as start index
	si += 8220
	mon_size = struct.unpack('>I',bytes[si:si+4])[0]
	si += 4
	if mon_size == 6080:
		f = open('mon_hist_offset.txt','w')
		for i in range(10):
			f.write('{}\n'.format(struct.unpack('>H',bytes[si+2*i:si+2*(i+1)])[0]))
		si += 2 * 10
		f.close()
		f = open('mon_hist_base.txt','w')
		for i in range(20):
			vals = [0,0,0]
			for j in range(3):
				vals[j]=struct.unpack('>H',bytes[si+2*(3*i+j):si+2*(3*i+j+1)])[0]
			f.write('{0} {1} {2}\n'.format(*vals))
		f.close()
		si += 2 * 20 * 3
		f = open('mon_hist_peak.txt','w')
		for i in range(150):
			vals = [0,0,0]
			for j in range(3):
				vals[j]=struct.unpack('>H',bytes[si+2*(3*i+j):si+2*(3*i+j+1)])[0]
			f.write('{0} {1} {2}\n'.format(*vals))
		f.close()
		si += 2 * 150 * 3
		f = open('mon_hist_charge.txt','w')
		for i in range(600):
			vals = [0,0,0,0]
			for j in range(4):
				vals[j]=struct.unpack('>H',bytes[si+2*(4*i+j):si+2*(4*i+j+1)])[0]
			f.write('{0} {1} {2} {3}\n'.format(*vals))
		f.close()
		si += 2 * 600 * 4
		f = open('mon_hist_shape.txt','w')
		for i in range(20):
			vals = [0,0,0]
			for j in range(3):
				vals[j]=struct.unpack('>I',bytes[si+4*(3*i+j):si+4*(3*i+j+1)])[0]
			f.write('{0} {1} {2}\n'.format(*vals))
		f.close()
		si += 4 * 20 * 3
	elif mon_size == 0:
		return 0
	else:
		f = open('BAD_MON_COMPRESS','w')
		f.close()
		return mon_size
	return si

def save_gps(bytes,si):
	gpssize = struct.unpack('>I',bytes[si:si+4])[0]
	si += 4
	block = ['Current100','Next100','Current40','Next40','PreviousST',
				'CurrentST','NextST']
	f = open('gps_info.txt','w')
	for i in range(7):
		val = struct.unpack('>I',bytes[si+4*i:si+4*(i+1)])[0]
		f.write(block[i]+' '+'{}\n'.format(val))
	si += 4*7
	if gpssize == 30:
		val = struct.unpack('>H',bytes[si:si+2])[0]
		f.write('Offset {}'.format(val))
	f.close()

def make_plots():
	mon_peak = np.loadtxt('mon_hist_peak.txt')
	for i in range(3):
		plt.plot(mon_peak[:,i],drawstyle='steps',label='PMT {}'.format(i+1))
	plt.legend()
	plt.savefig('mon_hist_peak.pdf')
	plt.clf()
	mon_charge = np.loadtxt('mon_hist_charge.txt')
	for i in range(4):
		plt.plot(mon_charge[:,i],drawstyle='steps',label='PMT(?) {}'.format(i+1))
	plt.legend()
	plt.savefig('mon_hist_charge.pdf')
	plt.clf()
	mon_shape = np.loadtxt('mon_hist_shape.txt')
	for i in range(3):
		plt.plot(mon_shape[:,i],drawstyle='steps',label='PMT {}'.format(i+1))
	plt.legend()
	plt.savefig('mon_hist_shape.pdf')
	plt.clf()
	fadc_hist = np.loadtxt('FADC_trace')
	xaxis = np.linspace(0,12.8,768)
	for i in range(3):
		plt.plot(xaxis,fadc_hist[:,i+1],
						drawstyle='steps',label='PMT {}'.format(i+1))
	plt.xlabel(r'time after trigger ($\mu$s)')
	plt.ylabel('ADC counts')
	plt.title('T1TH Trigger ADC Traces')
	plt.legend()
	plt.savefig('th_adc_trace.pdf')
	plt.clf()
	for i in range(3):
		plt.plot(xaxis,fadc_hist[:,i+4],
					drawstyle='steps',label='PMT {}'.format(i+1))
	plt.xlabel(r'time after trigger ($\mu$s)')
	plt.ylabel('ADC counts')
	plt.title('T1ToT Trigger ADC Traces')
	plt.legend()
	plt.savefig('tot_adc_trace.pdf')
	plt.clf()

#Read through T3 file collecting events into a large list
#Since writing the original script I've found a more elegant approach
#thanks to inspectorG4dget at stackoverflow
#http://stackoverflow.com/questions/18865058/

t3list = []
num_evts = 0
print("Reading T3 event file ...\t\t", end='')

with open('T3.out','r') as t3file:
	copy = False
	for line in t3file:
		if line.strip() == "Event ...":
			copy = True
			data_str = ''
			num_evts += 1
		elif line.strip() == "----------":
			copy = False
			#Avoid clipped data streams. Compressed event should be more than 10kB
			if len(data_str)>9000:
				t3list.append(data_str)
		elif copy:
			data_str += line.strip().replace(' ','')

print("[ OK ]")
print("Found {} events".format(num_evts))
   
for t3 in t3list:
	evt_id = int(t3[:4],16)
	error_code = int(t3[4:6],16) #Value of 1 indicates no error for T3
	packed=binascii.unhexlify(bytes(t3[8:],'ascii'))    
	dec_t3 = bz2.decompress(packed)
	# Now that we have uncompressed message let's get some information
	# The PowerPC hardware uses big endian format
	gps_YMDHMnS = struct.unpack('>I', dec_t3[:4])[0] #First 4 bytes are GPS sec
	gps_MICRO = struct.unpack('>I', dec_t3[4:8])[0] #Next 4B are GPS microsec
	try:
		os.mkdir("{0}_{1}_{2}".format(evt_id,gps_YMDHMnS,gps_MICRO))
	except OSError as e:
		#Catch folder already exists error
		if e.errno==17:
			print("This event has already been unpacked and saved, skipping ...")
			continue
	os.chdir('{0}_{1}_{2}'.format(evt_id,gps_YMDHMnS,gps_MICRO))
	f=open('T3_{}.bin'.format(evt_id), 'bw')
	f.write(dec_t3)
	f.close()   
	subprocess.call(["../x2", "T3_{}.bin".format(evt_id)])
	monstart = save_calib(dec_t3)
	gpsstart = save_mon(dec_t3,monstart)
	save_gps(dec_t3,gpsstart)
	make_plots()
	os.chdir('..')
