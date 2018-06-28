#!/usr/bin/python

from sys import  argv, exit
import h5py 
import numpy as np
import netCDF4 as nc

import grid_utils

LON_NAME  = 'XAXLEVITR'
LAT_NAME  = 'YAXLEVITR'
TIME_NAME = 'TAXI'
NPP_NAME  = 'NPPHRS'

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
class NPPInterpolator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, npp_file):
        self.npp_file = npp_file
        try:
            ncdata = nc.Dataset(npp_file, 'r')
            self.longs = np.array(ncdata.variables[LON_NAME])
            self.lats  = np.array(ncdata.variables[LAT_NAME])
            self.npps  = np.array(ncdata.variables[NPP_NAME])
            self.times = np.array(ncdata.variables[TIME_NAME])
            
            ncdata.close()
        except Exception,e:
            raise QHGError("couldn't open %s and extract data. Exception: %s" % (npp_file, e))
        #-- end try
    #--end def


    #-------------------------------------------------------------------------
    #-- findTimeIndex
    #--     time values in timmermann's data go from -407 to -1,
    #--     and we use ky b.p., therefore we have to 'invert' the time 
    #--
    def findTimeIndex(self, t0):
        index = -1
        d     = -1

        t0    = max(self.times) - t0 
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
            print("[npp]Index for %f : %d" % (target_time, index))
            print("[npp]qLat ("+str(qLat.shape)+") : %f %f  %f %f %f" %(qLat[0],qLat[1],qLat[2],qLat[3],qLat[4]))
            npp_temp = np.where( self.npps[index] < 0, 0, self.npps[index])
            #print("temp: %d values, min: %f, max %f" % (npp_temp.size, min(npp_temp), max(npp_temp)))
            print("[npp]temp: %d values, shape:%s, first %s, min %f, max %f" % (npp_temp.size, str(npp_temp.shape), str(npp_temp[0][0]), npp_temp.min(), npp_temp.max()))
            npp_new =  grid_utils.doGridInterpolation(qLon, qLat, self.longs, self.lats, npp_temp, np.float64, False)
            #print("int: %d values, shape:%s, first %s, min %f, max %f" % (npp_new.size, str(npp_new.shape), str(npp_new[0][0]), npp_new.min(), npp_new.max()))
            print("[npp]int: %d values, shape:%s, min %f, max %f" % (npp_new.size, str(npp_new.shape), npp_new.min(), npp_new.max()))
        #-- end if
        return npp_new
    #-- end def

#-- end classq


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
                ni = NPPInterpolator(argv[1])
            except QHGError,e:
                print("Constructor failed: %s" % e)
            else:
                qLon, qLat = grid_utils.extractQDFCoordinates(argv[3])
                npp1 = ni.doInterpolation(t, qLon, qLat)
                npp2 = np.where(npp1 < 0, 0, npp1)
                print("success: %d values (%s), min: %f, max %f" % (npp2.size, str(npp2.shape), npp2.min(), npp2.max()))
                grid_utils.replaceQDFFileArray(argv[3], 'Vegetation', 'NPP', npp2)
                #grid_utils.replaceQDFFileArray(argv[3], 'Climate', 'AnnualRainfall', npp2)
            #-- end try
            
            
    else:
        print("usage")
        print("  %s <ncfile> <time>" % argv[0])
    #-- end if
#-- end main
