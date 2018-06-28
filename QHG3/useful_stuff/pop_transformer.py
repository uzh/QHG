#!/usr/bin/python

import numpy as np
import h5py
from sys import argv

#-----------------------------------------------------------------------------
#--
#--
class PopTransformer:

    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdffile, popname):
        self.qdffile  = qdffile
        self.popname  = popname
        self.defaults = {}
    #-- end def



    #-------------------------------------------------------------------------
    #-- openpop
    #--
    def openpop(self):
        iResult = -1
        self.qdf = None
        self.pop = None
        try:
            self.qdf = h5py.File(self.qdffile,'r+')
            self.pop = self.qdf['Populations'][self.popname]
            iResult = 0
        except:
            print("Couldn't open ["+self.qdffile+"] or couldn't open suubgroup ['"+self.popname+"'] of ['Populations']")
        #-- end try
        return iResult
    #-- end def

    #-------------------------------------------------------------------------
    #-- term
    #--
    def term(self):
        self.qdf.flush()
        self.qdf.close()
    #-- end def


    #-------------------------------------------------------------------------
    #-- readDefaults
    #--
    def readDefaults(self, def_file):
        iResult = 0
        try:
            f=open(def_file, "r")
            for line in f:
                if line[0] != '#':
                    aha = line.split
                    if (len(aha) == 3):
                        if (aha[2] == "int16"):
                            dt = np.int16
                        elif (aha[2] == "int32"):
                            dt = np.int32
                        elif (aha[2] == "int64"):
                            dt = np.int64
                        elif (aha[2] == "float32"):
                            dt = np.float32
                        elif (aha[2] == "float64"):
                            dt = np.float64
                        else:
                            dt = None
                        #-- end if

                        try:
                            if dt is None:
                                self.defaults[aha[0]] = [aha[1]]
                            else:
                                self.defaults[aha[0]] = np.array([aha[1]], dtype=dt)
                            #-- end if
                        except:
                            print("Couldn't create entry for ["+line+"]")
                            iResult = -1
                            break
                        #-- end try
                    else:
                        print("Line should be '<name> <value> <type>' but is ["+line+"]")
                        iResult = -1
                        break;
                    #-- end if
                #-- end if
            #-- end for
        except:
            print("Couldn't open or read ["+def_file+"]")
            iResult = -1
        #-- end try
        return iResult
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- transform
    #--
    def transform(self, new_class_name, new_class_id):
        
        if (not self.pop is None):
            t0 = np.float(self.qdf.attrs['Time'][0])

            # change class name and id
            del self.pop.attrs['ClassName']
            self.pop.attrs['ClassName'] = [new_class_name]
            self.pop.attrs['ClassID'] = [np.int64(new_class_id)]

            #-- remove any ConfinedMove attributes
            try:
                del self.pop.attrs['ConfinedMoveX']
                del self.pop.attrs['ConfinedMoveY']
                del self.pop.attrs['ConfinedMoveR']
            except:
                priint("No ConfinedMove attributes found")
            #-- end try

            # in prios:  change Verhust to VerhulstVarK and remove ConfinedMove
            prios = self.pop.attrs['PrioInfo']
            del self.pop.attrs['PrioInfo']
            x=None
            for pp in prios:
                if (pp[0] == 'Verhulst'):
                    pp[0] = 'VerhulstVarK'
                else:
                    if (pp[0] == 'ConfinedMove'):
                        x = np.where(prios == pp)
                    #-- end if
                #-- end if
            #-- end for
            if (not x is None):
                prios = np.delete(prios, x)
            #-- end if
            self.pop.attrs['PrioInfo'] = prios


            # loop through all agents and adjust birthtime.
            # in an agent struct the birthtime is at index 4
            ads = self.pop['AgentDataSet']
            for i in range(len(ads)):
                z = ads[i]
                z[4] = z[4]-t0
                ads[i] = z
            #-- end for

            for n in defaults:
                self.pop.attrs[n] = [defaults[n]]
        else:
            print("population ["+self.popname+"] is not open")
        #-- end if
    #-- end def


    
 

if len(argv) > 4:
    pt = PopTransformer(argv[1],argv[2])
    pt.openpop()
    pt.readDefaults(argv[5])
    pt.transform(argv[3], argv[4])
    pt.term()
else:
    print(argv[0]+" - transforms a breeding population to an initial population")
    print("           with a new class name and class id")
    print("           The transformations:")
    print("           - replace current class name with new class name")
    print("           - replace current class id with new class id")
    print("           - remove all ConfinedMoveXYZ attributes")
    print("           - change 'Verhulst' to 'VerhulstVarK' in prio list")
    print("           - remove ConfinedMove from prio list")
    print("           - change every agent's birthtime to bt_new = bt_old - t0")
    print("             where t0 is the time stamp of the qdf file")
    print("     ATTENTION: These change are made in-file!")      
    print("usage:")
    print("  "+argv[0]+" <qdffile> <popname> <newclassname> <newclassid> <defaults>")
    print("where")
    print("  qdffile        qdf file to modify")
    print("  popname        name of population to modify")
    print("  newclassname   new class name") 
    print("  newclassid     new class id") 
    print("  defaults       text file containing line of the form")
    print("                    <name> <value> <type>")
    print("  where <value> is int16, int32, int64, float32, float64 or str") 
#-- end if
