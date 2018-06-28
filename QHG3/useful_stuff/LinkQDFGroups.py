#!/usr/bin/python 

from sys import argv, exit
from math import *
import h5py

if (len(argv) >= 3) and (len(argv)%2 == 0):
    gridfilenames = {}
    filename = argv[1]

    index = 2
    while index < len(argv):
        gridfilenames[argv[index]] = argv[index+1].upper()
        index = index+2
    #-- end while
    
    try:
        fdata = h5py.File(filename,"r+")
    except:
        print("error opening file " + filename)
        exit(-1)
    #-- end try

    
    for f in gridfilenames:
        if ".." in f:
            print("WARNING: relative path used:[%s]", f)
        #-- end if
        sWhat = gridfilenames[f]
        try:
            if ("S" in sWhat):
                if ("Grid" in fdata):
                    del fdata["/Grid"]
                #-- end if
                fdata["/Grid"] = h5py.ExternalLink(f, "/Grid")
            #-- end if
            if ("G" in sWhat):
                if ("Geography" in fdata):
                    del fdata["/Geography"]
                #-- end if
                fdata["/Geography"] = h5py.ExternalLink(f, "/Geography")
            #-- end if
            if ("C" in sWhat):
                if ("Climate" in fdata):
                    del fdata["/Climate"]
                #-- end if
                fdata["/Climate"] = h5py.ExternalLink(f, "/Climate")     
            #-- end if
            if ("V" in sWhat):
                if ("Vegetation" in fdata):
                    del fdata["/Vegetation"]
                #-- end if
                fdata["/Vegetation"] = h5py.ExternalLink(f, "/Vegetation")     
            #-- end if
            if ("N" in sWhat):
                if ("Navigation" in fdata):
                    del fdata["/Navigation"]
                #-- end if
                fdata["/Navigation"] = h5py.ExternalLink(f, "/Navigation")     
            #-- end if
            if ("M" in sWhat):
                if ("MoveStatistics" in fdata):
                    del fdata["/MoveStatistics"]
                #-- end if
                fdata["/MoveStatistics"] = h5py.ExternalLink(f, "/MoveStatistics")     
            #-- end if


        except:
            print("some error occurred, probably related to " + gridfilename)
            exit(-1)

    #-- end for
    
    fdata.flush()
    fdata.close()
    print("processed file " + filename)
        
        
else:
    print("USAGE: LinkQDFGroups.py <qdf_to_be_modified> (<grid+geo_file> <what>)*")
    print("where:")
    print("  grid+geo_file  file to which to link to")
    print("  what           a string consisting of any combination of 'S','G', 'C', 'V', 'N', 'M'")
    print("                    S: link to 'Grid' group")
    print("                    G: link to 'Geography' group")
    print("                    C: link to 'Climate' group")
    print("                    V: link to 'Vegetation' group")
    print("                    N: link to 'Navigation' group")
    print("                    M: link to 'MoveStatistics' group")
