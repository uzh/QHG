#!/usr/bin/python

from sys import argv, exit

import os
import h5py
import numpy as np


class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class

MODE_NONE = 0
MODE_NODE = 1
MODE_POLY = 2

#-----------------------------------------------------------------------------
#--
#--
class LocListBuilder:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, sNodeQDF, sRegionNodes, iMode):
        self.fNodes = None
        self.RegionNodes = {}
        self.RegionPolys = {}
        self.iMode      = iMode
        try:
            self.loadNodeQDF(sNodeQDF)
            
            if (self.iMode == MODE_NODE):
                self.loadRegionNodes(sRegionNodes)
            elif  (self.iMode == MODE_POLY):
                self.loadRegionPolys(sRegionNodes)
            else:
                raise QHGError("Unknown mode [%d]" % (iMode))
            #-- end if                
        except QHGError as qhge:
            raise QHGError("Initialisation failed:\n"+qhge.message)
        else:
            print("Initialisation succeeded")
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- destructor
    #--
    def __del__(self):
        if not self.fNodes is None:
            self.fNodes.close()
        #-- end if
    #-- end def


    #-------------------------------------------------------------------------
    #-- loadRegionNodes
    #--
    def loadRegionNodes(self, regionnodefile):
        iResult = -1
        print("Opening region node file [%s]" % (regionnodefile))
        try:
            gf = open(regionnodefile, 'r')
        except:
            raise QHGError("File [%s] does not exist" % (regionnodefile))
        else:
            iResult = 0
            iC = 0
            sCurName = ""
            sCurNumber = 0
            curList=[]
            for line in gf:
                line = line.strip()
               
                if len(line) > 0:
                    if (line.startswith("Region")):
                        line =line[7:]
                        desc = line.split(":")
                        if (len(desc)==2):
                            if (desc[0].strip().isdigit()):
                                
                                if (sCurName != ""):
                                    self.RegionNodes[(int(sCurNumber), sCurName)]=curList
                                #-- end if
                                sCurNumber = desc[0].strip()
                                sCurName   = desc[1].strip()
                                curList=[]
                            else:
                                raise QHGError("regioonNumber must be an integer")
                            #-- end if
                        else:
                            raise QHGError("Bad grid description (expected 'REGION : <RegionNumber> <RegionName>')")
                        #-- end if
                    else:
                        if (sCurName != ""):
                            if (line[0] != '#'):
                                try:
                                    if (line.isdigit()):
                                        curList.append(int(line))
                                    else:
                                        raise QHGError("Expected node number, not [%s]" % (line))
                                    #-- end if
                                except:
                                    raise QHGError("Couldn't convert to int [%s]" %(line))
                                #-- end try
                            #-- end if
                        #-- end if
                    #-- end if
                #-- end if
                iC = iC+1
            #-- end for
            if (sCurName != ""):
                self.RegionNodes[(int(sCurNumber), sCurName)]=curList
            #-- end if
            gf.close()
        #-- end try
        return iResult
    #-- end def




    #-------------------------------------------------------------------------
    #-- loadRegionPolys
    #--
    def loadRegionPolys(self, regionpolyfile):
        iResult = -1
        print("Opening region poly file [%s]" % (regionpolyfile))
        try:
            gf = open(regionpolyfile, 'r')
        except:
            raise QHGError("File [%s] does not exist" % (regionpolyfile))
        else:
            iResult = 0
            iC = 0
            sCurName = ""
            sCurNumber = 0
            curList=[]
            for line in gf:
                line = line.strip()
               
                if len(line) > 0:
                    if (line.startswith("Region")):
                        line =line[7:]
                        desc = line.split(":")
                        if (len(desc)==2):
                            if (desc[0].strip().isdigit()):
                                
                                if (sCurName != ""):
                                    self.RegionPolys[(int(sCurNumber), sCurName)]=curList
                                #-- end if
                                sCurNumber = desc[0].strip()
                                sCurName   = desc[1].strip()
                        
                                curList=[]
                            else:
                                raise QHGError("regioonNumber must be an integer")
                            #-- end if
                        else:
                            raise QHGError("Bad grid description (expected 'REGION : <RegionNumber> <RegionName>')")
                        #-- end if
                    else:
                        if (sCurName != ""):
                            if (line[0] != '#'):
                                try:
                                    a = line.split()
                                    curList.append((float(a[0]), float(a[1])))
                                except:
                                    raise QHGError("Couldn't convert to float [%s]" % (line))
                                #-- end try
                            #-- end if
                        #-- end if
                    #-- end if
                #-- end if
                iC = iC+1
            #-- end for
            if (sCurName != ""):
                self.RegionPolys[(int(sCurNumber), sCurName)]=curList
            #-- end if
            gf.close()
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- loadNodeQDF
    #--
    def loadNodeQDF(self, sNodeQDF):
        
        try:
            fTestNodes = h5py.File(sNodeQDF, "r")
            if (fTestNodes.__contains__('Grid')) and fTestNodes.__contains__('Geography'):
                if (fTestNodes['Grid'].__contains__('CellDataSet')) and \
                (fTestNodes['Geography'].__contains__('Longitude')) and \
                (fTestNodes['Geography'].__contains__('Altitude')):
                    print("OK")
                    self.fNodes = fTestNodes
                else:
                    raise QHGError("Group 'Grid' needs 'CellDataSet', group 'Geography' needs 'Longitude', 'Latitude', and 'Altitude'")
                #-- end if
            else:
                raise QHGError("The QDF file [%s] needs groups 'Geography' and Grid'" % (sNodeQDF))
            #-- end if
        except Exception as e:
            raise QHGError("Couldn't open [%s] as QDF file:\n%s" % (sNodeQDF, e))
        #-- end def
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- writeLocListNode
    #--
    def writeLocListNode(self, sOutput, sRadius, sNumSamples) :
        try:
            fOut = open(sOutput, "w")
            gGrid = self.fNodes['Grid'];
            gGeo  = self.fNodes['Geography'];
            iC = 0
            for i in range(gGrid.attrs['NumCells']):
                for r in self.RegionNodes:
                    if  i in self.RegionNodes[r]:
                        
                        fOut.write("%s[%d,%d]   %7.3f  %7.3f  %s %s\n" % (r[1], iC, r[0], gGeo['Longitude'][i], gGeo['Latitude'][i], sRadius, sNumSamples))
                        iC = iC+1
                        break
                    #-- end if
                #-- end if
            fOut.close()
        except Exception as e:
            print("Couldn't open [%s] for writing: %s" % (sOutput, e))
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- isPointInPoly
    #--
    def isPointInPoly(self, key, testx, testy):
  
        bInside = False
        nvert = len(self.RegionPolys[key])
  
        lastx =  self.RegionPolys[key][-1][0]
        lasty =  self.RegionPolys[key][-1][1]
  
        for i in range(nvert):
            curx =  self.RegionPolys[key][i][0]
            cury =  self.RegionPolys[key][i][1]

            if (curx != lastx) and (cury != lasty):
                if ( ((cury>testy) != (lasty>testy)) and
                     (testx < (lastx-curx) * (testy-cury) / (lasty - cury) + curx )):
                    bInside =  not bInside
                #-- end if
            #-- end if    
            
            lastx = curx
            lasty = cury
        #-- end for
        return bInside
    #-- end def


    #-------------------------------------------------------------------------
    #-- writeLocListPoly
    #--
    def writeLocListPoly(self, sOutput, sRadius, sNumSamples) :
        try:
            fOut = open(sOutput, "w")
            gGrid = self.fNodes['Grid'];
            gGeo  = self.fNodes['Geography'];
            iC = 0
            for node in gGrid['CellDataSet']:
                iNode = node[0]
                iLon0 = gGeo['Longitude'][iNode]
                iLat0 = gGeo['Latitude'][iNode]
                for p in self.RegionPolys:
                    bInside = self.isPointInPoly(p, iLon0, iLat0)
                    if (bInside):
                        fOut.write("%s[%d,%d]   %7.3f  %7.3f  %s %s\n" % (p[1], iC, p[0], iLon0, iLat0, sRadius, sNumSamples))
                        iC = iC+1
                        if (iLon0 < -144) and (iLat0 > 57):
                            print("Point [%7.3f,%7.3f]: used by %s" % (iLon0, iLat0, p[1]))
                        #-- end if
                        break
                    else:
                        if (iLon0 < -144) and (iLat0 > 57):
                            print("Point [%7.3f,%7.3f]: not used" % (iLon0, iLat0))
                        #-- end if

                    #-- end if
                #-- end for
            
            fOut.close()
        except Exception as e:
            print("Couldn't open [%s] for writing: %s" % (sOutput, e))
        #-- end try
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- writeLocList
    #--
    def writeLocList(self, sOutput, sRadius, sNumSamples) :
        if (self.iMode == MODE_NODE):
             self.writeLocListNode(sOutput, sRadius, sNumSamples)
        elif (self.iMode == MODE_POLY):
             self.writeLocListPoly(sOutput, sRadius, sNumSamples)
        #-- end if
    #--- end def

    
    #-------------------------------------------------------------------------
    #-- showRegionNodes
    #--
    def showRegionNodes(self):
        u = self.RegionNodes.keys();
        u.sort()
        for r in u:
            print("%d (%d): %d" % (r[0], r[1], len(self.RegionNodes[r])))
        #-- end for
    #-- end def
        
#-- end class


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 6):
        sSwitch      = argv[1]
        sRegionFile  = argv[2]
        sNodeQDF     = argv[3]
        sRadius      = argv[4]
        sNumSamples  = argv[5]
        sOutput      = argv[6]

        iMode = MODE_NONE
        if (sSwitch == "-n"):
            iMode = MODE_NODE
        elif (sSwitch == "-p"): 
            iMode = MODE_POLY
        #-- end if
        
        try:
            lbb = LocListBuilder(sNodeQDF, sRegionFile, iMode)
            lbb.writeLocList(sOutput, sRadius, sNumSamples)    
        except QHGError as qhge:
            print("Error: ["+qhge.message+"]")
        else:
            print("+++ success +++")
        #-- end try
    else:
        print(argv[0]+" - create a LocationList from a grid and a region node file")
        print("usage: "+argv[0]+" (-n <RegionNodeFile> | -p <RegionPolyFile>) <QDFNodeFile>  <radius> <NumSamples> <output>")
        print("Format RegionNodeFile:")
        print("  RegionNodeFile ::= <RegionData>*")
        print("  RegionData     ::= <RegionHeader> <NL> <RegionNodes>*")
        print("  RegionHeader   ::= \" Region \" <RegionNumber>\":\"<RegionName>")
        print("  RegionNodes    ::= <NodeID> <NL>")
        print("  RegionNumber   : integer (ID for region)")
        print("  RegionName     : string (name of region)")
        print("  NodeID         : integer (id of node to select)")
        
    #-- end if
#-- end if
