#!/usr/bin/python

from sys import argv, exit
import subprocess
import re
import pca_maker

PAR_DATADIR  = 1
PAR_FILEBODY = 2
PAR_PCNUMS   = 4

def usage(appname):
    print("%s  - using eigensoft to create pca" % (appname))
    print("Usage:")
    print("  %s -d <datadir> -f <filebody> -p <pc1>,<pc2>" % (appname))
    print("            [-e <eigenbindir>] [-v]")
    print("")
#-- end def


iResult = 0

sDataDir = ""
sFileBody = ""
pc1=0
pc2=0
sEigenBinDir = "/home/jody/EIG5.0.2/bin"
bVisual = False

if (len(argv) > 6):
    iNeeded = PAR_DATADIR | PAR_FILEBODY | PAR_PCNUMS
    i = 1
    while (iResult == 0) and (i < len(argv)):
        if argv[i] == '-d':
            iNeeded = iNeeded & ~PAR_DATADIR
            sDataDir = argv[i+1]
            i = i+2
        elif argv[i] == '-f':
            iNeeded = iNeeded & ~PAR_FILEBODY
            sFileBody = argv[i+1]
            i = i+2
        elif argv[i] == '-p':
            iNeeded = iNeeded & ~PAR_PCNUMS
            sPCNums = argv[i+1]
            i = i+2
        elif argv[i] == '-e':
            sEigenBinDir = argv[i+1]
            i = i+2
        elif argv[i] == '-v':
            bVisual = True
            i = i+1
        else:
            i = i+1
        #-- end if
    #-- end while

    if (iNeeded == 0):
        try:
            ap = sPCNums.split(",")
            pc1 = int(ap[0])
            pc2 = int(ap[1])
        except:
            print("Invalid pca definition")
            iResult = -1;
        #--- end try

        if iResult == 0:
            print("sDataDir:     [%s]" % (sDataDir))
            print("sFileBody:    [%s]" % (sFileBody))
            print("pca1:         [%s]" % (pc1))
            print("pca2:         [%s]" % (pc2))
            print("sEigenBinDir: [%s]" % (sEigenBinDir))
            print("bVisual:      [%s]" % (bVisual))


            pm = pca_maker.PCAMaker(sDataDir, sFileBody, sEigenBinDir)

            iResult = pm.makePCA(pc1, pc2, bVisual)

            if (iResult == 0):
                print("makePCA successful")
                f = open(sDataDir+"/gag.gnu", "w")
                f.write("load \"%s\"\n" % (pm.gnuplotfile))

                if (bVisual):
                    f.write("pause 9999")
                #-- end if
                f.close()

                try:
                    fNull = open("/dev/null", "w")
                    print("calling gnuplot ...")
                    arglist=["gnuplot", sDataDir+"/gag.gnu"]
                    iResult = subprocess.call(arglist, stderr=fNull)
                except:
                    print("gnuplot call failed - bye")
                else:
                    print("+++ success +++");
                #-- end try
            #-- end if
        #-- end if
    else:
        print("Error: mandataory parameters missing: "+str(iNeeded))
    #-- end if
else:
    usage(argv[0])
#-- end if
   
