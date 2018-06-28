#!/usr/bin/python

from sys import argv, exit
import numpy as np
from scipy.interpolate import interp1d
import os
import re
import netCDF4 as nc

# variable names for BRIDGE climate files
NAME_TEMP = "temp_mm_srf"
NAME_RAIN = "precip_mm_srf"
NAME_LON  = "longitude"
NAME_LAT  = "latitude"
NAME_T    = "t"
NAME_SRF  = "surface"


#-----------------------------------------------------------------------------
#-- collectNCFiles
#--
#-- collect files in ncdir matching the regular expression ncpat
#-- and return in a dictonary <time> => <filename>
#--
def collectNCFiles(ncdir, ncpat):
    files = os.listdir(ncdir)
    rr    = re.compile(ncpat)
    ncfiles = {}
    try:
        for f in files:
            m = rr.match(f)
            if m != None:
                t = m.group(1)
                ncfiles[int(t)] = f
            #-- end if
        #-- end for
        if len(ncfiles) == 0:
            print("No matches found for pattern " + ncpat)
        #-- end if
    except:
        print("Matched pattern ["+t+"] is not numeric")
        ncfiles.clear()
    #-- end try
    if (len(ncfiles) < 4):
        print("Found " + str(len(ncfiles)) + ", but need at least 4!")
        ncfiles.clear()
    else:
        print("have "+str(len(ncfiles))+" files")
    #-- end if
    return ncfiles
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
def collectArray(arrcum, ncdata, arrname, dshape):
    iResult = -1
    arrcum2 = np.array([])
    try:
        arrtemp = ncdata.variables[arrname] # 'temp_mm_srf']
        arrtemp = np.array(arrtemp[0,0])
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
#-- createInterpolator
#--
#-- Creates an interpolator for the data set <arrname> for all files listed
#-- in the dictionary ncfiles (<time> -> <filename>).
#-- Only arrays having shape <dshape> are used.
#-- When <dshape> is empty, it value will be set to the shape of the first
#-- extracted array.
#-- It returns a interpolator
#--
def createInterpolator(ncfiles, arrname, dshape):
    interp_cur = None
    curarr  = np.array([])
    iResult = 0
    for x in sorted(ncfiles):
        ncdata = nc.Dataset(ncdir+'/'+ncfiles[x], 'r')
                
        iResult, curarr, dshape = collectArray(curarr, ncdata, arrname, dshape)
                
        if (iResult != 0):
            break
        #-- end if
           
    #-- end for
            
    if iResult == 0:
        
        curarr = curarr.reshape(len(ncfiles), dshape[0], dshape[1])

        # found no better way to turn keys into an array
        keys=[x for x in ncfiles.keys()]
        times = np.array(keys)
        #print(str(times))
        #print(str(curarr))
        interp_cur = interp1d(times, curarr, kind='cubic', axis=0)
    #-- end if
    return interp_cur
#-- end def


#-----------------------------------------------------------------------------
#-- getCoords
#--  extracts longitude and latitude arrays from the specified file
#--
def getCoords(ncdir, ncfile):
    iResult = -1;
    longs = np.array([])
    lats  = np.array([])
    dshape  = ()

    try:
        ncdata = nc.Dataset(ncdir+'/'+ncfile, 'r')
    
        longs = ncdata.variables[NAME_LON]
        lats  = ncdata.variables[NAME_LAT]
    except:
        longs = None
        lats  = None
        print("Couldn't extract longitude/latitude")
    #-- end try
    return longs, lats
#-- end def


#-----------------------------------------------------------------------------
#-- createNCDF
#--   create a rudimentary NetCDF file containing only longitude, latitude,
#--   temperature and rain in the same way the BRIDGE nc files do
#--
def createNCDF(temps, rains, longs, lats, output):
    # open a new NetCDF file
    rootgrp = nc.Dataset(output, "w", format="NETCDF4")

    # define dimensions
    t_dim   =  rootgrp.createDimension(NAME_T, 1)
    srf_dim =  rootgrp.createDimension(NAME_SRF, 1)
    lon_dim =  rootgrp.createDimension(NAME_LON, len(longs))
    lat_dim =  rootgrp.createDimension(NAME_LAT, len(lats))

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
    long_var[:] = longs
    lat_var[:]  = lats
    temp_var[:] = temps
    rain_var[:] = rains

    # close the file
    rootgrp.close()

#-- end def

     
#-----------------------------------------------------------------------------
#-- interpolateClimate
#--   create interpolators for rain and temp
#--   create interpolations for all time in new_time
#--   write to nc-files named according to outputpat
#--   (the strin "###" in outputpat is replaced by the current time value)
#--
def interpolateClimate(ncdir, ncpat, new_times, outputpat):
    ncfiles = collectNCFiles(ncdir, ncpat)
    dshape  = ()
    if len(ncfiles) > 0:
        interp_temp = createInterpolator(ncfiles, NAME_TEMP, dshape)
        interp_rain = createInterpolator(ncfiles, NAME_RAIN, dshape)

        if (interp_temp != None) and (interp_rain != None):
            first_file = next (iter (ncfiles.values()))
            longitude, latitude = getCoords(ncdir, first_file)
            
            if (longitude != None) and (latitude != None):
                #print(str(longitude))
                #print(str(latitude))
                for t in new_times:
                    temp_new = interp_temp(t)
                    rain_new = interp_rain(t)

                    sname=outputpat.replace("###", str(t))
                    createNCDF(temp_new, rain_new, longitude, latitude, sname)
                    print("Wrote ["+sname+"]")
                    
                #-- end for

            else:
                print("Couldn't extract longitude/latitude from ["+first_file+"]")
            #-- end if
            
        else:
            print("Failed to create interpolators");
        #-- end if
    else:
        print("No files found in ["+ncdir+"] matching ["+ncpat+"]")
    #-- end if
#-- end def


#-----------------------------------------------------------------------------
#-- parseTimeDef
#--  Time definition format:
#--    time-def    ::= <time-el> ["," <time-el>]*
#--    time-el     ::= <time-point> | <time-range>
#--    time-point  :   float
#--    time-range  ::= <time-from> ":" <time-to> "@" <time-step>
#--    time-from   :   float
#--    time-to     :   float
#--    time-step   :   float
#--
def parseTimeDef(time_def, prec):
    times = []
    floatexp = '[0-9]*[\.0-9][0-9]*'
    rr     = re.compile('('+floatexp+'):('+floatexp+')@('+floatexp+')$')
    times0 = time_def.split(',')
    for t in times0:
        m = rr.match(t)
        if  m != None:
            #-- it's a time range
            e = m.groups()
            if len(e) == 3:
                try:
                    low  = float(e[0])
                    hi   = float(e[1])
                    step = float(e[2])
                except:
                    print("Ignoring time def ["+t+"] (not numeric)")
                else:
                    if low <= hi:
                        t1 = low
                        while t1 <= hi:
                            times.append(str(t1)) 
                            t1 += step
                        else:
                            print("No numbers in the interval ["+low+", "+hi+"]")
                        #-- end for
                    else:
                        print("Ignoring time def ["+t+"] (hi < low)")
                    #-- end if
                #-- end try
            #-- end if
        else:
            try:
                t1 = float(t)
            except:
                print("Ignoring time def ["+t+"] (not numeric)")
            else:
                times.append(str(t1))
            #-- end try
        #-- end if
    #-- end for
    # remove doubles, sort numerically, format
    return ["%.*f"%(prec, y) for y in sorted([float(x) for x in list(set(times))])]
#-- end def


#-----------------------------------------------------------------------------
#-- usage
#--
def usage(progname):
    print("")
    print(progname + " - interpolating BRIDGE-style NetCDF data")
    print("usage:")
    print("  " + progname + " <nc-dir> <nc-pat> <time-list> <output-pat>")
    print("where:")
    print("  nc-dir      directory in which to search for files matching <nc-pat>")
    print("  nc-pat      regular expression for the files of interest (enclosed in quotes).")
    print("              We assume that the filenames havea uniform structure")
    print("              which contains the time. The wildcards standing for")
    print("              this number must be enclosed in round brackets, so")
    print("              that this value can be extracted.")
    print("              The matching files should be NetCDF files with variables")
    print("              named '"+NAME_TEMP+"' and '"+NAME_RAIN+"'")
    print("  time-list   comma-separated list of time for which to calculate")
    print("              interpolations.")
    print("  output-pat  pattern for the generation of ouput file names.")
    print("              The substring '###' will be replaced by the time value")
    print("")
    print("Example:")
    print("  " + progname + " CLIMATE/BRIDGE/ '(2[0-9]|3[0-4])ka\.ann\.nc' 23,25,30:32@0.5  new_###.nc")
    print("This will create the files")
    print(" 'new_23.0.nc', 'new_25.0.nc', 'new_27.0.nc', 'new_29.0.nc',")
    print(" 'new_30.0.nc', 'new_30.5.nc', 'new_31.0.nc', 'new_31.5.nc', 'new:32.0.nc")
    print("containing interpolations based on the data from the files 'XXka.ann.nc'")
    print("where XX is any integer number between 20 and 34")
    print("")
#-- end def

       

#-----------------------------------------------------------------------------
#-- main
#--
if len(argv) > 4:
    ncdir   = argv[1]
    ncpat   = argv[2]
    times   = parseTimeDef(argv[3],1)
    output  = argv[4]
    print("times:" + str(times))
    interpolateClimate(ncdir, ncpat, times, output)
else:
    usage(argv[0])
#-- end if
