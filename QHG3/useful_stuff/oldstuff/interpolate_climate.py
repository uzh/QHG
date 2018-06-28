#!/usr/bin/python

from sys import argv, exit
import re
import climate_interpolator as ci

#-----------------------------------------------------------------------------
#-- usage
#--
def usage(progname):
    print("")
    print(progname + " - interpolating BRIDGE-style NetCDF data")
    print("usage:")
    print("  " + progname + " <nc-dir> <nc-pat> <time-list> <output-pat>")
    print("where:")
    print("  nc-dir      directory in which to search for files matching <nc-pat>")
    print("  nc-pat      regular expression for the files of interest (enclosed in quotes).")
    print("              We assume that the filenames havea uniform structure")
    print("              which contains the time. The wildcards standing for")
    print("              this number must be enclosed in round brackets, so")
    print("              that this value can be extracted.")
    print("              The matching files should be NetCDF files with variables")
    print("              named '"+ci.NAME_TEMP+"' and '"+ci.NAME_RAIN+"'")
    print("  time-list   comma-separated list of time for which to calculate")
    print("              interpolations.")
    print("  output-pat  pattern for the generation of ouput file names.")
    print("              The substring '###' will be replaced by the time value")
    print("")
    print("Example:")
    print("  " + progname + " CLIMATE/BRIDGE/ '(2[0-9]|3[0-4])ka\.ann\.nc' 23,25,30:32@0.5  new_###.nc")
    print("This will create the files")
    print(" 'new_23.0.nc', 'new_25.0.nc', 'new_27.0.nc', 'new_29.0.nc',")
    print(" 'new_30.0.nc', 'new_30.5.nc', 'new_31.0.nc', 'new_31.5.nc', 'new:32.0.nc")
    print("containing interpolations based on the data from the files 'XXka.ann.nc'")
    print("where XX is any integer number between 20 and 34")
    print("")
#-- end def


#-----------------------------------------------------------------------------
#-- parseTimeDef
#--  Time definition format:
#--    time-def    ::= <time-el> ["," <time-el>]*
#--    time-el     ::= <time-point> | <time-range>
#--    time-point  :   float
#--    time-range  ::= <time-from> ":" <time-to> "@" <time-step>
#--    time-from   :   float
#--    time-to     :   float
#--    time-step   :   float
#--
def parseTimeDef(time_def, prec):
    times = []
    floatexp = '[0-9]*[\.0-9][0-9]*'
    rr     = re.compile('('+floatexp+'):('+floatexp+')@('+floatexp+')$')
    times0 = time_def.split(',')
    for t in times0:
        m = rr.match(t)
        if  m != None:
            #-- it's a time range
            e = m.groups()
            if len(e) == 3:
                try:
                    low  = float(e[0])
                    hi   = float(e[1])
                    step = float(e[2])
                except:
                    print("Ignoring time def ["+t+"] (not numeric)")
                else:
                    if low <= hi:
                        t1 = low
                        while t1 <= hi:
                            times.append(str(t1)) 
                            t1 += step
                        #-- end while
                    else:
                        print("Ignoring time def ["+t+"] (hi < low)")
                    #-- end if
                #-- end try
            #-- end if
        else:
            try:
                t1 = float(t)
            except:
                print("Ignoring time def ["+t+"] (not numeric)")
            else:
                times.append(str(t1))
            #-- end try
        #-- end if
    #-- end for
    # remove doubles, sort numerically, format
    return ["%.*f"%(prec, y) for y in sorted([float(x) for x in list(set(times))])]
#-- end def



#-----------------------------------------------------------------------------
#-- main
#--
if len(argv) > 4:
    ncdir   = argv[1]
    ncpat   = argv[2]
    times   = parseTimeDef(argv[3],1)
    output  = argv[4]

    x = ci.ClimateInterpolator(ncdir, ncpat)

    x.doInterpolations(times, output)
else:
    usage(argv[0])
#-- end if

