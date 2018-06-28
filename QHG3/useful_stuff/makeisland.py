
from sys import argv
import h5py
import numpy as np

#-----------------------------------------------------------------------------
#--
#--
class IslandMaker:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdf_grid):
        try: 
            self.hqdf = h5py.File(qdf_grid,'r+')
        except:
            print("Couldn't open["+qdf_grid+"] as QDF file")
            exit(-1)
        else:
            if (self.hqdf.__contains__('Grid')):
                grid = self.hqdf['Grid']
                attrs = grid.attrs

                    
                if ((attrs.__contains__('NumCells')) and
                    (attrs.__contains__('W')) and
                    (attrs.__contains__('H'))):
                    self.w = int(attrs['W'][0])
                    self.h = int(attrs['H'][0])
                    self.numcells = int(attrs['NumCells'][0])
                    print("hooray: W %d, H %d, numcells %d" % (self.w, self.h, self.numcells))
                else:
                    print("grid does not have the attributes 'W' and 'H'")
                #-- end if
            else:
                print("qdf file does not have a 0Grid' group")
            #-- end if
        #-- end try
    #-- end def

    #-------------------------------------------------------------------------
    #-- addIslandGeo
    #--
    def addIslandGeo(self, wi, hi):
        iResult = -1
        print("addIslandGeo(%d,%d)" % (wi,hi))
        if (wi < self.w-1) and (hi < self.h-1):
            if (self.hqdf.__contains__('Geography')):
                geo = self.hqdf['Geography']
            else:
                geo = self.hqdf.create_group("Geography")
            #-- end if

            if ("Longitude" in geo):
                del geo['Longitude']
            #-- end if
            if ("Latitude" in geo):
                del geo['Latitude']
            #-- end if
            if ("Altitude" in geo):
                del geo['Altitude']
            #-- end if

        
            
            altitudes  = np.zeros(self.numcells)
            longitudes = np.zeros(self.numcells)
            latitudes  = np.zeros(self.numcells)
            offX = (self.w - wi)/2
            offY = (self.h - hi)/2
            for i in range(hi):
                for j in range(wi):
                    x = offX + j
                    y = offY + i
                    index = y * self.w + x
                    altitudes[index]  = 1
                #-- end for
            #-- end for

            for i in range(self.h):
                for j in range(self.w):
                    index = i * self.w + j
                    longitudes[index] = j
                    latitudes[index]  = i
                #-- end for
            #-- end for

            geo['Altitude']  = altitudes;
            geo['Longitude'] = longitudes;
            geo['Latitude']  = latitudes;
            
            self.hqdf.flush()
            self.hqdf.close()
        else:
            print("Island of size %dx%d does not fit into grid with size %dx%d" %(wi,hi,self.w,self.h))
        #-- end if

        return iResult
    #-- end def
    
#-- end class


if (len(argv) > 2):
    sGridQDF = argv[1]
    a=argv[2].split('x')
    if (len(a) == 2):
        im = IslandMaker(sGridQDF)
        iResult = im.addIslandGeo(int(a[0]), int(a[1]))
    else:
        print("Expected a size <W>x<H>, but got [%s]" % (argv[2]))
    #-- end if
else:
    print("%s - create a geography for an island" % (argv[0]))
    print("usage:")
    print("  %s <gridqdf> <W>x<H>" % (argv[0]))
    print("where")
    print("  gridqdf   a qdf file witha flat grid")
    print("  W         with of island to create")
    print("  H         height of island to create")
    print("If gridqdf contains a 'Geography' group, %s will delete the arrays 'Altitude','Latitude' and 'Longitude' if they exist." % (argv[0]))
    print("If gridqdf contains no 'Geography' group, a new one is created.")
    print("New arrays are created")
    print("  'Altitude':  altitude values corresponding to a centered island")
    print("  'Longitude': longitude values (actually the x-coordinates)")          
    print("  'Latitude':  latitude values (actually the Y-coordinates)")
    print("Finally, these arrays are added to the 'Geography' group and everything is saved")
#-- end if
