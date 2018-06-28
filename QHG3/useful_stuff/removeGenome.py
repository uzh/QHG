#!/usr/bin/python

from sys import argv, exit
import h5py
import numpy as np

if len(argv) != 2:
    print("error")
    exit()

f = h5py.File(argv[1],'r+')

p = f['/Populations/Sapiens']
try:
    del p['Genome']
except:
    pass

f.flush()
f.close()

