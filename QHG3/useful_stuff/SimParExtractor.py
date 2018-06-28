from sys import argv, exit
from QHGError import QHGError

#-----------------------------------------------------------------------------
#--
#--
class SimParExtractor:

    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, attr_exclude=None, arr_keys=None, order=None):
        if (attr_exclude is None):
            self.attr_exclude = ["AltCapPref",
                                 "InitialSeed",
                                 "NPPCap_veg_selection",
                                 "NPPPref", "PrioInfo"]
        else:
            self.attr_exclude = attr_exclude
        #-- end if

        if (arr_keys is None):
            self.arr_keys = ["Australia[221,8]",
                             "North_West_America[138,12]",
                             "South_America_South[273,17]"]
        else:
            self.arr_keys = arr_keys
        #-- end if

        if (order is None):
            self.order = ["ClassName",
                          "SpeciesName",
                          "NumCells",
                          "WeightedMoveProb",
                          "Verhulst",
                          "Genetics",
                          "Fertility",
                          "NPPCap",
                          "Navigate",
                          "OAD"]
        else:
            self.order = order
        #- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #--  getAttrs
    #--    get attributes from attribute text file
    #--    expected format:
    #--       <name> "[" <value> "]"
    #--    
    def getAttrs(self, attr_file):
        attrs = {}
        try:

            fattrs = open(attr_file, "r")
            data   = fattrs.readlines()

            for line in data:
                line = line.replace("[","").replace("]","").strip()
                words = line.split()
                if (not words[0] in self.attr_exclude):
                    if (len(words) == 2):
                        attrs[words[0]] = words[1]
                    else:
                        raise QHGError("Bad line format [%s]; len %d" % (line, len(words))) 
                        attrs = {}
                    #-- end if
                #-- end if
            #-- end for
            fattrs.close()
        except QHGError as qhge:
            raise
        except Exception as e:
            raise QHGError("Error opening attribute file [%s]: %s" % (attr_file,e.message))
        
        #-- end try
        return attrs
    #-- end def


    #-----------------------------------------------------------------------------
    #--  getSomeArrivals
    #--    get some arrival data
    #-- 
    def getSomeArrivals(self, arr_file):
        
        arrivals = {}
        for key in self.arr_keys:
            arrivals[key] = "-1"
        #-- end for    
        try:
            farr = open(arr_file, "r")
            data   = farr.readlines()

            for line in data:
                arr_parts = line.split(':')
                arr_loc = arr_parts[0].split()[0]
                arr_val = arr_parts[1].split()[1]
                if arr_loc in self.arr_keys:
                    arrivals[arr_loc] = arr_val
                #-- end if
            #-- end for
            farr.close()
        except Exception as e:
            print("Error opening arrival file [%s]: %s" % (arr_file, e.message))
#            raise QHGError("Error opening arrival file [%s]: %s" % (arr_file, e.message))
        #-- end try
        return arrivals
    #-- end def
    

    #-----------------------------------------------------------------------------
    #--  extractSingle
    #--   
    #-- 
    def extractSingle(self, attr_file, arr_file):
        attrs = self.getAttrs(attr_file)
        arrs  = self.getSomeArrivals(arr_file)
        
        outline = [];
        outkeys = [];
        for o in self.order:
            for a in sorted(attrs):
                if a.find(o) == 0:
                    outkeys.append(a)
                    outline.append(attrs[a])
                #-- end if
            #-- end for
        #-- end for
        for a in arrs:
            outkeys.append(a)
            outline.append(arrs[a])
        #-- end for
        return outkeys, outline
    #-- end def
    
#-- end class

if __name__ == '__main__':
    if (len(argv) > 1):
        iResult = 0
        analysis_dir = argv[1]
        attr_file = analysis_dir + "/analysis.attr"
        arr_file  = analysis_dir + "/analysis.arr"
        
        if True:
            extractor = SimParExtractor("oink")
            outkeys, outlist = extractor.extractSingle(attr_file, arr_file)
            for o in outlist:
                print("%s" % o)
            #-- end for
        #-- end if
    #-- end if
#-- end if
