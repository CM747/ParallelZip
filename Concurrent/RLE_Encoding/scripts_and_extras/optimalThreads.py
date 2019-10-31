import sys
import os
from subprocess import Popen,call,PIPE
from shutil import copyfile
import time
import matplotlib.pyplot as plt
import numpy as np

filenames = list()
for i in range(500):
    copyfile("sample.txt", "sample{}.txt".format(i+1))
    filenames.append("sample{}.txt".format(i+1))


times = list()
threads = list()

for i in range(1, 10):
	command = [sys.argv[1], str(i)]
	command.extend(filenames)
	cmd = ' '.join(command)

	# call(command)

	p = Popen(cmd, shell=True, stdout=PIPE)
	(output,err) = p.communicate()
	exit_status = p.wait()

	if(exit_status==0):
		times.append(int(output))
		threads.append(i)
	print(i)

fig = plt.figure()
ax = fig.add_subplot(111)

line, = ax.plot(threads, times)

ymix = min(times)
xpos = times.index(ymix)
xmix = threads[xpos]

ax.annotate('local mix', xy=(xmix, ymix), xytext=(-xmix, -ymix-5),
            arrowprops=dict(facecolor='black', shrink=5),
            )
plt.show()
