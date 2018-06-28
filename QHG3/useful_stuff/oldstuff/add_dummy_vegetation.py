#!/usr/bin/python

from sys import argv, exit
import numpy as np
import h5py


if len(argv) != 2:
    print("usage: " + argv[0] + " <QDF> ")
    exit()


qdf = h5py.File(argv[1],'r+')


geogroup = qdf["/Geography"]

try:
    veg = qdf.create_group("/Vegetation")
except:
    print("WARNING: Vegetation group already exists in " + argv[1] + "... replacing")
    del qdf["/Vegetation"]
    veg = qdf.create_group("/Vegetation")
    
# Climate group attributes
veg.attrs['NumCells'] = geogroup.attrs['NumCells']
nCells = geogroup.attrs['NumCells'][0]

dyn = np.int8([1, 1, 1])
veg.attrs.create("Dynamic", dyn, dyn.shape, dtype='int8')

spc = np.int32([3])
veg.attrs.create("NumSpecies", spc, spc.shape, dtype='int32')

MASS = np.zeros((nCells, 3), dtype='float64')
NPP = np.zeros((nCells, 3), dtype='float64')

# Climate datasets

veg.create_dataset("Mass", MASS.shape, MASS.dtype, MASS)
veg.create_dataset("NPP", NPP.shape, NPP.dtype, NPP)

qdf.flush()
qdf.close()


