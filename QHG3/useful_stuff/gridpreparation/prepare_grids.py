#!/usr/bin/python

from sys import argv, exit
import os
import re
import numpy as np
from shutil import copyfile
import subprocess
import h5py

import npp_interpolator as ni
import ice_gano_interpolator as igi
import ice_interpolator as ii
import climate_interpolator as ci
import alt_interpolator as ai
import sealevel_changer as slc
import grid_utils
import grid_fixer as gf
import repairNPP

if ('QHG_DIR') in os.environ:
    QHG3_BASE = os.environ['QHG_DIR']
else:
  QHG3_BASE    =  os.environ['HOME']+"/progs/QHG3/"
#-- end if

# grid stuff
CORE_DATA    = QHG3_BASE+"/useful_stuff/core_data"
IMPORT_DIR   = QHG3_BASE+"/import"
grid_level   = 128
ico_grid_pat   = CORE_DATA+"/GridSGCV_ieq_###.qdf"

# geo stuff
topo_file   = CORE_DATA+"/ETOPO1_Bed_g_gmt4.grd"
#gridfix_pat = CORE_DATA+"/gridfixes2_###.dat"
gridfix_pat = CORE_DATA+"/gridfixes2_nobridge_###.dat"

# sea  levels
sea_levels_w   = CORE_DATA+"/waelbroeck2002_table.txt"
sea_levels_w_tcol = 0
sea_levels_w_hcol = 4
sea_levels_g   = CORE_DATA+"/sealevel_GanopolskiR.dat"
sea_levels_g_tcol = 0
sea_levels_g_hcol = 1

# water stuff
COASTAL_DIST ="3"
COASTAL_OUT  ="coasts_temp.qdf"

river_file     = CORE_DATA+"/ne_10m_rivers_lake_centerlines_scale_rank" 
river_field    = "strokeweig" 
surf_file_pat  = CORE_DATA+"/eq###.ieq"

# ice
ice_peltier_dir      = CORE_DATA+"/CLIMATE"
ice_peltier_filepat  = 'ice5g_v1.2_([1-9]*)_([0-9]*).nc'
ice_gano_file  =  CORE_DATA+'/icesheet_Ganopolski.nc'
bUsePeltier = False

# climate stuff
climate_dir     = CORE_DATA+"/CLIMATE/BRIDGE"
climate_filepat = '([0-9]*)ka\.ann\.nc'

# npp stuff
npp_file        = CORE_DATA+"/npp_timmermann.nc"
bPrecomputedNPP = True

# navigation
SEAWAY_DIST = "200"
SEAWAY_BOX  = "103:-11:130:8,120:-15:154:0"
#----------------------------------------------------------------------------
#-- doWork
#--
def doWork(grid_level, tMin, tMax, deltaT, output_prefix, sealevels, bSkipInitial):

    ico_grid  = ico_grid_pat.replace("###", str(grid_level))
    surf_file = surf_file_pat.replace("###", str(grid_level))
    
    base_grid   = CORE_DATA+"/WorldMap_ieq_"+str(grid_level)+"_"+str(tMax)+".qdf"
    output_pat  = output_prefix+"_###_kya_"+str(grid_level)+".qdf"
    gridfix_file = gridfix_pat.replace("###", str(grid_level))

    iResult = 0


    if not bSkipInitial:
        # copy ico grid to basegrid & set topography & add water
        try:
            copyfile(ico_grid, base_grid)
        except:
            print("Couldn't copy ["+ico_grid+"] to ["+ base_grid + "]")
            iResult = -1;
        else:
            print("copied  ["+ico_grid+"] to ["+ base_grid + "]")
            aip = ai.AltitudeInterpolator(topo_file)
            iResult = aip.doInterpolation(base_grid)
            
            if iResult == 0:
                # add rivers
                iResult = subprocess.call([IMPORT_DIR+"/VectorImport",
                                           "-s", surf_file,
                                           "-q", base_grid,
                                           "-v", river_file+".shp",
                                           "-d", river_file+".dbf",
                                           "-f", river_field])
                print("VectorImport: "+str(iResult))
            #-- end if
        #-- end try
    #-- end if
    qLon, qLat = grid_utils.extractQDFCoordinates(base_grid)


    if iResult == 0:
        # create climate interpolator
        clim_int = ci.ClimateInterpolator(climate_dir, climate_filepat)
        if (clim_int.bPronto):
            print("Have ClimateInterpolator")
        else:
            print("Creation of ClimateInterpolator failed")
            #-- end if
        #-- end if

    
    if iResult == 0:
        if bUsePeltier:
            # create ice interpolator
            ice_int  = ii.IceInterpolator(ice_dir, ice_filepat)
            print("Have IceInterpolator")
        else:
            # create gano ice interpolator
            ice_int  = igi.IceGanoInterpolator(ice_gano_file)
            print("Have IceGanoInterpolator")
        #-- end if
    #-- end if


    if iResult == 0:
        if (sealevels == "wael"):
            sea_level_changer = slc.SeaLevelChanger(sea_levels_w, sea_levels_w_tcol, sea_levels_w_hcol)
        else:
            sea_level_changer = slc.SeaLevelChanger(sea_levels_g, sea_levels_g_tcol, sea_levels_g_hcol)
        #-- end if
        print("Have SeaLevelChanger")
    #-- end if

    if iResult == 0:
        # create npp interpolator
        npp_int  = ni.NPPInterpolator(npp_file)
        print("Have NPPInterpolator")
    #-- end if

    
    # loop
    if iResult == 0:
    
        t = tMax
        while (t > tMin) and (iResult == 0):
            qdf_name=output_pat.replace("###", "%03d"%(t))
            copyfile(base_grid, qdf_name)

            # create or destroy land bridges
            gfixer = gf.GridFixer()
            iResult = gfixer.applyFixes(qdf_name, gridfix_file)
            if (iResult == 0):
 
                # set sea level
                iResult = sea_level_changer.setSeaLevel(qdf_name, t)
                if (iResult == 0):
                    # find coasts for new sea level
                    sCoastalOut = COASTAL_OUT+"_"+str(t)
                    print("  "+str([IMPORT_DIR+"/Coastal",
                                       "-i", qdf_name,
                                       "-d", COASTAL_DIST,
                                       "-o", sCoastalOut]))
                    iResult = subprocess.call([IMPORT_DIR+"/Coastal",
                                       "-i", qdf_name,
                                       "-d", COASTAL_DIST,
                                       "-o", sCoastalOut])
                    if (iResult == 0):
                        qdfC = h5py.File(sCoastalOut, 'r')
                        temp_coast = np.int32(np.array(qdfC['Geography']['Coastal']))
                        qdfC.close()
                        qdf = h5py.File(qdf_name,'r+')
                        iResult = grid_utils.replaceQDFArray(qdf, 'Geography', 'Coastal', temp_coast)
                        qdf.flush()
                        qdf.close()
                    #-- end if
                    os.remove(sCoastalOut)

                    if (iResult == 0) :
                        print("  "+str([IMPORT_DIR+"/SeaWays",
                                        "-i", qdf_name,
                                        "-d", SEAWAY_DIST,
                                        "-b", SEAWAY_BOX]))
                        iResult = subprocess.call([IMPORT_DIR+"/SeaWays",
                                                   "-i", qdf_name,
                                                   "-d", SEAWAY_DIST,
                                                   "-b", SEAWAY_BOX])
                    
                    
                    if (iResult == 0):
                        t_act = t
                        t_sim = t
                        if t  > 120:
                            c_level = sea_level_changer.calcSeaLevel(t)
                            t_sim = sea_level_changer.findLastTimeWithLevel(c_level, 120)
                            print("L %f -> T %f" % (c_level, t_act))
                        #-- end if

                        # peltier ice data only reaches as far back as 120ky
                        if (bUsePeltier):
                            t_act = t_sim
                        #-- end if

                        # our climate data only reaches as far back as 120ky
                        temp_new, rain_new = clim_int.doInterpolation(t_sim, qLon , qLat)

                        if bUsePeltier:
                            ice_new = ice_int.doInterpolation(t_act, qLon , qLat)
                        else:
                            ice_tmp = ice_int.doInterpolation(t_act, qLon, qLat)
                            ice_new = np.where(ice_tmp < 0, 0, ice_tmp)
                        #-- end if
                        
                        if bPrecomputedNPP:
                            print("using precomputd NPPs")
                            npp_tmp = npp_int.doInterpolation(t_act, qLon, qLat)
                            npp_new = np.where(npp_tmp < 0, 0, npp_tmp)
                        else:
                            # same size as e.g. rain
                            print("setting empty BaseNPP")
                            npp_new = np.zeros(rain_new.shape, dtype=np.float64)
                        #-- end if
                        
                        if True: #try: 
                            qdf = h5py.File(qdf_name,'r+')

                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Climate', 'ActualTemps', temp_new)
                            #--  end if
                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Climate', 'AnnualMeanTemp', temp_new)
                            #-- end if
                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Climate', 'ActualRains', rain_new)
                            #--  end if
                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Climate', 'AnnualRainfall', rain_new)
                            #-- end if
                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Geography', 'IceCover', ice_new)
                            #-- end if
                            if (iResult == 0):
                                iResult = grid_utils.replaceQDFArray(qdf, 'Vegetation', 'BaseNPP', npp_new)
                            #-- end if
                            if (iResult == 0):
                                iResult = repairNPP.repair_contents(qdf)
                            #-- end if
                            qdf.flush()
                            qdf.close()
                            if (iResult == 0):
                                print("completed ["+qdf_name+"]")
                            #-- end if
                        else: #except:
                            print("Couldn't open ["+qdf_name+"] as QDF file")
                            iResult = -1
                        #-- end try
                    #-- end if
                #-- end if
            #-- end if
            t = t - deltaT
        #-- end while
    #-- end if
#-- end def
        
if len(argv) > 5:
    grid_level = int(argv[1])
    tRange     = argv[2]
    deltaT     = float(argv[3])
    output_prefix = argv[4]
    sealevels = argv[5]
    if len(argv) > 6:
        bSkipInitial = True
    else:
        bSkipInitial = False
    #-- end if
    tMin = 0
    tMax = 120
    a = tRange.split(':')
    if (len(a) > 0):
        if (len(a) == 1):
            if (a[0] != ""):
                tMax = float(a[0])
            #-- end if
            tMin = 0
        else:
            if  (a[0] != ""):
                tMin = float(a[0])
            #-- end if
            if  (a[1] != ""):
                tMax = float(a[1])
            #-- end if
        #-- end if
    #-- end if
    doWork(grid_level, tMin, tMax, deltaT, output_prefix, sealevels, bSkipInitial)
else:
    print(argv[0]+" - prepares grids with climate, ice and water for a series of times")
    print("usage")
    print("  " + argv[0]+" <grid_level> [<tMin>:]<tMax> <deltaT> <prefix> <sealevels>")
    print("where")
    print("  grid_level   level of eqsahedron grid (128 or 256)")
    print("  tMax         highest time in ky for which to create a grid (max 120)")
    print("  deltaT       decrement in ky")
    print("  prefix       prefix for output files")
    print("  sealevels    one of \"wael\" and \"gano\" (waelbroek or ganopolski)")
    print("")
#-- end if
