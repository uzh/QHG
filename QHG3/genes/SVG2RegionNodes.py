#!/usr/bin/python

from sys import argv
import string    
import xml.etree.ElementTree as ET

#-----------------------------------------------------------------------------
#--
#--
class SVGPathReader:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, sSVGFile, fMergeRadius):
        self.sSVGFile = sSVGFile
        self.fMergeRadius2 = fMergeRadius*fMergeRadius
        self.paths = {}
        self.unique_points = {}
        self.scalex = 1.0
        self.scaleY = 1.0
        self.img_h  = 1.0
        self.findUniquePoints()
    #-- end def


    #-------------------------------------------------------------------------
    #-- translateToAbsolutePath
    #--
    def translateToAbsolutePath(self, sPath):
        aPath = sPath.split()
        p0 = [0,0]
        sOut = ''
        bAbs = True
        bClose = False
        #print("[%s]" % (aPath))
        for x in aPath:
            #print("-- %s" % x)
            if x == 'M':
                bAbs = True
            elif x =='m':
                bAbs = False
            elif x =='L':
                bAbs = True
            elif x == 'l':
                bAbs = False
            elif x == 'z':
                bClose = True
            else:
                pt = x.split(',')
                pt = [float(pt[0]), float(pt[1])]
                if bAbs:
                    p0 = pt
                else:
                    p0[0] += pt[0]
                    p0[1] += pt[1]
                #-- end if
                sOut = sOut +  "%d,%d " % (round(p0[0]), round(p0[1]))
            #-- end if
        #-- end for
        if (bClose):
            sOut += 'z'
        #-- end if
       
        return sOut

    #-- end def
    


    #-------------------------------------------------------------------------
    #-- findUniquePoints
    #--
    def findUniquePoints(self):
        self.paths = {}

        try:
            tree = ET.parse(self.sSVGFile)
            root=tree.getroot()
            ns = '{http://www.w3.org/2000/svg}'
            timg = root.find(ns+'image')
            self.scalex = 1.0/float(timg.attrib['width'])
            self.img_h = float(timg.attrib['height'])
            self.scaley = 1.0/self.img_h
            for child in root:
                if child.tag.endswith('path'):
                    sPath = self.translateToAbsolutePath(child.attrib['d'])
                    self.paths[child.attrib['id']] = sPath
                #-- end if
            #-- end for
        except Exception as e:
            print("Problem reading %s: %s" % (self.sSVGFile, e))
        else:
        
            all_coords = []
            for x in self.paths:
            
                ap = self.paths[x].split()
                for p in ap:
                    if (p != 'z'):
                        all_coords.append(p)
                    #-- end if
                #-- end for
            #-- end for
        
            self.unique_points = {}
        
            b = sorted(all_coords)
            for i in range(len(b)-1):
                if not  self.unique_points.__contains__(all_coords[i]):
                    #print("ac: "+ all_coords[i])
                    c0 = all_coords[i].split(',')
                    #print("c0: "+str(c0))
                    x0=int(c0[0])
                    y0=int(c0[1])
                    for j in range(i+1, len(b)):
                        c1 = all_coords[j].split(',')
                        x1=int(c1[0])
                        y1=int(c1[1])
                    
                        if ((x0 - x1)*(x0 - x1)+(y0 - y1)*(y0 -y1)) < self.fMergeRadius2:
                            if (all_coords[j] != all_coords[i]):
                                self.unique_points[all_coords[j]] = all_coords[i]
                                #print("t "+str(j)+":"+ all_coords[j]+"-->"+str(i)+":"+all_coords[i])
                            #-- end if
                        #-- end if
                    #-- end for
                #-- end if
            #-- end for
            print("Found %d doubles" % (len(self.unique_points)))

            
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- makeSVGAbsolute
    #--
    def makeSVGAbsolute(self, sOutput):
        fIn = open(self.sSVGFile, "r")
        fOut = open(sOutput, "w")
        iC = 0
        for line in fIn:
            if (line.find(" d=") >= 0):
                line = line.replace('"', '')
                #print("Have d-line ["+line+"]")
                k = line.split('=')
                sOut = k[0] + '="M '

                a = k[1].split()
               
                # merge close points
                d = self.translateToAbsolutePath(k[1]);
                for c in self.unique_points:
                    if (d.find(c) >= 0):
                        d=d.replace(c, self.unique_points[c])
                        iC = iC + 1
                    #-- end if
                #-- end for

                # remove repetitions
                dRed = ""
                aa = d.split()
                cPrev = aa[-2]
                for x in aa:
                    if (x == 'z'):
                        dRed = dRed + x
                    elif (x != 'z') and (x!= cPrev):
                        dRed = dRed + x + " "
                        cPrev = x
                    #-- end if
                #-- end for

                # write line
                sOut = sOut + dRed + '"'
                fOut.write(sOut+"\n")
            else:
                fOut.write(line)
            #-- end if
        #-- end for
        fIn.close()
        fOut.close()
        print("Performed %d changes to [%s] -> [%s]" %(iC, self.sSVGFile, sOutput))
    #-- end def


    #-------------------------------------------------------------------------
    #-- makeRegionPolyFile
    #-- 
    def makeRegionPolyFile(self, sOutput):
        try:
            fOut = open(sOutput, "w")
        except:
            print("Couldn't open [%s]" % (sOutput))
        else:
            iC = 0
            for p in self.paths:
                fOut.write("Region %d:%s\n" % (iC, p))
            
                d = self.paths[p].replace(" z", "")
                for c in self.unique_points:
                    if (d.find(c) >= 0):
                        d=d.replace(c, self.unique_points[c])
                    #-- end if
                #-- end for    
                a = d.split()
                for x in a:
                    v = x.split(',')
                    lon = 360.0*float(v[0])*self.scalex - 180.0
                    lat = 180*(self.img_h - float(v[1]))*self.scaley - 90.0
                    fOut.write(" %7.3f %7.3f\n" % (lon, lat))
                    iC = iC+1
                #-- end for
                fOut.write("\n")
            #-- end for
            print("Output written to %s" % sOutput)
            fOut.close()
            print("+++ success +++")
        #-- end try
    #-- end def
    
#-- end class


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if len(argv) > 4:
        try:
            fMRadius = float(argv[3])
            spr = SVGPathReader(argv[2], fMRadius)
            if (argv[1] == "-svg"):
                spr.makeSVGAbsolute(argv[4])
            else:
                spr.makeRegionPolyFile(argv[4])
            #-- end if
        except Exception as e:
            print("Expected float for MergeRadius, not [%s]\n%s" % (argv[2], e))
        #-- end try
    else:
        print("Usage:")
        print("   %s (-reg | -svg) <SVGFile> <MergeRadius> <OutputName>", (argv[0]))
     
    #-- end if
#--  end main
    
