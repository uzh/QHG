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
    lon = ncdata.variables['longitude']
    lat = ncdata.variables['latitude']
    rain = ncdata.variables['precip_mm_srf']
    temp = ncdata.variables['temp_mm_srf']
    snow = ncdata.variables['snowdepth_mm_srf']
except:
    print("ERROR: netCDF data not found")
    exit()


# take care of periodicity

rain = np.array(rain[0,0])  # eliminate 2 spurious dimensions 
rain = rain * 31557600  # convert kg/m^2/s to kg/m^2/y = mm/y
# now replicate grid
rain = np.hstack( [rain, rain] )

temp = np.array(temp[0,0])  # eliminate 2 spurious dimensions 
temp = temp - 273.15  # convert K to Celsius
# now replicate grid
temp = np.hstack( [temp, temp] )

snow = np.array(snow[0,0])  # eliminate 2 spurious dimensions 
# now replicate grid
snow = np.hstack( [snow, snow] )


# prepare coordinates
lon = np.array(lon)
lat = -np.array(lat)  # redefine latitude so that it grows, otherwise the spline complains

# add points for periodic

lon = np.hstack( [lon, lon+360] )


# compute interpolation coefficients
rain_rbs = RectBivariateSpline(lat, lon, rain)
temp_rbs = RectBivariateSpline(lat, lon, temp)


# open the QDF

qdf = h5py.File(argv[2],'r+')

try: 
    geogroup = qdf["/Geography"]
except:
    print("ERROR: Geography group not found in " + argv[2])
    exit()

try:
    climate = qdf.create_group("/Climate")
except:
    print("WARNING: Climate group already exists in " + argv[2])
    del qdf["/Climate"]
    climate = qdf.create_group("/Climate")
    
# Climate group attributes
climate.attrs['NumCells'] = geogroup.attrs['NumCells']
nCells = geogroup.attrs['NumCells'][0]
npzero = np.zeros(1, dtype='int8')
climate.attrs.create("Dynamic", npzero, npzero.shape, dtype='int8')
climate.attrs.create("NumSeasons", npzero, npzero.shape, dtype='int8')

# Climate datasets

qLon = geogroup['Longitude'][:]
qLat = geogroup['Latitude'][:]

AMT = np.zeros(nCells, dtype='float64')  # AnnualMeanTemp
ARF = np.zeros(nCells, dtype='float64')  # AnnualRainfall

for icell in range(nCells):

    # let's shift the grid in order to avoid problems around lon=0
    qLon[icell] = 360 + qLon[icell]
    
    ARF[icell] = np.float64(rain_rbs.ev(-qLat[icell], qLon[icell]))
    AMT[icell] = np.float64(temp_rbs.ev(-qLat[icell], qLon[icell]))
    
    

ACT = AMT  # ActualTemps; this only copies the reference, not the array
ACR = ARF  # ActualRains; this only copies the reference, not the array

CURSEASON = np.zeros(1, dtype='int32')

climate.create_dataset("AnnualMeanTemp", AMT.shape, AMT.dtype, AMT)
climate.create_dataset("AnnualRainfall", ARF.shape, ARF.dtype, ARF)
climate.create_dataset("ActualTemps", ACT.shape, ACT.dtype, ACT)
climate.create_dataset("ActualRains", ACR.shape, ACR.dtype, ACR)
climate.create_dataset("CurSeason", CURSEASON.shape, CURSEASON.dtype, CURSEASON)

qdf.flush()
qdf.close()

ncdata.close()


