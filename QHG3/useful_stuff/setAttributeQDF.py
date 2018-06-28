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
    del p.attrs['ClassID']
except:
    pass
classID = np.array([124], dtype='int32')
p.attrs.create('ClassID', classID, classID.shape, classID.dtype)

try:
    del p.attrs['ClassName']
except:
    pass
className = np.array(['VegDwellerPop'], dtype='|S13')
p.attrs.create('ClassName', className, className.shape, className.dtype)

dt = h5py.special_dtype(vlen='float64')

try:
    del p.attrs['GrassCapPref']
except:
    pass
grass_x = np.array([10, 1000, 10000], dtype='float64')
grass_y = np.array([7.5, 750, 750], dtype='float64')
grass_slp = np.array([(grass_y[i+1]-grass_y[i])/(grass_x[i+1]-grass_x[i]) for i in range(len(grass_y)-1) ], dtype='float64')
grass = np.array([grass_y, grass_x, grass_slp])
p.attrs.create('GrassCapPref', grass, grass.shape, dtype=dt)

try:
    del p.attrs['GrassWeight']
except:
    pass
grass_w = np.array([0.45], dtype='float64')
p.attrs.create('GrassWeight', grass_w, grass_w.shape, grass_w.dtype)

try:
    del p.attrs['ShrubCapPref']
except:
    pass
shrub_x = np.array([10, 1000, 10000], dtype='float64')
shrub_y = np.array([7.5, 750, 750], dtype='float64')
shrub_slp = np.array([(shrub_y[i+1]-shrub_y[i])/(shrub_x[i+1]-shrub_x[i]) for i in range(len(shrub_y)-1) ], dtype='float64')
shrub = np.array([shrub_y, shrub_x, shrub_slp])
p.attrs.create('ShrubCapPref', shrub, shrub.shape, dtype=dt)

try:
    del p.attrs['ShrubWeight']
except:
    pass
shrub_w = np.array([0.45], dtype='float64')
p.attrs.create('ShrubWeight', shrub_w, shrub_w.shape, shrub_w.dtype)

try:
    del p.attrs['TreeCapPref']
except:
    pass
tree_x = np.array([10, 1000, 10000], dtype='float64')
tree_y = np.array([7.5, 750, 750], dtype='float64')
tree_slp = np.array([(tree_y[i+1]-tree_y[i])/(tree_x[i+1]-tree_x[i]) for i in range(len(tree_y)-1) ], dtype='float64')
tree = np.array([tree_y, tree_x, tree_slp])
p.attrs.create('TreeCapPref', tree, tree.shape, dtype=dt)

try: 
    del p.attrs['TreeWeight']
except:
    pass
tree_w = np.array([0.1], dtype='float64')
p.attrs.create('TreeWeight', tree_w, tree_w.shape, tree_w.dtype)

try:
    del p.attrs['AltMovePref']
except:
    pass
amp_alt = np.array([-0.1, 0., 2500.], dtype='float64')
amp_val = np.array([0., 1., 0.], dtype='float64')
amp_slope = np.array([ (amp_val[i+1]-amp_val[i])/(amp_alt[i+1]-amp_alt[i]) for i in range(len(amp_val)-1) ], dtype='float64')
amp = np.array([ amp_alt, amp_val, amp_slope ])
p.attrs.create('AltMovePref', amp, amp.shape, dtype=dt)

f.flush()
f.close()

