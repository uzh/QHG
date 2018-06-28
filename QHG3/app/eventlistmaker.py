#!/usr/bin/python
from sys import argv

if len(argv) > 3:
    tstart = int(argv[1])
    tstep  = int(argv[2])
    output = argv[3]

        
    fOut = file(output, "wt")
    if (tstart < 0):
        for i in range(tstep, -tstart+tstep, tstep): 
            fOut.write("env|geo+climate+veg+nav:navworld_%03d_kya_256.qdf@[%d]\n" % (i, -i*1000))
        #-- end for
    else:
        for i in range(tstart, 0, -tstep):
            a = tstart - i;
            fOut.write("env|geo+climate+veg+nav:navworld_%03d_kya_256.qdf@[%d]\n" % (i, a*1000))
        #-- end if
    #-- end for
    fOut.close()
else:
    print("%s <start_t> <delta_t> <output>" % (argv[0]))
    print("creates an event file with 'env|geo+climate+veg+nav' events")
    print("  start_t and delta_t are given in ky")
    print("  it is possible to have start_t < 0: creates events  backwards from 0")
#-- end if 
