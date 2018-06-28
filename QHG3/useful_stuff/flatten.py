#!/usr/bin/python 

from sys import argv, exit
from math import *
import numpy as np
import h5py

#-- lon/lat rectangle encompassing oceania
REG_OCEANIA_LONMIN = 115.0;
REG_OCEANIA_LONMAX = 150.0;
REG_OCEANIA_LATMIN = -12.0;
REG_OCEANIA_LATMAX =   1.0;
NPP_MIN = 0.08

#---------------------------------------------------
#-- calcMiami
#--   calculate npp value for given rain and
#--   temperature using the Miami model
#--
def calcMiami(temp, rain, i):
    nppT = 3000.0 / (1 + exp(1.315 - 0.119*temp[i]))
    nppP = 3000.0 * (1 - exp(-0.000664*rain[i]))

    return 0.000475*min(nppT,nppP)
#-- end def
    



#---------------------------------------------------
#-- flatten
#--
def flatten(qdf_file, bDoAlt, def_alt, bDoNPP, def_npp, bDoIce, def_ice):
    iResult = -1
    try:
        qdf = h5py.File(qdf_file,'r+')
    except:
        print("Couldn't open [" + geogrid_qdf + "] as QDF file")
    else:
        try:
            geogroup = qdf["/Geography"]
        except:
            print("ERROR: Geography group not found in " + qdf_file)
        else:
            if geogroup.__contains__('Altitude'):
                
                alt = np.array(geogroup['Altitude'])
                if bDoNPP:
                    try:
                        veggroup = qdf["/Vegetation"]
                    except:
                        print("ERROR: vegetation group not found in " + qdf_file)
                    else:
                        if veggroup.__contains__('BaseNPP'):
                            bnpp = np.array(veggroup['BaseNPP'])

                            if veggroup.__contains__('NPP'):
                                tnpp = np.array(veggroup['NPP'])

                                print("setting bnpp and tnpp to %f" % (def_npp))

                                bnpp[np.where (alt > 0)]  = def_npp
                                bnpp[np.where (alt <= 0)] = -1
                                tnpp[np.where (alt > 0)]  = def_npp
                                tnpp[np.where (alt <= 0)] = -1

                                del veggroup['BaseNPP']
                                veggroup.create_dataset("BaseNPP", bnpp.shape, bnpp.dtype, bnpp)
                            
                                del veggroup['NPP']
                                veggroup.create_dataset("NPP", tnpp.shape, tnpp.dtype, tnpp)
                  
                                iResult = 0
                            #-- end if
                        #-- end if
                    #-- end try
                #-- end if
                if ((iResult == 0) or not bDoNPP) and (bDoAlt or bDoIce):

                    if bDoAlt:
                        alt[np.where (alt > 0)]  = def_alt
                        alt[np.where (alt <= 0)] = -1
                        del geogroup['Altitude']
                        geogroup.create_dataset("Altitude", alt.shape, alt.dtype, alt)

                        h20 =  np.array(geogroup['Water'])
                        h20 = 0 * h20
                        del geogroup['Water']
                        geogroup.create_dataset("Water", h20.shape, h20.dtype, h20)

                        cst =  np.array(geogroup['Coastal'])
                        cst = 0 * cst
                        del geogroup['Coastal']
                        geogroup.create_dataset("Coastal", cst.shape, cst.dtype, cst)
                        
                    #-- end if
                    
                    if bDoIce:
                        ice = np.array(geogroup['IceCover'])
                        ice = def_ice*ice
                        del geogroup['IceCover']
                        geogroup.create_dataset("IceCover", ice.shape, ice.dtype, ice)
                    #-- end if
                    
                    iResult = 0
                #-- end if
            #-- end if
            if (iResult == 0) and bDoNPP and (def_npp > 0):
                try:
                    cligroup = qdf["/Climate"]
                    veggroup = qdf["/Vegetation"]
                except:
                    print("ERROR: Climate group not found in " + qdf_file)
                else:
                    if cligroup.__contains__('ActualRains'):
                        if cligroup.__contains__('ActualTemps'):
                            if veggroup.__contains__('BaseNPP'):
                                if veggroup.__contains__('NPP'):
                                    if geogroup.__contains__('Altitude'):
                                        if geogroup.__contains__('Longitude'):
                                            if geogroup.__contains__('Latitude'):
                                                temp = np.array(cligroup['ActualTemps'])
                                                rain = np.array(cligroup['ActualRains'])
                                                alt  = np.array(geogroup['Altitude'])
                                                lon  = np.array(geogroup['Longitude'])
                                                lat  = np.array(geogroup['Latitude'])
                                                bnpp = np.array(veggroup['BaseNPP'])
                                                tnpp = np.array(veggroup['NPP'])
                                                print("fixin bnpp and tnpp")
                                                for i in range(len(alt)):
                                                    if alt[i] > 0:
                                                        if  (lon[i] > REG_OCEANIA_LONMIN) and (lat[i]  > REG_OCEANIA_LATMIN) and (lon[i] < REG_OCEANIA_LONMAX) and (lat[i]  < REG_OCEANIA_LATMAX):
                                                            if (bnpp[i] < NPP_MIN):
                                                                bnpp[i] = calcMiami(temp, rain, i)
                                                                if (bnpp[i] < NPP_MIN):
                                                                    bnpp[i] = NPP_MIN
                                                                #--- end if
                                                            #-- end if                       
                                                        #-- end if
                                                    else:
                                                        bnpp[i] = 0
                                                    #-- end if
                                                    tnpp[i] = bnpp[i]

                                                #-- end for
                                                del veggroup['BaseNPP']
                                                veggroup.create_dataset("BaseNPP", bnpp.shape, bnpp.dtype, bnpp)
                                                
                                                del veggroup['NPP']
                                                veggroup.create_dataset("NPP", tnpp.shape, tnpp.dtype, tnpp)

                                            #-- end if lat
                                        #-- end if lon
                                        
                                    #-- end if alt
                                            
                                #-- end if tnpp
                            #-- end if bnpp
                        #-- end if temp 
                    #-- end if rain
                #-- end try
            #- end if
                                
            if (iResult == 0):
                print("flush it")
                qdf.flush()
                qdf.close()
            #-- end if
        #-- end try
    #-- end try
#-- end def


#---------------------------------------------------
#-- usage
#--
def usage(appname):
    print("%s - flatten a qdf file" % (appname))
    print("Usage:")
    print("  %s  <qdf_file> [\"alt\" <alt_val>] [\"npp\" <npp_val>] [\"ice\" <ice_val>]" % (appname))
    print("where")
    print("  qdf_file    QDF file to be modified")
    print("  alt_val     uniform altitude for landmasses")
    print("  npp_val     uniform npp value for landmasses")
    print("  ice_val     uniform ice value for ice covered cells")
    print("In order to give npp values to oceania islands, <qdf_file> must contain")
    print("a 'Climate' group with rain and temperature arrays")
#-- end def

    
#---------------------------------------------------
#-- main
#--  expect
#--     <qdf_file> ["alt" <alt_val>]] ["npp" <npp_val>] ["ice" <ice_val>]
#--
if (len(argv) > 1):
    qdf_file = argv[1]
    bDoAlt  = False
    bDoNPP  = False
    bDoIce  = False
    alt_val = 0
    npp_val = 0
    ice_val = 0

    i = 2
    try:
        print("i %d, len %d" % (i, len(argv)))
        while i+1 < len(argv):
            print("i %d, len %d, [%s]" % (i, len(argv), argv[i]))
            if (argv[i] == "alt"):
                alt_val = float(argv[i+1])
                bDoAlt = True
                i = i+2
            elif (argv[i] == "npp"):
                npp_val = float(argv[i+1])
                bDoNPP = True
                i = i+2
            elif (argv[i] == "ice"):
                ice_val = float(argv[i+1])
                bDoIce = True
                i = i+2
            else:
                i = i+1
            #-- end if
            
        #-- end while
    except Exception as e:
        print(e.message)
    else:
        flatten(qdf_file, bDoAlt, alt_val, bDoNPP, npp_val, bDoIce, ice_val)


    #-- end try
else:
    usage(argv[0])
#-- end main
