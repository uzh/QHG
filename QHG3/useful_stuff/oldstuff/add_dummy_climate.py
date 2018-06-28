#!/usr/bin/python

from sys import argv, exit
import numpy as np
import h5py


if len(argv) != 2:
    print("usage: " + argv[0] + " <QDF> ")
    exit()


qdf = h5py.File(argv[1],'r+')


geogroup = qdf["/Geography"]

try:
    cli = qdf.create_group("/Climate")
except:
    print("WARNING: Climate group already exists in " + argv[1] + "... replacing")
    del qdf["/Climate"]
    cli = qdf.create_group("/Climate")
    
# Climate group attributes
cli.attrs['NumCells'] = geogroup.attrs['NumCells']
nCells = geogroup.attrs['NumCells'][0]

dyn = np.int8(0)
cli.attrs.create("Dynamic", dyn, dyn.shape, dtype='int8')

dyn = np.int8(0)
cli.attrs.create("NumSeasons", dyn, dyn.shape, dtype='int8')


datas = np.zeros(nCells, dtype='float64')

cli.create_dataset("ActualRains", datas.shape, datas.dtype, datas)

cli.create_dataset("ActualTemps", datas.shape, datas.dtype, datas)

cli.create_dataset("AnnualRainfall", datas.shape, datas.dtype, datas)

cli.create_dataset("AnnualMeanTemp",  datas.shape, datas.dtype, datas)

curs = np.int32([0])
cli.create_dataset("CurSeason", curs.shape, curs.dtype, curs)


qdf.flush()
qdf.close()


