#!/usr/bin/python

from sys import argv
import glob
import os
import re
import subprocess

PAR_INDIR     =  1
PAR_GRID      =  2
PAR_LOCLIST   =  4
PAR_QHGDIR   =  8
PAR_REFLOC    = 16

def usage(appname):
    print(appname + "  - QDFSampler, make_pca & GridSampler")
    print("Usage:")
    print("  "+appname+" -i <indir> -Q <qhgdir> -g <grid>")
    print("              -l <loclist> -r <refloclist>")
    print("            [-s <speciesname>] [-n <maxstep>]")
    print("            [-o <output>] [-f <filebody>] [-e <eigenbindir>]")
    print("            [-b <numgenomesperbuf>]")
    print("where")
    print("  indir:        directory containing the QDF file to select from (no trailing '/'!)")
    print("  qhgdir:       path to th QHG3 directory (no trailing '/'!)")
    print("  grid:         QDF file of the correct size with geo info")
    print("  loclist:      location list specifying the sampling locations")
    print("  refloclist:   location list (1 entry) specifying the sampling location for the reference population")
    print("  speciesname:  species name (default: name of the first species found)")
    print("  maxstep:      step number of the file to qdf file to use (default: highest step number found)")
    print("  output:       output directory (default: last part of indir)")
    print("  filebody:     perefix ofr output files (default: output directory name)")
    print("  eigenbindir:  path to bin subdrectory of EigenSoft (no trailing '/'!)")
    print("  numgenomeperbuf: buffersize")
    print("")
    print("Format LocatoionList:")
    print("  LocList ::= LoLine*")
    print("  LocLine ::= <LocName>\"[\"<locID>\",\" <RegionID> \"]\" <Lon> <Lat> <Radius> <number>")
    print("  LocName:    name of location")
    print("  LocID:      id for location")
    print("  RegionID:   id for region (or group)")
    print("  Lon:        longitude of location")
    print("  Lat:        latitude of location")
    print("  Radius:     sampling radius around location")
    print("  Number:     Number of individuals to sample")
    print("")
#-- end def

iResult = 0

sInDir = ""
sGrid = ""
sQHGDir= ""
sLocList = ""
sRefLocList = ""
sSpeciesName = "Sapiens_ooa"
sNumSteps = ""
sOutput = ""
sFileBody = ""
sEigenBinDir = "/home/jody/EIG5.0.2/bin"
sNumGenomesPerBuf = "-1"
bQuiet = False

if (len(argv) > 5):
    iNeeded = PAR_INDIR | PAR_GRID | PAR_LOCLIST | PAR_QHGDIR | PAR_REFLOC
    i = 1
    while (iResult == 0) and (i < len(argv)):
        if   argv[i] == '-i':
            iNeeded = iNeeded & ~PAR_INDIR
            sInDir = argv[i+1]
            i = i+2
        elif argv[i] == '-g':
            iNeeded = iNeeded & ~PAR_GRID
            sGrid = argv[i+1]
            i = i+2
        elif argv[i] == '-l':
            iNeeded = iNeeded & ~PAR_LOCLIST
            sLocList = argv[i+1]
            i = i+2
        elif argv[i] == '-r':
            iNeeded = iNeeded & ~PAR_REFLOC
            sRefLocList = argv[i+1]
            i = i+2
        elif argv[i] == '-Q':
            iNeeded = iNeeded & ~PAR_QHGDIR
            sQHGDir = argv[i+1]
            i = i+2
        elif argv[i] == '-s':
            sSpeciesName = argv[i+1]
            i = i+2
        elif argv[i] == '-n':
            sNumSteps = argv[i+1]
            i = i+2
        elif argv[i] == '-o':
            sOutput = argv[i+1]
            i = i+2
        elif argv[i] == '-f':
            sFileBody = argv[i+1]
            i = i+2
        elif argv[i] == '-e':
            sEigenBinDir = argv[i+1]
            i = i+2
        elif argv[i] == '-b':
            sNumGenomesPerBuf = argv[i+1]
            i = i+2
        elif argv[i] == '-q':
            bQuiet = True
            i = i+1
        else:
            print("Unknown option ["+argv[i]+"]")
            iResult = -1
        #-- end if
    #-- end for

    if iNeeded == 0:
        print("sInDir:       [%s]" % (sInDir))
        print("sQHGDir:      [%s]" % (sQHGDir))
        print("sGrid:        [%s]" % (sGrid))
        print("sLocList:     [%s]" % (sLocList))
        print("sRefLocList:  [%s]" % (sRefLocList))
        print("sSpeciesName: [%s]" % (sSpeciesName))
        print("sEigenBinDir: [%s]" % (sEigenBinDir))
        #../genes/Loc_35.0_5.0
        

        #-- if the step nmber is not specified, use the highest one found
        if sNumSteps == "":
            a=glob.glob(sInDir+"/ooa_pop-"+sSpeciesName+"__*.qdf")
            a.sort()
            print(sInDir+"/ooa_pop-"+sSpeciesName+"__*.qdf"+" a:"+ str(a))
            sNumSteps=a[-1].split("__")[-1].replace(".qdf","")
        #-- end if
        print("sNumSteps:    [%s]" % (sNumSteps))
        sPopQDF = sInDir+"/ooa_pop-Sapiens_ooa__"+sNumSteps+".qdf"
        sMQDF = sInDir+"/ooa_M_"+sNumSteps+".qdf"
        sPop0QDF  = sInDir+"/ooa_pop-Sapiens_ooa__000000.qdf"
        sGeneDir  = sQHGDir + "/genes"
        sStuffDir = sQHGDir + "/useful_stuff"
        
        #-- ake sure the number of Genomes per buffer is actually numeric
        try:
            iNumGenomesPerBuf = int(sNumGenomesPerBuf)
        except:
            printf("NumGenomesPerBuf is not a number: [%s]" % (sNumGenomesPerBuf))
            iResult = -1
        else:
            print("sNumGenomesPerBuf: [%s]" % (sNumGenomesPerBuf))
        #-- end try

        #-- if no otput directory is specified, use the lat part of input dir
        if sOutput == "":
            sOutput = sInDir.split("/")[-1]
        #-- end if
        print("sOutput:      [%s]" % (sOutput))

        #-- create output directory
        try:
            os.mkdir(sOutput)
        except:
            pass
        #-- end try

        #-- remove trailing '/' from some directory names
        sOutput = re.sub("/$","",sOutput)
        sGeneDir = re.sub("/$","",sGeneDir)

        if sFileBody == "":
            sFileBody = sInDir.split("/")[-1]
        #-- end if
        
        if (iResult == 0):
            print("-----")
            print("QDFSampler for "+sLocList)
            arglist=[sGeneDir+"/QDFSampler", "-i", sPopQDF, "-o", sOutput+"/"+sFileBody, "-g", sGrid, "-f", "ped:asc:bin", "--location-file="+sLocList, "--attr-genome-size=Genetics_genome_size", "--genomes-per-buf="+str(iNumGenomesPerBuf)]
            if (bQuiet):
                arglist.append("-q")
            #-- end if
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist)
        #-- end if

        if (iResult == 0):
            print("-----")
            print("make_pca")
            arglist=["/usr/bin/python", sGeneDir+"/make_pca.py", "-d", sOutput, "-f", sFileBody, "-p", "1,2", "-e", sEigenBinDir]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist)
        #-- end if

        if (iResult == 0):
            print("-----")
            print("QDFSampler for "+sRefLocList)
            arglist=[sGeneDir+"/QDFSampler", "-i", sPopQDF, "-o", sOutput+"/"+sFileBody+"_orig", "-g", sGrid, "-f", "ped:asc:bin", "--location-file="+sRefLocList, "--attr-genome-size=Genetics_genome_size", "--genomes-per-buf="+str(iNumGenomesPerBuf)]
            if (bQuiet):
                arglist.append("-q")
            #-- end if
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist)
        #-- end if

        if (iResult == 0):
            print("-----")
            print("GeneDist ")
            arglist=[sGeneDir+"/GeneDist", "-g", sOutput+"/"+sFileBody+".bin", "-o", sOutput+"/"+sFileBody, "-m", sMQDF, "-G", sGrid, "-r",  sOutput+"/"+sFileBody+"_orig.bin"]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist)
        #-- end if

        fNull=open("/dev/null", "w")
        if (iResult == 0):
            print("-----")
            print("gnuplot ")
            arglist=["/usr/bin/gnuplot", sOutput+"/"+sFileBody+".gd.gpl"]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist, stderr=fNull)
        #-- end if

        if (iResult == 0):
            arglist=["/usr/bin/gnuplot", sOutput+"/"+sFileBody+".gd_2.gpl"]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist, stderr=fNull)
        #-- end if

        if (iResult == 0):
            print("-----")
            print("copy initial file ")
            arglist=["cp", sPop0QDF, sOutput+"/"+sFileBody+".init.qdf"]
            print("["+" ".join(arglist)+"]")
            iResult = subprocess.call(arglist, stderr=fNull)
        #-- end if

        if (iResult == 0):
            print("-----")
            print("initial attributes ")
            try:
                fOut = open(sOutput+"/"+sFileBody+".init.attrs", "w")
                arglist=["/usr/bin/python", sStuffDir+"/show_attributes.py", "all", sOutput+"/"+sFileBody+".init.qdf"]
                print("["+" ".join(arglist)+"]")
                iResult = subprocess.call(arglist, stdout=fOut, stderr=fNull)
                fOut.close()
            except:
                print("Couldn't open [%s/%s.init.attrs] for writing" % (sOutput, sFileBody))
                iResult = -1
            #-- end try
        #-- end if
        
        if (iResult == 0):
            print("+++ success +++")
        #-- end if
    else:
        print("Error: mandataory parameters missing: "+str(iNeeded))
    #-- end if
else:
    usage(argv[0])
#-- end if
