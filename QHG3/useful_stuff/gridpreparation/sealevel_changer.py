#!/usr/bin/python

from sys import argv, exit
import h5py
import numpy as np

#-----------------------------------------------------------------------------
#--
#--
class SeaLevelChanger:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, sealevelfile, t_col, h_col):
        sealeveldata = open(sealevelfile,'r')
        self.sealevel = sealeveldata.readlines()
        sealeveldata.close()
        self.t_col = t_col
        self.h_col = h_col
        self.sealevel = [ list(map(float,line.split())) for line in self.sealevel if len(line) > 1 and line[0] != '#' ]
    #-- end def

    #-------------------------------------------------------------------------
    #-- calcSeaLevel
    #--
    def calcSeaLevel(self, age):
        # find index for interpolation
        i = 0
        while self.sealevel[i][self.t_col] < age :
            i += 1
        #-- end while
        
        x1 = self.sealevel[i][self.t_col]
        x0 = self.sealevel[i-1][self.t_col]
        y1 = self.sealevel[i][self.h_col]
        y0 = self.sealevel[i-1][self.h_col]

        sealev = y0 + (age - x0) * (y1 - y0) / (x1 - x0)  # linear interpolation

        return sealev
    #-- end def


    #-------------------------------------------------------------------------
    #-- setSeaLevel
    #--
    def setSeaLevel(self, qdffile, age):
        iResult = -1

        sealev = self.calcSeaLevel(age)

        # print("setting relative sea level to %f in " % (sealev) + qdffile)
        try: 
            qdf = h5py.File(qdffile,'r+')
        except:
            print("Couldn't open["+qdffile+"] as QDF file")
        else:
            qdf.attrs['Time'] = - age * 1000.0   # time in years
            
            if qdf.__contains__('Geography'):
                geo = qdf['/Geography']

                if geo.attrs.__contains__('SeaLevel'):
                    geo.attrs['SeaLevel'] = np.array([sealev])

                    if geo.__contains__('Altitude'):
                        newalt = np.float64( np.array(geo['Altitude'][:]) - sealev )
                        del geo['Altitude']
                        geo.create_dataset('Altitude', newalt.shape, newalt.dtype, newalt)
                        iResult = 0
                    else:
                        print("No 'Altitude' in geo group of ["+qdffile+"]")
                    #-- end if
                else:
                    print("No 'SeaLevel' attribute in geo group of ["+qdffile+"]")
                #-- end if
            else:
                print("No 'Geography' group in ["+qdffile+"]")
            #-- end if
            qdf.flush()
            qdf.close()
        #-- end try
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- findLastTimeWithLevel
    #--
    def findLastTimeWithLevel(self, curLevel, maxt):
        tl = []
        tprev = 0
        lprev = 0
        iC = 0

        for l in self.sealevel:
            if l[0] <= maxt:
                #print("T[%f] L[%f] P[%f] " % (l[self.t_col], l[self.h_col],lprev))
                if ((lprev <= curLevel) and (curLevel < l[self.h_col])) or ((l[self.h_col] <= curLevel) and (curLevel < lprev)) :
                    #print("*")
                    tl.append((iC, l[self.t_col], l[self.h_col], tprev, lprev))
                #-- end if
                lprev=l[self.h_col]
                tprev=l[self.t_col]
                iC = iC+1
            else:
                break;
            #-- end if
        #-- end for
        print(str(tl))

        if len(tl) > 0:
            tll = tl[-1]
            t0 = (curLevel - tll[4])*(tll[1] - tll[3]) /(tll[2] -tll[4])+ tll[3]
        else:
            print("no level of %f found before %f; using level at %f" % (curLevel, maxt, maxt))
            t0 = maxt
        #-- end if
        
        #print("%f+(%f -%f)*(%f-%f)/(%f-%f)" % (tll[3], curLevel, tll[4], tll[1], tll[3], tll[2], tll[4]))
        return t0
    #-- end def
    
#-- end class


#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 4):
        try:
            t_col = int(argv[2])
            h_col = int(argv[3])
        except Exception,e:
            print("%s\n" % e)
            print("t_col and h_col must be integers")
        else:            
            print("ok")
            slc = SeaLevelChanger(argv[1], t_col,h_col)
            t0 = slc.findLastTimeWithLevel(float(argv[4]), 120)
            L=slc.calcSeaLevel(t0)
            print(str(t0)+":"+str(L))
        #-- end except
    else:
        print("Usage:")
        print("  %s <sealevelfile> <t_col> <h_col>" % argv[0])
        print("where")
        print("  sealevelfile    an ascii file containingrelativer sea levels")
        print("  t_col           column of time values")
        print("  h_col           column of selevel values")
    #-- end if
#-- end main
