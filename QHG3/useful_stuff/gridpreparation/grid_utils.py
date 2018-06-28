#!/usr/bin/python

import numpy as np
from scipy.interpolate import RectBivariateSpline
import h5py

#-----------------------------------------------------------------------------
#-- replaceQDFArray
#--
def replaceQDFArray(qdf_data, group_name, array_name, array_val):
    iResult = -1
 
    if qdf_data.__contains__(group_name):
        gr = qdf_data[group_name]
        if gr.__contains__(array_name):
            del gr[array_name]
        #-- end if

        gr.create_dataset(array_name, array_val.shape, array_val.dtype, array_val)
        iResult = 0
    else:
        print("No group ["+group_name+"] in file")
    #-- end if
    return iResult
#-- end def


#-----------------------------------------------------------------------------
#-- replaceQDFFileArray
#--
def replaceQDFFileArray(qdf_file, group_name, array_name, array_val):
    iResult = -1
    try:
        qdf_data = h5py.File(qdf_file,'r+')
        iResult = replaceQDFArray(qdf_data, group_name, array_name, array_val)
        qdf_data.close
    except Exception, e:
        iResult = -1
        print("Couldn't open ["+qdf_file+"] as QDF file (%s)" % e)
    #-- end if
    return iResult
#-- end def


#-----------------------------------------------------------------------------
#-- extractQDFCoordinates
#--
def extractQDFCoordinates(qdf_file):
    print("extractQDFCoordinates for ["+qdf_file+"]");
    vLon = None
    vLat = None
    qdf_data = h5py.File(qdf_file, 'r')
    
    if qdf_data.__contains__('Geography'):
        gr = qdf_data['Geography']
        if gr.__contains__('Longitude'):
            vLon = np.array(gr['Longitude'])
            if gr.__contains__('Latitude'):
                vLat = np.array(gr['Latitude'])
            #-- end if
        #-- end if
    #-- end if
    qdf_data.close()
    return vLon, vLat
#-- end def


#-------------------------------------------------------------------------
#-- doGridInterpolation
#--   qLon:       array of longitudes (from Geography)
#--   qLat:       array of latitudes  (from Geography)
#--   alt_lon:    longitude values of value grid
#--   alt_lat:    latitude values of value grid
#--   alt_values: values of value grid
#--   alt_type:   data type of values
#--   bInvertLat: invert latitude values - only do that if latitudes are
#--               symmetrical around 0
#--
def doGridInterpolation(qLon, qLat, alt_lon, alt_lat, alt_values, alt_type, bInvertLat):
    alt_new = None
    if not alt_values is None:
        
        lat_factor = 1
        if bInvertLat:
            lat_factor = -1
        #-- end if
            
        alt_lon = np.hstack([alt_lon, alt_lon[1:]+360])
  
        # prevent errors around lon 0
        alt_values = np.hstack([alt_values, alt_values[:,1:]])
  
        alt_values.reshape([len(alt_lon), len(alt_lat)])
        alt_rbs = RectBivariateSpline(lat_factor*alt_lat, alt_lon, alt_values, kx=2,ky=2)
        alt_new  = np.zeros(len(qLon), dtype=alt_type)
        # prevent errors around lon 0
        qLon = 360+qLon
        for i in range(len(qLat)):
            alt_new[i] = np.array(alt_rbs(lat_factor*qLat[i], qLon[i]), dtype=alt_type)
        #-- end for
    #-- end if

    return alt_new
#-- end def
