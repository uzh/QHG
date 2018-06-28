#!/usr/bin/python

from sys import argv
import random as rnd
import h5py
from shutil import copyfile

MAX_TRIES = 100

#-----------------------------------------------------------------------------
#--  getAttrs
#--    get attributes for a specific population
#--    if no population name is given, the first population is used
#--
def getAttrs(hf, pop_name):
    attr = None
    if 'Populations' in list(hf.keys()):
        gp= hf['Populations']
        if (len(list(gp.keys())) > 0):
            if pop_name == '':
                attr=gp[list(gp.keys())[0]].attrs
            else:
                if pop_name in list(gp.keys()):
                    attr=gp[pop_name].attrs
                else:
                    print("No population ["+pop_name+"] found")
            #-- end if        
        else:
            print("No population subgroups found")
        #-- end if
    else:
        print("No group ['Populations'] found")
    #-- end if
    return attr
#-- end def


#-----------------------------------------------------------------------------
#--
#--
class PopMutator:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qhgfile, popname, jobdescfile, completedjobsfile):
        self.qhgfile           = qhgfile
        self.popname           = popname
        self.jobdescfile       = jobdescfile
        self.abbrevs           = {}
        self.completedjobs     = {}
        self.jobdesc           = {}
        self.completedjobsfile = completedjobsfile
        self.readJobDesc()
        self.readCompletedJobs()
        self.curvals = []
    #-- end def

    #-------------------------------------------------------------------------
    #-- readJobDesc
    #--
    def readJobDesc(self):
        iResult = 0
        f = open(self.jobdescfile, 'r')
        
        self.names = []
        iC = 0
        
        for line in f:
            # read first line for names
            line = line.strip()
            if (len(line) > 0) and (line[0] != '#'):
                vals = line.split()
                if len(vals) == 4:
                    nameparts = vals[0].split(":")
                    if (len(nameparts) > 1):
                        self.jobdesc[nameparts[0]] = [float(vals[1]),float(vals[2]),float(vals[3])]
                        self.abbrevs[nameparts[0]] = nameparts[1]
                    else:
                        print("the name of the attribute should be followed by ':' and an abbreviation")
                        iResult = -1
                        break
                else:
                    print("Expected 4 items: <name>':'<abbrev> <minval> <maxval> <step>: ["+line+"]")
                    iResult = -1
                    break
                #-- end if
            #-- end if
            iC = iC+1
        #-- end for
        f.close()
        self.names = sorted(list(self.jobdesc.keys()))
     
        print(str(self.jobdesc))
        return iResult
    #-- end def



    #-------------------------------------------------------------------------
    #-- readCompletedJobs
    #--
    def readCompletedJobs(self):
        iResult = -1
        try:
            f = open(self.completedjobsfile, 'r')
        except:
            print("File ["+self.completedjobsfile+"] does not exist yet")
        else:
            iResult = 0
            curnames = []
            iC = 0
        
            for line in f:
                line = line.strip()
                if len(line) > 0:
                
                    # read first line for names
                    if (iC == 0) and (line[0] == "#") and (len(curnames) == 0):
                        curnames = line[1:].split()
                    else:
                        if (set(self.names) != set(curnames)):
                            iResult = -1
                            print("Names in ["+self.completedjobsfile+"] ("+str(curnames)+") do not match names in jobdesc ("+str(self.names)+")")
                            break
                        else:
                            if (line[0] != '#'):
                                vals = line.split()
                                if len(vals) == len(curnames)+1:#len(self.names):
                                    self.completedjobs[tuple(vals[0:-1])] = vals[-1]
                                else:
                                    print("Expected "+str(len(self.names))+" values, but got "+str(len(vals))+"\n  ["+line+"]")
                                #-- end if
                            #-- end if
                        #-- end if    
                    #-- end if
                #-- end if
                iC = iC+1
            #-- end for
            f.close()
        #-- end try
    
        print("read "+str(len(self.completedjobs))+" entries from "+self.completedjobsfile)
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- writeCompletedJobs
    #--
    def writeCompletedJobs(self):
        f = open(self.completedjobsfile, 'w')

        s=""
        for i in range(len(self.names)):
            s = s + str(self.names[i-1])+" "
        #-- end for
        f.write("#"+s+"\n")

        for x in sorted(self.completedjobs):
            s=""
            for i in range(len(x)):
                s = s + str(x[i-1])+" "
            #-- end for
            f.write(s+str(self.completedjobs[x])+"\n")
        #-- end for
        f.close()
    #-- end def


    #-------------------------------------------------------------------------
    #-- initialiseVals
    #--
    def initialiseVals(self):
        self.curvals = [self.jobdesc[i][0] for i in self.jobdesc]
    #-- end def

    

    #-------------------------------------------------------------------------
    #-- doRandomChoices
    #--  a generator!
    #--
    def doRandomChoices(self, count):
        i = 0
        while i < count:
            bSearching = True;
            iTried = 0
            while bSearching:
                point = []
                for e in self.jobdesc:
                    min  = self.jobdesc[e][0]
                    max  = self.jobdesc[e][1]
                    step = self.jobdesc[e][2]
                    if step > 0:
                        k = int((max-min)/step)
                        if  k < 1:
                            k = 1
                        #-- end if
                        point.append(min+step*rnd.randrange(k))
                    else:
                        point.append(rnd.uniform(min, max))
                    #-- end if
                #-- end for
                try:
                    n = self.completedjobs[tuple(point)]
                except:
                    bSearching = False
                else:
                    iTried = iTried + 1
                    print("point "+str(point)+" appeared already")
                    if (iTried > MAX_TRIES):
                        bSearching = False
                        point = None
                    #-- end if
                #-- end try
            #-- end while
            print("cur point: "+str(point))
            yield point
            i = i+1
            '''
            '''
        #-- end for
    #-- end def

  

    #-------------------------------------------------------------------------
    #-- updateCompleted
    #--
    def updateCompleted(self, curpoint):
        n = 0
        try:
            n = self.completedjobs[tuple(curpoint)]
        except:
            pass
        #-- end try
        self.completedjobs[tuple(curpoint)] = n+1

    #-- end def


    #-------------------------------------------------------------------------
    #-- verifyAttributes
    #--
    def verifyAttributes(self):
        iResult = 0
        print("verifing ["+self.qhgfile+"]")
        try:
            q = h5py.File(self.qhgfile, "r")
        except:
            print("couldn't open ["+self.qhgfile+"] as QDF file")
            iResult = -1
        else:
            attr = getAttrs(q, self.popname)
            if (not attr is None):
                for n in self.names:
                    if not n in attr:
                        print("attributes do not contain ["+n+"]")
                        iResult = -1
                    #-- end if
                #-- end for
            else:
                print("Didn't find attributes")
            #-- end if
            q.close()
        #-- end try
        return iResult
    #-- end def
    

    #-------------------------------------------------------------------------
    #-- applyData
    #--
    def applyData(self, curpoint, prefix):
        # create a name
        i = 0
        newname = prefix
        for d in curpoint:
            newname = newname + "_" + self.abbrevs[list(self.abbrevs.keys())[i]] + str(d)
            i = i+1
        #-- end for
        if tuple(curpoint) in self.completedjobs:
            v = self.completedjobs[tuple(curpoint)]
        else:
            v = 0
        #-- end if
        sv = "%03d" % v
        
        newname = newname + "_v"+sv+".qdf"
        
        print("output ["+newname+"]")
        copyfile(self.qhgfile, newname)
        q= h5py.File(newname, "r+")
        attr = getAttrs(q, self.popname)
        
        i = 0
        for d in curpoint:
            attname = list(self.jobdesc.keys())[i]
            attr[attname] = d
            i = i+1
            print("Setting attr ["+attname+"] to ["+str(d)+"]")
        #-- end for
        q.flush()
        q.close()
    #-- end def

#-- main

if len(argv) > 6:
    sQDFFile = argv[1]
    sPopName = argv[2]
    sJobFile = argv[3]
    sRepeats = argv[4]
    sDone    = argv[5]
    sPrefix  = argv[6]
  
    s = PopMutator(sQDFFile, sPopName, sJobFile, sDone)
    print("names:["+str(s.names)+"]")
    iResult = s.verifyAttributes()
    if (iResult == 0):
        iRepeats = int(sRepeats)
        s.initialiseVals()
        for curpoint in s.doRandomChoices(iRepeats):
            if not curpoint is None:
                s.applyData(curpoint, sPrefix)
            
                s.updateCompleted(curpoint)
            else:
                print("Couldn't find different points")
                break
            #-- end if
        #-- end for

        s.writeCompletedJobs()
    #-- end if
else:
    print(argv[0]+" - changes a population's attributes according to a job description")
    print("usage:")
    print("  "+argv[0]+" <qdffile> <popname> <jobfile> <repeats> <donefile> <prefix>")
    print("where")
    print("  qdffile    name of source qdf file")
    print("  popname    name of population (if '', the first population is used")
    print("  jobfile    file containing job description")
    print("  donefile   file containing records of previous jobs")
    print("  prefix     prefix for output file names")
#-- end if
    
