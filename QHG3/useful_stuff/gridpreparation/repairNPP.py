#!/usr/bin/python 

from sys import argv, exit
from math import *
import numpy as np
import h5py

REG_OCEANIA_LONMIN = 115.0;
REG_OCEANIA_LONMAX = 150.0;
REG_OCEANIA_LATMIN = -12.0;
REG_OCEANIA_LATMAX =   1.0;
NPP_MIN = 0.01

def calcMiami(temp, rain, i):
    nppT = 3000.0 / (1 + exp(1.315 - 0.119*temp[i]))
    nppP = 3000.0 * (1 - exp(-0.000664*rain[i]))

    return 0.000475*min(nppT,nppP)
#-- end def
    



#---------------------------------------------------
#-- repair_file
#--
def repair_file(qdf_file):
    iResult = -1
    try:
        qdf = h5py.File(qdf_file,'r+')
    except:
        print("Couldn't open [" + qdf_file + "] as QDF file")
    else:
        iResult = repair_contents(qdf)
        if (iResult == 0):
            print("flush it")
            qdf.flush()
            qdf.close()
        #-- end if
    
    #-- end try
    
#-- end def


#---------------------------------------------------
#-- repair_contents
#--
def repair_contents(qdf):
    iResult = -1
    try:
        geogroup = qdf["/Geography"]
    except:
        print("ERROR: Geography group not found")
    else:
        if geogroup.__contains__('Altitude'):
            
            alt = np.array(geogroup['Altitude'])
            
            try:
                cligroup = qdf["/Climate"]
                veggroup = qdf["/Vegetation"]
            except:
                print("ERROR: Climate group not found")
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
                                            print("fixing bnpp and tnpp")
                                            for i in range(len(alt)):
                                                if alt[i] > 0:
                                                    if  (lon[i] > REG_OCEANIA_LONMIN) and (lat[i]  > REG_OCEANIA_LATMIN) and (lon[i] < REG_OCEANIA_LONMAX) and (lat[i]  < REG_OCEANIA_LATMAX):
                                                        if (bnpp[i] < NPP_MIN):
                                                            bnpp[i] = calcMiami(temp, rain, i)
                                                        #-- end if                       
                                                    #-- end if
                                                    if (bnpp[i] < NPP_MIN):
                                                        bnpp[i] = NPP_MIN
                                                    #--- end if
                                                        
                                                else:
                                                    bnpp[i] = 0
                                                #-- end if
                                                tnpp[i] = bnpp[i]
                                            #-- end for

                                            del veggroup['BaseNPP']
                                            veggroup.create_dataset("BaseNPP", bnpp.shape, bnpp.dtype, bnpp)
                                                
                                            del veggroup['NPP']
                                            veggroup.create_dataset("NPP", tnpp.shape, tnpp.dtype, tnpp)

                                            iResult = 0
                                            
                                        #-- end if lat
                                    #-- end if lon
                                        
                                #-- end if alt
                                            
                            #-- end if tnpp
                        #-- end if bnpp
                    #-- end if temp 
                #-- end if rain
            #-- end try
        #- end if
                                
    return iResult
#-- end def


#---------------------------------------------------
#-- usage
#--
def usage(appname):
    print("%s - apply miami model to land cells with less than minimal NPP" % (appname))
    print("Usage:")
    print("  %s  <qdf_file>" % (appname))
    print("where")
    print("  qdf_file    QDF file to be modified")
    print("In order to give npp values to oceania islands, <qdf_file> must contain a 'Climate' group")
#-- end def
    
#---------------------------------------------------
#-- main
#--  expect
#--     <qdf_file> ["alt" <alt_val>]] ["npp" <npp_val>] ["ice" <ice_val>]
if __name__ == '__main__':
    if (len(argv) > 1):
        qdf_file = argv[1]
        repair_file(qdf_file)
        
    else:
        usage(argv[0])
    #-- end if
#-- end main
