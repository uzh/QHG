#!/usr/bin/python


from sys import argv, exit
import numpy as np
import h5py




class Node2Coords:
        
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qdffile):
        self.numcells   = 0
        self.longitudes = None
        self.latitudes  = None
        self.altitudes  = None
        self.ice        = None

        self.openArrays(qdffile)
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- openArrays
    #--
    def openArrays(self, qdffile):
        try: 
            qdf = h5py.File(qdffile,'r+')
        except:
            print("Couldn't open["+qdffile+"] as QDF file")
        else:
            if (qdf.__contains__('Grid')):
                grid = qdf['Grid']
                attrs = grid.attrs
                if (attrs.__contains__('NumCells')):
                    self.numcells = int(attrs['NumCells'][0])
                    print("numcells %d" % (self.numcells)) 
                    if qdf.__contains__('Geography'):
                        geo = qdf['/Geography']
                        if geo.__contains__('Longitude'):
                            self.longitudes=geo['Longitude']
                            print("Found %d longitude values: %f" % (len(self.longitudes), self.longitudes[0]))
                        else:
                            print("'Geography' group does not contain 'Longitude' data set")
                            iResult = -1
                        #-- end if
                
                        if geo.__contains__('Latitude'):
                            self.latitudes=geo['Latitude']
                            print("Found %d latitude values" % (len(self.latitudes)))
                        else:
                            print("'Geography' group does not contain 'Latitude' data set")
                            iResult = -1
                        #-- end if
                        if geo.__contains__('Altitude'):
                            self.altitudes=geo['Altitude']
                            print("Found %d altitude values" % (len(self.altitudes)))
                        else:
                            print("'Geography' group does not contain 'Altitude' data set")
                            iResult = -1
                        #-- end if
                        if geo.__contains__('IceCover'):
                            self.ice=geo['IceCover']
                            print("Found %d ice values" % (len(self.ice)))
                        else:
                            print("'Geography' group does not contain 'Altitude' data set")
                            iResult = -1
                        #-- end if
                    else:
                        print("No 'Geography' group in ["+qdffile+"]")
                        iResult = -1
                    #-- end if  
                else:
                    print("No 'NumCells' attribute in Grid group")
                    iResult = -1
                #-- end if

            else:
                print("No 'Grid' group in ["+qdffile+"]")
                iResult = -1
            #-- end if  
        #-- end if
    #-- end def

    
        
    #-------------------------------------------------------------------------
    #-- getValuesNice
    #--
    def getValuesNice(self, nodeid):
        if (nodeid < self.numcells):
            print("Node: %d" % (nodeid))
            self.longitudes[0]
            print("  Lon %f\n  Lat %f" % (self.longitudes[nodeid], self.latitudes[nodeid]))
            print("  Alt %f\n  Ice %f" % (self.altitudes[nodeid], self.ice[nodeid]))
        else:
            print("Node ID (%d)must  be less than %d" % (nodeid, self.numcells))
        #-- end if
    #-- end def

    #-------------------------------------------------------------------------
    #-- writeHeaderCSV
    #--
    def writeHeaderCSV(self):
        print("nodeID,longitude,latitude,altitude,ice")
    #-- end def
    
    #-------------------------------------------------------------------------
    #-- getValuesCSV
    #--
    def getValuesCSV(self, nodeid):
        if (nodeid < self.numcells):
            print("%d,%f,%f,%f,%d" % (nodeid,self.longitudes[nodeid], self.latitudes[nodeid],self.altitudes[nodeid], self.ice[nodeid]))
        else:
            print("Node ID (%d)must  be less than %d" % (nodeid, self.numcells))
        #-- end if
    #-- end def


if (len(argv) > 2):
    q=Node2Coords(argv[1])
    mode=argv[2]
    nodes = argv[3:]
    #    print(nodes)
    if (mode == "csv"):
        q.writeHeaderCSV()
        for node in nodes:
            q.getValuesCSV(int(node))
        #-- end for
    elif (mode == "nice"):
        for node in nodes:
            q.getValuesNice(int(node))
    else:
        print("unkniwn mode: [%s]" % (mode))
    #-- end if
else:
    print("%s - show qdf values" % (argv[0]))
    print("Usage")
    print("  %s <qdf-file> <mode> <nodeid>* " % (argv[0]))
    print("where")
    print("  qdf-file   a qdf file with Grid and Geography grtoup")
    print("  mode       \"csv\" or \"nice\"")
    print("  nodeids    node id")
    print("Example")
    print(" %s  navworld_085_kya_256.qdf 123 4565 7878 31112 0" % (argv[0]))
    print("")
#-- endmain
