#!/usr/bin/python

from sys import exit
import numpy as np
from scipy.interpolate import interp1d, RectBivariateSpline
import os
import re
import netCDF4 as nc

import grid_utils

ICE_THRESH = 100

#-----------------------------------------------------------------------------
#--
#--
class IceInterpolator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, ice_dir, ice_pat):
        self.ice_dir = ice_dir
        self.ice_pat = ice_pat
        self.collectIceFiles() # sets self.filelist
        self.times = self.getTimes()
        self.tkeys = sorted(list(self.times))
        print("Range: ["+str(self.tkeys[0])+", "+str(self.tkeys[-1])+"]")
        self.getCoordinates() # creates self.lon, self.lat arrays
        self.vLo = np.array([])
        self.vHi = np.array([])
        self.tLo = -1
        self.tHi = -1
    #--end def


    #-------------------------------------------------------------------------
    #-- doInterpolation
    #--
    def doInterpolation(self, target_time, qLon, qLat):
        ice_new = None
        ice_tmp = self.doTimeInterpolation(target_time)
        ice_new=grid_utils.doGridInterpolation(qLon, qLat, self.lon, self.lat, ice_tmp, np.int32, True)
        
        # binarize
        ice_new = np.int32(np.where(ice_new > ICE_THRESH, np.int32(1), np.int32(0)))

        return ice_new
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doTimeInterpolation
    #--
    def doTimeInterpolation(self, target_time):
        ice_new = None

        self.getBounds(target_time)
        if (self.tLo >= 0):

            #print("Bounds: "+str(self.tLo)+", "+str(self.tHi))
            self.vLo = np.array(self.extractData(self.tLo))
            self.vHi = np.array(self.extractData(self.tHi))
            x_range = [self.tLo, self.tHi]
            v_range = [self.vLo, self.vHi]
            interp_cur = interp1d(x_range, v_range, kind='linear', axis=0)
            ice_new = interp_cur(target_time)
        else:
            print("Value "+str(target_time)+" out of range ["+str(tkeys[0])+", "+str(tkeys[-1])+"]")
        #-- end if
        return np.array(ice_new)
    #-- end def


    #-----------------------------------------------------------------------------
    #-- getCoordinates
    #--   collect the lon and lat arrays for later use
    #--
    def getCoordinates(self):
        iResult = -1
        # we assume all files have same longitudes/latitudes
        f0 = self.filelist[0]
        try:
            ncdata = nc.Dataset(f0, 'r')
            self.lon = np.array(ncdata.variables['Lon'])
            self.lat = np.array(ncdata.variables['Lat'])
            ncdata.close()
            iResult = 0;
        except:
            print("Couldn't open ["+f0+"], or extract Lon/Lat data")
        #-- end try
        return iResult
    #-- end def


    #-----------------------------------------------------------------------------
    #-- getTimes
    #--   collect the time for the variables in the files
    #--   we expect the variable names to have the form ICE_XXX
    #--   where XXX is a number (k.b.p.)
    #--
    def getTimes(self):
        floatexp = '[0-9]*[\.0-9][0-9]*'
        rr=re.compile("ICE_("+floatexp+")")
        times = {}
        
        for f in self.filelist:
            print("checking ["+f+"]")
            rootgrp = nc.Dataset(f, 'r')
            
            vkeys=[x for x in rootgrp.variables.keys()]

            for t in vkeys:
                m = rr.match(t)
                if (not m is None):
                    e = m.groups()
                    times[float(e[0])] = (e[0], f)
                #-- end if
            #-- end for
        #-- end for
        return times
    #-- end def
    


    #-----------------------------------------------------------------------------
    #-- getBounds
    #--   find the times
    def getBounds(self, target_time):
        i = 0

        self.tHi = -1
        self.tLo = -1

        if (target_time >= self.tkeys[0]) and (target_time < self.tkeys[-1]):
            
            while (i < len(self.tkeys)) and (self.tkeys[i] < target_time) :
                i += 1
            #-- end while
    
            if (i > 0) :    
                self.tHi = self.tkeys[i]
                self.tLo = self.tkeys[i-1]
            #-- end if
        #-- end if
    #-- end def    


    #-----------------------------------------------------------------------------
    #-- collectIceFiles
    #--   collect all files in directory ice_dir matching the pattern ice_pat
    #-- 
    def collectIceFiles(self):
        self.filelist = []
        r_ice= re.compile(self.ice_pat)
        for f in os.listdir(self.ice_dir):
            m = r_ice.match(f)
            if m != None:
                self.filelist.append(self.ice_dir+"/"+m.string)
            #-- end if
        #-- end for
    #-- end def

    
    #-----------------------------------------------------------------------------
    #-- extractData
    #--   extract ice data - we assume the variabl names stat with "ICE_"
    #-- 
    def extractData(self, tCur):
        # print("getting 'ICE_"+self.times[tCur][0]+"' from "+self.times[tCur][1])
        rootgrp = nc.Dataset(self.times[tCur][1], 'r')
        return rootgrp.variables['ICE_'+self.times[tCur][0]]
    #-- end def

#-- end class
