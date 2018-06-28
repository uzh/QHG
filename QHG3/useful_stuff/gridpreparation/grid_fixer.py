#!/usr/bin/python

from sys import argv, exit
import h5py
import numpy as np

#-----------------------------------------------------------------------------
#--
#--
class GridFixer:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self):
        self.subdiv   = 0
        self.surftype = ""
        self.curfixes = {}
        self.NumCells = 0
    #-- end def


    #-------------------------------------------------------------------------
    #-- loadFixes
    #--
    def loadFixes(self, gridfixes):
        iResult = -1
        #print("Opening grid fix file ["+gridfixes+"]")
        try:
            gf = open(gridfixes, 'r')
        except:
            print("File ["+gridfixes+"] does not exist")
        else:
            iResult = 0
            iC = 0
        
            for line in gf:
                line = line.strip()
                if len(line) > 0:
                    if (iC == 0):
                        if (line.startswith("Grid:")):
                            desc = line.split(":")
                            if (len(desc)==2):
                                adesc = desc[1].strip().split()
                                if (((adesc[0].upper() == "IEQ") or (adesc[0].upper() == "ICO")) and adesc[1].isdigit()):
                                    self.surftype = adesc[0]
                                    self.subdiv   = int(adesc[1])
                                else:
                                    print("Bad grid description: surftype must be 'IEQ' or 'ICO',  subdiv must be an integer")
                                    iResult = -1;
                                    break
                                #-- end if
                            else:
                                print("Bad grid description (expected 'GRID: <sruftype> <subdiv>')")
                                iResult = -1;
                                break
                            #-- end if
                        else:
                            print("Expected header line:  'GRID: <sruftype> <subdiv>'")
                            iResult = -1;
                            break
                        #-- end if
                    else:
                        if (line[0] != '#'):
                            assignment = line.split()
                            if (len(assignment)==2):
                                if (assignment[0].isdigit()):
                                    
                                    try:
                                        f = float(assignment[1])
                                    except:
                                        print("Bad assignment: <newvalue> must be float")
                                        iResult = -1;
                                        break
                                    else:
                                        self.curfixes[assignment[0]] = float(assignment[1])
                                    #-- end try
                                    
                                else:
                                    print("Bad assignment: <cellid> must be integer ("+assignment[0]+")")
                                    iResult = -1;
                                    break
                                #-- end if
                            
                            else:
                                print("Expected assignment:  '<cellid> <newvalue>'")
                                iResult = -1;
                                break
                            #-- end if
                        #-- end if
                    #-- end if
                #-- end if
                iC = iC+1
            #-- end for
            gf.close()
        #-- end try
        return iResult
    #-- end def

    

    #-------------------------------------------------------------------------
    #-- applyFixes
    #--
    def applyFixes(self, qdffile, gridfixes):
        iResult = -1

        iResult = self.loadFixes(gridfixes)

        if (iResult == 0):
            
            #print("Opening qdf file ["+qdffile+"]")
            try: 
                qdf = h5py.File(qdffile,'r+')
            except:
                print("Couldn't open["+qdffile+"] as QDF file")
            else:
                if qdf.__contains__('Grid'):
                    gr = qdf['Grid']
                    if (gr.attrs.__contains__('SUBDIV') and gr.attrs.__contains__('SURF_TYPE') and gr.attrs.__contains__('NumCells')):
                        try:
                            iSubDiv = int(gr.attrs['SUBDIV'][0])
                            sSType  = str(gr.attrs['SURF_TYPE'][0])
                            iNumCells = int(gr.attrs['NumCells'])
                        except:
                            print("'SUBDIV' ("+str(gr.attrs['SUBDIV'])+") and 'NumCells' ("+str(gr.attrs['NumCells'])+") must be integers")
                            iResult = -1
                        else:
                    
                            if (iSubDiv == self.subdiv) and (sSType == self.surftype):
                                        
                                if qdf.__contains__('Geography'):
                                    geo = qdf['/Geography']
                                    if geo.__contains__('Altitude'):
                                        alt=geo['Altitude']
                                        for a in self.curfixes:
                                            if (int(a) < iNumCells):
                                                alt[int(a)] = self.curfixes[a]
                                            else:
                                                print("Id in ["+gridfixes+"] too large ("+str(a)+" >= "+str(iNumCells)+")")
                                                iResult = -1
                                                break
                                            #-- end if
                                        #-- end for

                                    else:
                                        print("'Geography' group does not contain 'Altitude' data set")
                                        iResult = -1
                                    #-- end if
                                else:
                                    print("No 'Geography' group in ["+qdffile+"]")
                                    iResult = -1
                                #-- end if
                            else:
                                print("Attribute mismatch:")
                                print("  QDF surftype: '"+sSType+"'; gridfix surftype: '"+self.surftype+"': "+str(sSType==self.surftype))
                                print("  QDF subdiv:   '"+str(iSubDiv)+"'; gridfix subdiv: '"+str(self.subdiv)+"': "+str(iSubDiv==self.subdiv))
                                iResult = -1
                            #-- end if
                        #-- end try
                    else:
                        print("'Grid' group does not contain all required attributes ('NumCells', 'SUBDIV' and 'SURF_TYPE')")
                        iResult = -1;
                    #-- end if
                else:
                    print("No 'Grid' group in ["+qdffile+"]")
                    iResult = -1;
                #-- end if

            #-- end if
            qdf.flush()
            qdf.close()
        #-- end try
        return iResult
    #-- end def
    
#-- end class
