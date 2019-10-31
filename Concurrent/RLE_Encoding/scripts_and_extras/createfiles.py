import sys
import os
from subprocess import call
from shutil import copyfile

filenames = list()
for i in range(int(sys.argv[1])):
    copyfile("sample.txt", "sample{}.txt".format(i+1))
    filenames.append("sample{}.txt".format(i+1))

# command = [sys.argv[2]]
# command.extend(filenames)

# call(command)
