#!/usr/bin/python

from sys import argv, exit
import h5py
import numpy as np



if (len(argv) > 1):
    bAll = False
    index = 1
    if (argv[1] == "all"):
        bAll = True
        index = 2
    #-- end if
    if (len(argv) > index):
        qdffile = argv[index]
        
        popname  = None
        if (len(argv) > index+1):
            popname = argv[2]
        #-- end if
    else:
        print("expected <qdffile> argument")
    #-- end if
    try: 
        qdf = h5py.File(qdffile,'r')
    except:
        print("Couldn't open["+qdffile+"] as QDF file")
    else:
        if (qdf.__contains__('Populations')):
            pops = qdf['Populations']
            if popname is None:
                popname = pops.values()[0].name.split("/")[-1]
            #-- end if
            aa0={}
            aa1={}
            if (pops.__contains__(popname)):
                p0 = pops[popname]
                l0=0
                l1=0
                for x in p0.attrs:
                    if (type(p0.attrs[x])==np.ndarray) and (len(p0.attrs[x]) > 1):
                        aa1[x]=p0.attrs[x]
                        if len(x) > l1:
                            l1 = len(x)
                        #-- end if
                    else:   
                        aa0[x]=p0.attrs[x]
                        if len(x) > l0:
                            l0 = len(x)
                        #-- end if
                    #-- end if
                    
                #-- end for
                for n in sorted(aa0):
                    print(n.ljust(l0)+"\t"+str(aa0[n]))
                #-- end for
                if (bAll):
                    for n in sorted(aa1):
                        print(n.ljust(l1)+"\t"+str(aa1[n]).replace("\n", " "))
                    #-- end for
                #-- end if
            else:
                print("no population '"+popname+"' found in 'Populations' group of ["+qdffile+"]")
            #-- end if
        else:
            print("no 'Populations' group found in ["+qdffile+"]")
        #-- end if
    #-- end try<
else:
    print("Usage:")
    print("  "+argv[0]+" [\"all\"] <qdffile> [<poname>]")
#-- end if

