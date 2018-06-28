from sys import argv,exit

import traceback
from numpy import prod

#-----------------------------------------------------------------------------
#--
#--
class QHGError(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)
    #-- end def
#-- end class


#--------------------------------------------------------------------
#-- col_count
#--    make sure all lines have same number of columns
#--    return number of column or -1 on error
#--
def col_count(file_name):
    nc = 0
    try:
        for l in open(file_name):
                          
            n = len(l.split())
            if (n > 0):
                if nc == 0:
                    nc = n
                else:
                    if not nc == n:
                        nc = -1
                        raise QHGError("Different number of columns in lines")
                        break;
                    #-- end if
                #-- end if
            #-- end if
        #-- end for
    except  Exception as e:
        raise QHGError("couldn't open %s for reading: Excpetion %s" % (file_name, e))
    #-- end try
    return nc
#-- end def


#--------------------------------------------------------------------
#-- get_col_set
#--   translate the columns specification to a list
def get_col_list(colspec, idx_max):
    s = []
    if (colspec == "all"):
        s=range(0, idx_max)
    else:
        element = colspec.split(':')
        for x in element:
            u = x.split('-')
            lo = int(u[0])
            
            if (len(u) == 1):
                hi = lo
            else:
                hi = int(u[1])
            #-- end if

            if (hi > idx_max):
                hi = idx_max
            #--end if
        
            if (lo > hi):
                hi = lo
            #-- end if

            for i in range(lo-1, hi):
                s.append(i)
            #-- end for
        #-- end for
    #-- end if
    print("colspec: \"%s\" -> %s" % (colspec, str(s)))
    return s
#-- end def 


#--------------------------------------------------------------------
#-- file_line_provider
#--   a generator yielding specified columns of the
#--   non-empty, non-blank lines of a file
#--
def file_line_provider(filename, collist):
    try:
        f=open(filename,"rt")
        for line in f:
            line = line.strip()
            if (len(line) > 0): 
                words = ""
                x = line.split()
                for c in collist:
                    if (len(words) > 0):
                        words = words + "\t"
                    #-- end if
                    words = words + x[c]
                #--end for
                yield words
            #-- end if
        #-- end for
        f.close()
    except QHGError as e:
        raise e
    except  Exception as e:
        raise QHGError("couldn't open %s and extract data. Exception: %s" % (filename, e))
    #-- end try
    yield None
#-- end def


#--------------------------------------------------------------------
#-- expect input
#--   <file1> <cols1> ... <fileN> <colsN> <output>
#--
iResult = 0
iNum = len(argv)

if (iNum > 3) and (iNum % 2) == 0:
    iResult = -1
    line_providers=[]
    fOut = None
    try:
        for i in range(0,iNum//2-1):
            n1 = col_count(argv[2*i+1])
         
            s1 = get_col_list(argv[2*i+2], n1)
            line_providers.append(file_line_provider(argv[2*i+1], s1))
        #-- end for
        fOut = open(argv[-1], "wt")
        iResult = 0
                                  
    except QHGError as e:
        print("QHGError:  %s" % (e))
    except  Exception as e:
        print("Exception: %s" % (e))
        tb=traceback.format_exc()
        print(tb)
    else:
        bMore = True
        iLine = 1
        while (iResult == 0) and bMore:
            sCurLine = ""
            delivered = []
         
            for ff in line_providers:
                if (len(sCurLine) > 0):
                    sCurLine = sCurLine + "\t"
                #-- end if
         
                sPart = next(ff)
                if sPart is None:
                    # this one is finished
                    delivered.append(0)
                else:
                    sCurLine = sCurLine + sPart
                    delivered.append(1)
                #-- end if
            #-- end for
            if (sum(delivered) == 0):
                #-- all ave finished
                bMore = False
            elif (prod(delivered) == 1):
                fOut.write("%s\n" % (sCurLine))
                bMore = True
            else:
                #-- partially done: error
                print("At least one file has the wrong number of lines")
                print("truncated after %d lines" % (iLine-1))
                iResult = -1
            #-- end if
            iLine = iLine + 1
        #-- end while
        fOut.close()
    #-- end try
    if (iResult == 0):
        print("+++ success +++")
    #-- end if
else:
    print(" %s -- merge columns of text files" % (argv[0]))
    print("Usage:")
    print("  %s <file> <cols> (<file> <cols>)* <output>" % (argv[0]))
    print("where;")
    print("  file    name of text file containing table data")
    print("          each line should have the same number of columns")
    print("  output  name of output file")
    print("  cols    column specification: columns to be selected")
    print("          (column numbering starts at 1)")
    print("          format:")
    print("            cols ::= \"all\" | <cols> \":\" <cols> | <lo> \"-\" <hi>")
    print("          Example:")
    print("            2-4:7:5:6-9")
    print("          yields")
    print("            [2,3,4,7,5,6,7,8,9]")
    print("")
#-- end if

