import sys
import os
from subprocess import Popen,call,PIPE
from shutil import copyfile
import time
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import curve_fit 


filenames = list()
for i in range(150):
    copyfile("sample.txt", "sample{}.txt".format(i+1))
    filenames.append("sample{}.txt".format(i+1))


timesv1 = list()
timesv2 = list()
threads = list()

for i in range(1, 150):
	command = ["./pzipv2"]
	command.extend(filenames[:i])
	cmdv1 = ' '.join(command)
	command = ["./pnzip"]
	command.extend(filenames[:i])
	cmdv2 = ' '.join(command)

	p = Popen(cmdv1, shell=True, stdout=PIPE)
	(outputv1,err) = p.communicate()
	exit_status = p.wait()

	if(exit_status==0):
		p = Popen(cmdv2, shell=True, stdout=PIPE)
		(outputv2,err) = p.communicate()
		exit_status = p.wait()

		if(exit_status==0):
			timesv1.append(int(outputv1))
			timesv2.append(int(outputv2))
			threads.append(i)
		
	print(i)


plt.plot(threads,timesv1,color="green")
plt.plot(threads,timesv2)
plt.xlabel("no. of files",fontsize=20)
plt.ylabel("Memory Usage (KB)",fontsize=20)
plt.show()


# for i in filenames:

# 	os.remove(i)
# 	os.remove(i[:-3] + "pzip")