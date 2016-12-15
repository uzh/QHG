#!/usr/bin/python 

from sys import argv, exit
from math import *
import h5py

if len(argv) == 3:

    filename = argv[1]
    gridfilename = argv[2]

    try:
        fdata = h5py.File(filename,"r+")
    except:
        print("error opening file " + filename)
        exit(-1)

    if ("Grid" in fdata):
        del fdata["/Grid"]
    if ("Geography" in fdata):
        del fdata["/Geography"]
    if ("Climate" in fdata):
        del fdata["/Climate"]

    if ("Grid" not in fdata and "Geography" not in fdata):
        try:
            fdata["/Grid"] = h5py.ExternalLink(gridfilename,"/Grid")
            fdata["/Geography"] = h5py.ExternalLink(gridfilename,"/Geography")
            fdata["/Climate"] = h5py.ExternalLink(gridfilename,"/Climate")     

            fdata.flush()
            fdata.close()

            print("processed file " + filename + "; gridfilename " + gridfilename)

        except:
            print("some error occurred, probably related to " + gridfilename)
            exit(-1)

    else:
        print(filename + ": failed to substitute existing Grid")
        exit(-1)

        
else:
    print("USAGE: LinkGridGeoQDF.py <qdf to be modified> <grid+geo file>")

