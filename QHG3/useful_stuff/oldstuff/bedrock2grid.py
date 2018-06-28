#!/usr/bin/python

from sys import argv, exit
import numpy as np
from scipy.interpolate import RectBivariateSpline
import netCDF4 as nc
import h5py


if len(argv) != 3:
    print("usage: " + argv[0] + " <netCDF> <QDF grid> ")
    exit()


ncdata = nc.Dataset(argv[1], 'r')

try:
    lon = ncdata.variables['x']
    lat = ncdata.variables['y']
    alt = ncdata.variables['z']
except:
    print("ERROR: netCDF data not found")
    exit()



lon = np.array(lon)
lat = np.array(lat)  
alt = np.array(alt)


# compute interpolation coefficients
alt_rbs = RectBivariateSpline(lat, lon, alt)


# open the QDF

qdf = h5py.File(argv[2],'r+')

try: 
    geogroup = qdf["/Geography"]
except:
    print("ERROR: Geography group not found in " + argv[2])
    exit()


try:
    altdset = geogroup['Altitude']
    del geogroup['Altitude']
except:
    print("Altitude not found, will create")

qLon = geogroup['Longitude'][:]
qLat = geogroup['Latitude'][:]
nCells = geogroup.attrs['NumCells']

#MAP = np.zeros(nCells, dtype='float64') 

#for icell in range(nCells):

#    MAP[icell] = np.float64(alt_rbs.ev(qLat[icell], qLon[icell]))
MAP = np.float64(alt_rbs.ev(qLat, qLon))

    
geogroup.create_dataset("Altitude", MAP.shape, MAP.dtype, MAP)

qdf.flush()
qdf.close()

ncdata.close()


