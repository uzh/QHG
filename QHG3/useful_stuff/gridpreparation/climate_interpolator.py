#!/usr/bin/python

from sys import argv, exit
import numpy as np
from scipy.interpolate import interp1d, RectBivariateSpline
import os
import re
import netCDF4 as nc

import grid_utils

# variable names for BRIDGE climate files
NAME_TEMP = "temp_mm_srf"
NAME_RAIN = "precip_mm_srf"
NAME_LON  = "longitude"
NAME_LAT  = "latitude"
NAME_T    = "t"
NAME_SRF  = "surface"

#-----------------------------------------------------------------------------
#--
#--
class ClimateInterpolator:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, ncdir, ncpat):
        self.ncdir = ncdir
        self.ncpat = ncpat
        self.bPronto = self.prepareInterpolators()
        
    #--end def


    #-----------------------------------------------------------------------------
    #-- doInterpolation
    #--   do bivariate interpolations for temp and rain
    #--   onto the grid defined by qLon and qLat
    #--
    def doInterpolation(self, target_time, qLon, qLat):
        temp_tmp, rain_tmp = self.doTimeInterpolation(target_time)
        if not temp_tmp is None:
            temp_new  = grid_utils.doGridInterpolation(qLon, qLat, self.longs, self.lats, temp_tmp, np.float64, True)
            rain_new  = grid_utils.doGridInterpolation(qLon, qLat, self.longs, self.lats, rain_tmp, np.float64, True)
        #-- end if
        return temp_new, rain_new
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- doTimeInterpolation
    #--   create interpolators for rain and temp
    #--   create interpolations for all time in new_time
    #--   write to nc-files named according to outputpat
    #--   (the strin "###" in outputpat is replaced by the current time value)
    #--
    def doTimeInterpolation(self, target_time):
        temp_cur = np.array([])
        rain_cur = np.array([])
        # print("target-times: "+str(target_time))
        if self.bPronto:
            temp_cur = self.interp_temp(target_time)
            rain_cur = self.interp_rain(target_time)
        else:
            print("ClimateINterpolator not initialized")
        #-- end if
        return temp_cur, rain_cur
    #-- end def


    #-----------------------------------------------------------------------------
    #-- prepareInterpolators
    #--   create interpolators for rain and temp
    #--
    def prepareInterpolators(self):
        self.bPronto = False
        self.collectNCFiles() # sets self.ncfiles
        dshape  = ()
        if len(self.ncfiles) > 0:
            self.interp_temp = self.createInterpolator(NAME_TEMP, dshape,        1.0,-273.16, False) # convert Kelvin to Celsius
            self.interp_rain = self.createInterpolator(NAME_RAIN, dshape, 31557600.0,   0.0,  True)  # 31557600: convert kg/m^2/s to kg/m^2/y = mm/y
            
            if (not self.interp_temp is None) and (not self.interp_rain is None):
                cur_file = next (iter (self.ncfiles.values()))
                self.getCoords(cur_file)  # sets sekf.longs and self.lats
            
                if (not self.longs is None) and (not self.lats is None):
                    self.bPronto = True
                else:
                    print("Couldn't extract longitude/latitude from ["+cur_file+"]")
                #-- end if
            
            else:
                print("Failed to create interpolators");
            #-- end if
        else:
            print("No files found in ["+ncdir+"] matching ["+ncpat+"]")
            #-- end if
        return self.bPronto
    #-- end def


    #-----------------------------------------------------------------------------
    #-- createInterpolator
    #--
    #-- Creates an interpolator for the data set <arrname> for all files listed
    #-- in the dictionary ncfiles (<time> -> <filename>).
    #-- Only arrays having shape <dshape> are used.
    #-- When <dshape> is empty, it value will be set to the shape of the first
    #-- extracted array.
    #-- It returns a interpolator
    #--
    def createInterpolator(self, arrname, dshape, factor, offset, clamp):
        interp_cur = None
        curarr  = np.array([])
        iResult = 0
        for x in sorted(self.ncfiles):
            ncdata = nc.Dataset(self.ncdir+'/'+self.ncfiles[x], 'r')
                
            iResult, curarr, dshape = self.collectArray(curarr, ncdata, arrname, dshape, factor, offset, clamp)
            # curarr = factor*curarr + offset    
            if (iResult != 0):
                break
            #-- end if
           
        #-- end for
            
        if iResult == 0:
        
            curarr = curarr.reshape(len(self.ncfiles), dshape[0], dshape[1])

            # found no better way to turn keys into an array
            keys=[x for x in self.ncfiles.keys()]
            times = np.array(keys)
            interp_cur = interp1d(times, curarr, kind='cubic', axis=0)
        #-- end if
        return interp_cur
    #-- end def


    #-----------------------------------------------------------------------------
    #-- collectNCFiles
    #--
    #-- collect files in ncdir matching the regular expression ncpat
    #-- and return in a dictonary <time> => <filename>
    #--
    def collectNCFiles(self):
        files = os.listdir(self.ncdir)
        rr    = re.compile(self.ncpat)
        self.ncfiles = {}
        try:
            for f in files:
                m = rr.match(f)
                if not m is None:
                    t = m.group(1)
                    self.ncfiles[int(t)] = f
                #-- end if
            #-- end for
            if len(self.ncfiles) == 0:
                print("No matches found for pattern " + ncpat)
            #-- end if
        except:
            print("Matched pattern ["+t+"] is not numeric")
            self.ncfiles.clear()
        #-- end try
        if (len(self.ncfiles) < 4):
            print("Found " + str(len(ncfiles)) + ", but need at least 4!")
            self.ncfiles.clear()
        else:
            print("have "+str(len(self.ncfiles))+" files")
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- collectArray
    #--
    #-- extract dataset with name <arrname> from N file <ncdata> and
    #-- concatenate it with <arrcum>, if its shape is equal to <dshape>.
    #-- At the first call <arrcum> is set to the new data, and <dshape>
    #-- is set to its shape.
    #-- It returns
    #--    iResult -  0 if OK, -1 on error
    #--    arrcum2 -  the concatenation of arrcum with the new array
    #--    dshape  -  the shape of the new array
    #--
    def collectArray(self, arrcum, ncdata, arrname, dshape, factor, offset, clamp):
        iResult = -1
        arrcum2 = np.array([])
        try:
            arrtemp = ncdata.variables[arrname] # 'temp_mm_srf']
            arrtemp = np.array(arrtemp[0,0])
            arrtemp = arrtemp*factor+offset

            if (arrcum.size == 0):
                dshape  = arrtemp.shape
                arrcum2  = arrtemp
                iResult = 0
            else:
                if dshape == arrtemp.shape:
                    arrcum2 = np.concatenate((arrcum, arrtemp))
                    iResult = 0
                else:        
                    print(ncfiles[x]+"['"+arrname+"']: expected array size "+iNumVals+" but got "+arrtemp.size)
                #-- end if
            #--  end if
        except:
            print("Exception during array collect\n")
        #-- end try
        return iResult, arrcum2, dshape
    #-- end def                              



    #-----------------------------------------------------------------------------
    #-- getCoords
    #--  extracts longitude and latitude arrays from the specified file
    #--
    def getCoords(self, curfile):
        iResult = -1;
        self.longs = np.array([])
        self.lats  = np.array([])
        dshape  = ()

        try:
            ncdata = nc.Dataset(self.ncdir+'/'+curfile, 'r')
    
            self.longs  = np.array(ncdata.variables[NAME_LON])
            self.lats   = np.array(ncdata.variables[NAME_LAT])
        except:
            self.longs  = None
            self.lats   = None
            print("Couldn't extract longitude/latitude from ["+self.ncdir+'/'+curfile+"]")
        #-- end try
    #-- end def


    #-----------------------------------------------------------------------------
    #-- createNCDF
    #--   create a rudimentary NetCDF file containing only longitude, latitude,
    #--   temperature and rain in the same way the BRIDGE nc files do
    #--
    def createNCDF(self, temps, rains, output):
        # open a new NetCDF file
        rootgrp = nc.Dataset(output, "w", format="NETCDF4")

        # define dimensions
        t_dim   =  rootgrp.createDimension(NAME_T, 1)
        srf_dim =  rootgrp.createDimension(NAME_SRF, 1)
        lon_dim =  rootgrp.createDimension(NAME_LON, len(self.longs))
        lat_dim =  rootgrp.createDimension(NAME_LAT, len(self.lats))

        # define varaiables
        t_var    = rootgrp.createVariable(NAME_T,    "f4", (NAME_T))
        srf_var  = rootgrp.createVariable(NAME_SRF,  "f4", (NAME_SRF))
        long_var = rootgrp.createVariable(NAME_LON,  "f4", (NAME_LON))
        lat_var  = rootgrp.createVariable(NAME_LAT,  "f4", (NAME_LAT))
        temp_var = rootgrp.createVariable(NAME_TEMP, "f4", (NAME_T, NAME_SRF, NAME_LAT, NAME_LON))
        rain_var = rootgrp.createVariable(NAME_RAIN, "f4", (NAME_T, NAME_SRF, NAME_LAT, NAME_LON))

        # fill variable
        t_var[:]   = 0
        srf_var[:] = 0
        long_var[:] = self.longs
        lat_var[:]  = self.lats
        temp_var[:] = temps
        rain_var[:] = rains

        # close the file
        rootgrp.close()
        
    #-- end def

    


