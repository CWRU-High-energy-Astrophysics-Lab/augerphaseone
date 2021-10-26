import subprocess as sp
import random

proc = sp.Popen(['tail','-n','25','AN_T2.txt'],stdout=sp.PIPE)
t2s = proc.stdout.read()
t2list = t2s.split('\n')
t2 = random.sample(t2list[:15],1)[0]
sec = t2.split('.')[0]
micros = t2.split('.')[1]
t3 = "T3,%s,%s,100" %(sec,micros)
sp.Popen(['./topm',t3])
# print "./topm " + t3
