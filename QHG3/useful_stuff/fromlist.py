#!/usr/bin/python

from sys import argv
from shutil import copyfile
import h5py
import os

import attr_tools
from QHGError import QHGError


#----------------------------------------------------------------------
#-- QDFFromList
#--
class QDFFromList:

    #----------------------------------------------------------------------
    #-- constructor
    #--  
    def __init__(self, base_qdf, csv_file, prefix):
        self.base_qdf   = base_qdf
        self.pop_name   = ""
        self.prefix     = prefix
        self.csv_file   = csv_file
    #-- end def


    #----------------------------------------------------------------------
    #-- make_all
    #-  
    def make_all(self):
        self.lineno = 0
        fIn = open(self.csv_file, "rt")
        
        line = fIn.readline()
        self.lineno = self.lineno + 1
        self.attr_names = line.split(",")
        print self.attr_names
        print("---------")
        
        line = fIn.readline()
        self.lineno = self.lineno + 1
        self.attr_short = line.split(",")
        print self.attr_short
        
        line = fIn.readline()
        self.lineno = self.lineno + 1
        #temp_order = [int(x) for x in line.split(",")]
        self.attr_order = inv([int(x) for x in line.split(",")])
        print self.attr_order
        
        # read the data
        line = fIn.readline()
        self.lineno = self.lineno + 1
        while line:
            self.make_for_line(line)
            line = fIn.readline()
            self.lineno = self.lineno + 1
        #-- end while

        fIn.close()
    #-- end def

    
    #----------------------------------------------------------------------
    #-- make_for_line
    #-  
    def make_for_line(self, line):
       
        attr_vals = line.split(",")

        # build name (ignore fragment if order number is negative or format string is '-')
        qdf_name = self.prefix
        for i in range(len(attr_vals)):
            p = self.attr_order[i]
            if (p >= 0):
                short_format = self.attr_short[p].strip()
                if (short_format != '-'):
                    print("i:%d; order[i]:%d; attr:[%s]" % (i, p, short_format))
                    qdf_name = qdf_name + "_" + (short_format % (float(attr_vals[p])))
                #-- end if
            #-- end if
        #-- end for
        qdf_name = qdf_name + ".qdf"

        if (os.path.exists(qdf_name)):
            print("[%d] %s already exists" % (self.lineno, qdf_name))
        #-- end if
        print("Building [%s]" % (qdf_name))
        # make a copy of the base qdf
        copyfile(self.base_qdf, qdf_name)

        # change attribute values in copy
        qdf = h5py.File(qdf_name, 'r+')

        attrs = attr_tools.get_attrs(qdf, self.pop_name)
        if not attrs is None:
            for i in range(len(attr_vals)):
            
                attr_name  = self.attr_names[i].strip()
                attr_value = attr_vals[i].strip()
                attr_tools.change_attr(attrs, attr_name, attr_value)
            #-- end for
            qdf.flush()
            qdf.close()
        else:
            printf("Problem opening attributes")
            #QHGException
        #-- end if
    #-- end def

#-- end class


#-----------------------------------------------------------------------------
#-- usage
#--
def inv(perm):
    inverse = [-1] * len(perm)
    for i, p in enumerate(perm):
        if (p >= 0):
            inverse[p] = i
        #-- end if
    return inverse
#-- end def

#-----------------------------------------------------------------------------
#-- usage
#--
def usage(app):
    print("%s - create qdf files from parameter list" % (app))
    print("Usage;")
    print("  %s <base_qdf> <csv_file> <prefix>" % (app))
    print("where")
    print("  base_qdf   base qdf file whose copies will be modified")
    print("  csv_file   csv file containing name, formats and values for the modifications")
    print("  prefix     prefix for output files")
    print("")
    print("Format of csv_file: ")
    print("  csv_file     ::= <header> <value_line>*")
    print("  header       ::= <name_line> <NL> <format_line> <NL> <order_line> <NL>")
    print("  name_line    ::= <attr_name> [ \",\" <attr_name>]* <NL>")
    print("  format_line  ::= <frag_format> [ \",\" <frag_format>]* <NL>")
    print("  order_line   ::= <order_num> [ \",\" <order_num>]* <NL>")
    print("  value_line   ::= <value> [ \",\" <value>]* <NL>")
    print("  attr_name    : name of attribute")
    print("  frag_format  : format string to build output-name fragment for attribute")
    print("                 usually something like \"b%0.2f\", \"N%03d\" etc")
    print("                 To omit fragment, use '-'.")
    print("  order_num    : defines ordering of output-name fragments")
    print("                 To omit fragment, use -1") 
    print("  value        : attribute value to use")
    print("")
    print("%s will create 1 qdf file per value_line and create name of the form" % (app))
    print("    <prefix>_<frag_1>_...<frag_n>.qdf  ")
    print("where frag_i is the result of formatting value[order[i]]  with format[order[i]]")
    print("")
    print("Example of a csv file:")
    print("  Verhulst_b0, NPPCap_K_min, WeightedMoveProb")
    print("  b%.2f,       K%02d,        P%0.1f")
    print("  0,           2,            1")
    print("  0.19,        5,            0.1")
    print("  0.20,        6,            0.1")
    print("  0.22,        5,            0.3")
#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 1):
        try:
            base_qdf = argv[1]
            csv_file = argv[2]
            prefix   = argv[3]

            vava = QDFFromList(base_qdf, csv_file, prefix)

            vava.make_all()
            
        except QHGError as qhge:
            print("Error: [%s]" % (qhge.message))
        #-- end try

    else:
        print("oh-oh")
        usage(argv[0])
    #-- end if
#-- end main
