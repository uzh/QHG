#!/usr/bin/python

from sys import argv, exit
import numpy as np
from scipy.interpolate import interp1d
import os
import re
import netCDF4 as nc


#-----------------------------------------------------------------------------
#-- getTimes
#--   collect the time for the variables in the files
#--   we expect the variable names to have the form ICE_XXX
#--   where XXX is a number (k.b.p.)
#--
def getTimes(filelist):
    floatexp = '[0-9]*[\.0-9][0-9]*'
    rr=re.compile("ICE_("+floatexp+")")
    times = {}

    for f in filelist:
        rootgrp = nc.Dataset(f, 'r')
       
        vkeys=[x for x in rootgrp.variables.keys()]

        for t in vkeys:
            m = rr.match(t)
            if (m != None):
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
def getBounds(times, target_time):
    i = 0

    tHi = -1
    tLo = -1

    if (target_time >= times[0]) and (target_time < times[-1]):

        while (i < len(times)) and (times[i] < target_time) :
            i += 1
        #-- end while

        if (i > 0) :    
            tHi = times[i]
            tLo = times[i-1]
        #-- end if
    #-- end if

    return tLo, tHi
#-- end def    



#-----------------------------------------------------------------------------
#-- extractData
#--   extract ice data - we assume the variabl names stat with "ICE_"
#--
def extractData(times, tCur):
    print("getting 'ICE_"+times[tCur][0]+"' from "+times[tCur][1])
    rootgrp = nc.Dataset(times[tCur][1], 'r')
    return rootgrp.variables['ICE_'+times[tCur][0]]
#-- end def


#-----------------------------------------------------------------------------
#-- usage
#--
def usage(pApp):
    print(pApp+ " - interpolating ice")
    print("Usag;")
    print("  " + pApp + " <target-time> <file-list>")
    print("where")
    print("  taret-time  time for which to do the interpolation")
    print("  file_list   list of files containing ice data")
    print("              (ass: veriable name start with 'ICE_')")
#-- end def

def doInterpolation(target_time, filelist):
    ice_new = None
    times = getTimes(filelist)
    tkeys = sorted(list(times))
    print("Range: ["+str(tkeys[0])+", "+str(tkeys[-1])+"]")

    tLo, tHi = getBounds(tkeys, target_time)
    if (tLo >= 0):

        print("Bounds: "+str(tLo)+", "+str(tHi))
        v1 = np.array(extractData(times, tLo))
        v2 = np.array(extractData(times, tHi))
        x_range = [tLo, tHi]
        v_range = [v1, v2]
        interp_cur = interp1d(x_range, v_range, kind='linear', axis=0)
        ice_new = interp_cur(target_time)

    else:
        print("Value "+str(target_time)+" out of range ["+str(tkeys[0])+", "+str(tkeys[-1])+"]")

    return ice_new
#-- end def
              
#-- main
# if len(argv) > 2:
#     target_time = float(argv[1])
#     filelist = list(argv[2:])
#     print("target time "+str(target_time))
#     print("data files  "+str(filelist))
#     ice_new = interp_cur(target_time)

#     print("----- "+str(tLo)+" -----")
#     print(str(v1))
#     print("----- "+str(target_time)+" -----")
#     print(str(ice_new))
#     print("----- "+str(tHi)+" -----")
#     print(str(v2))

 
#     times = getTimes(filelist)
#     tkeys = sorted(list(times))
#     print("Range: ["+str(tkeys[0])+", "+str(tkeys[-1])+"]")
#     print(times[6.5])

#     tLo, tHi = getBounds(tkeys, target_time)
#     if (tLo >= 0):

#         print("Bounds: "+str(tLo)+", "+str(tHi))
#         v1 = np.array(extractData(times, tLo))
#         v2 = np.array(extractData(times, tHi))
#         x_range = [tLo, tHi]
#         v_range = [v1, v2]
#         interp_cur = interp1d(x_range, v_range, kind='linear', axis=0)
#         ice_new = interp_cur(target_time)

#         print("----- "+str(tLo)+" -----")
#         print(str(v1))
#         print("----- "+str(target_time)+" -----")
#         print(str(ice_new))
#         print("----- "+str(tHi)+" -----")
#         print(str(v2))
#     else:
#         print("Value "+str(target_time)+" out of range ["+str(tkeys[0])+", "+str(tkeys[-1])+"]")
#     #-- end if
# else:
#    usage(argv[0])
#-- end if
