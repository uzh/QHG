#!/usr/bin/python

from sys import  argv, exit
import h5py 
import numpy as np
import netCDF4 as nc

import grid_utils

# variable names as given in the nc file
LON_NAME  = 'XAXIS'
LAT_NAME  = 'YAXIS'
TIME_NAME = 'TAXI'
ICE_NAME  = 'IM'

# our ice is binary: 100m is the threshold
ICE_THRESH = 100

#-----------------------------------------------------------------------------
#--
#--
class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class


#-----------------------------------------------------------------------------
#--
#--
class IceGanoInterpolator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, npp_file):
        self.npp_file = npp_file
        try:
            ncdata = nc.Dataset(npp_file, 'r')
            self.longs = np.array(ncdata.variables[LON_NAME]) - 360
            self.lats  = np.array(ncdata.variables[LAT_NAME])
            self.ice   = np.array(ncdata.variables[ICE_NAME])
            self.times = np.array(ncdata.variables[TIME_NAME])
            
            ncdata.close()
        except Exception,e:
            raise QHGError("couldn't open %s and extract data. Exception: %s" % (npp_file, e))
        #-- end try
    #--end def


    #-------------------------------------------------------------------------
    #-- findTimeIndex
    #--   time values in ganopolski's data are in ky b.p,
    #--   as are ours - no 'inverting' necessary
    #--
    def findTimeIndex(self, t0):
        index = -1
        d     = -1
        for i in range(self.times.size):
            d1 = abs(t0 - self.times[i])
            if (d < 0) or (d1 < d):
                d = d1
                index = i
            #-- end if
        #-- end for
        return index
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- doInterpolation
    #--   do bivariate interpolations for npp at given time
    #--   onto the grid defined by qLon and qLat
    #--
    def doInterpolation(self, target_time, qLon, qLat):
        index = self.findTimeIndex(target_time)
        if (index >= 0):
            print("[ice]Index for %f : %d" % (target_time, index))
            ice_temp = np.where( self.ice[index] < 0, -1, self.ice[index])

            print("[ice]temp: %d values, shape:%s, first %s, min %f, max %f" % (ice_temp.size, str(ice_temp.shape), str(ice_temp[0][0]), ice_temp.min(), ice_temp.max()))
            ice_new =  grid_utils.doGridInterpolation(qLon, qLat, self.longs, self.lats, ice_temp, np.float64, False)
                        
            print("[ice]int: %d values, shape:%s, min %f, max %f" % (ice_new.size, str(ice_new.shape), ice_new.min(), ice_new.max()))
        #-- end if
        return np.int32(np.where(ice_new > ICE_THRESH, np.int32(1), np.int32(0)))
    #-- end def

#-- end class


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 3):
        try:
            t = float(argv[2])
        except:
            print("time [%s] must be a float" % argv[2])
        else:
            try:
                igi = IceGanoInterpolator(argv[1])
            except QHGError,e:
                print("Constructor failed: %s" % e)
            else:
                qLon, qLat = grid_utils.extractQDFCoordinates(argv[3])
                ice1 = igi.doInterpolation(t, qLon, qLat)
                ice2 = np.where(ice1 < 0, 0, ice1)
                print("success: %d values (%s), min: %f, max %f" % (ice2.size, str(ice2.shape), ice2.min(), ice2.max()))
                # real: grid_utils.replaceQDFFileArray(argv[3], 'Vegetation', 'NPP', ice2)
                #grid_utils.replaceQDFFileArray(argv[3], 'Climate', 'AnnualRainfall', ice2)
            #-- end try
            
            
    else:
        print("usage")
        print("  %s <ncfile> <time> <qdf_geo>" % argv[0])
    #-- end if
#-- end main
