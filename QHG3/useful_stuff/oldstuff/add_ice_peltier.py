#!/usr/bin/python

from sys import argv, exit
import numpy as np
from scipy.interpolate import RectBivariateSpline
import netCDF4 as nc
import h5py


if len(argv) != 4:
    print("usage: " + argv[0] + " <netCDF> <kya> <QDF grid> ")
    exit()


ncdata = nc.Dataset(argv[1], 'r')
icevar = "ICE_" + argv[2]

try:
    lon = ncdata.variables['Lon']
    lat = ncdata.variables['Lat']
    ice = ncdata.variables[icevar]
except:
    print("data not found in netCDF file")
    exit()

# take care of periodicity

ice = np.hstack( [ice, ice[:,1:]] )


# prepare coordinates
lon = np.array(lon)
lat = -np.array(lat)  # redefine latitude so that it grows, otherwise the spline complains

# add points for periodic

lon = np.hstack( [lon, lon[1:]+360] )

# compute interpolation coefficients
ice_rbs = RectBivariateSpline(lat, lon, ice, kx=1, ky=1)


# open the QDF

qdf = h5py.File(argv[3],'r+')

try: 
    geogroup = qdf["/Geography"]
except:
    print("ERROR: Geography group not found in " + argv[3])
    exit()


try:
    del geogroup['IceCover']
except:
    print("WARNING: creating new IceCover in " + argv[3])
    

qLon = geogroup['Longitude'][:]
qLat = geogroup['Latitude'][:]
nCells = geogroup.attrs['NumCells'][0]

ICE = np.zeros(nCells, dtype='int32')  

for cell in range(nCells):

    # let's shift the grid in order to avoid problems around lon=0
    qLon[cell] = 360 + qLon[cell]
    
    local_ice = np.int32(ice_rbs.ev(-qLat[cell], qLon[cell]))
    if local_ice > 100:
        ICE[cell] = np.int32(1)
    else:
        ICE[cell] = np.int32(0)

    
geogroup.create_dataset("IceCover", ICE.shape, ICE.dtype, ICE)

qdf.flush()
qdf.close()

ncdata.close()


