#!/usr/bin/python

from sys import argv, exit
import ice_interpolator as ii

#-----------------------------------------------------------------------------
#-- usage
#--
def usage(pApp):
    print(pApp+ " - interpolating ice")
    print("Usag;")
    print("  " + pApp + " <target-time> <file-list>")
    print("where")
    print("  taret-time  time for which to do the interpolation")
    print("  file_list   list of files containing ice data")
    print("              (ass: veriable name start with 'ICE_')")
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if len(argv) > 2:
    target_time = float(argv[1])
    filelist = list(argv[2:])
    print("target time "+str(target_time))
    print("data files  "+str(filelist))

    icei = ii.IceInterpolator(filelist)
    
    ice_new = icei.doInterpolation(target_time)

    print("----- "+str(icei.tLo)+" -----")
    print(str(icei.vLo))
    print("----- "+str(target_time)+" -----")
    print(str(ice_new))
    print("----- "+str(icei.tHi)+" -----")
    print(str(icei.vHi))

else:
    usage(argv[0])
#-- end if
