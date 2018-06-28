#!/usr/bin/python

from sys import argv, exit
import h5py
import numpy as np


if len(argv) != 4:
    print("usage: " + argv[0] + " <Waelbroeck table> <kya> <QDF>")
    exit()


waelfile = open(argv[1],'r')
wael = waelfile.readlines()
waelfile.close()

wael = [ list(map(float,line.split())) for line in wael if len(line) > 1 and line[0] != '#' ]

age = float(argv[2])

i = 0
while wael[i][0] < age :
    i += 1

x1 = wael[i][0]
x0 = wael[i-1][0]
y1 = wael[i][4]
y0 = wael[i-1][4]

sealev = y0 + (age - x0) * (y1 - y0) / (x1 - x0)  # linear interpolation

print("setting relative sea level to %f in " % (sealev) + argv[3])

qdf = h5py.File(argv[3],'r+')

qdf.attrs['Time'] = - age * 1000.0   # time in years

geo = qdf['/Geography']

geo.attrs['SeaLevel'][0] = sealev

newalt = np.float64( np.array(geo['Altitude'][:]) - sealev )

del geo['Altitude']

geo.create_dataset('Altitude', newalt.shape, newalt.dtype, newalt)

qdf.flush()
qdf.close()







