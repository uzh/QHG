#!/usr/bin/python

from sys import argv, exit, stdout
from SimParExtractor import SimParExtractor
import glob
import os
from QHGError import QHGError



                
#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 1):
        iResult = 0
        dir_pat = argv[1]

        if (len(argv) > 2):
            fout=open(argv[2], 'w')
        else:
            fout = stdout
        #-- end if
        
        separators = {"spc":' ', "semi":';', "comma":',', "tab":'\t', "col":':'}
        sep = separators["semi"]
        allkeys=[]
        bFirst = True
        try:
            extractor = SimParExtractor()
            candidates = glob.glob(dir_pat)
            for sim_dir in candidates:
                if (os.path.isdir(sim_dir)):
                    attr_file = sim_dir + "/analysis.attr"
                    arr_file  = sim_dir + "/analysis.arr"

                    if (os.path.exists(attr_file) and os.path.exists(arr_file)):
                        outkeys, outlist = extractor.extractSingle(attr_file, arr_file)
                        if (bFirst):
                            allkeys = outkeys
                            allkeys.insert(0, "Simulation")
                            bFirst = False
                            for x in allkeys:
                                fout.write("%s%s" % (x, sep))
                            #-- end for
                            fout.write("\n")
                        #-- end if
                        outlist.insert(0, sim_dir)
                        if (len(allkeys) == len(outlist)):
                            for o in outlist:
                                fout.write("%s%s" % (o, sep))
                            #-- end for
                            fout.write("\n")
                        else:
                            print("number of keys %d, number of values %d" % (len(outkeys), len(outlist)))
                        #-- end if
                    else:
                        if not os.path.exists(attr_file):
                            print("Missing attr file for [%s]" % (sim_dir))
                        #-- end if
                        if not os.path.exists(arr_file):
                            print("Missing arr file for [%s]" % (sim_dir))
                        #-- end if
                    #-- end if
                #-- end if
            #-- end for
        except QHGError as qhge:
            print("Error: [%s]" % (qhge.message))
        except Exception as e:
            print("Error: [%s]" % (e.message))
        #-- end try

        fout.close()
    else:
        print("%s \"<dir_pattern>\"" % (argv[0]))
    #-- end if
#-- end if
            
