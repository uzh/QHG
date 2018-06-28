#!/usr/bin/python

from sys import argv
from shutil import copyfile
import h5py

import attr_tools
from QHGError import QHGError


#----------------------------------------------------------------------
#-- QDFVariator
#--
class QDFVariator:

    #----------------------------------------------------------------------
    #-- constructor
    #--  
    def __init__(self, base_qdf, prefix, var_descriptions):
        self.base_qdf   = base_qdf
        self.pop_name   = ""
        self.prefix     = prefix
        self.var_keys   = []
        self.var_arrays = {}
        self.make_arrays(var_descriptions)
    #-- end def


    #----------------------------------------------------------------------
    #-- make_arrays
    #--  var_descriptions is alist of string
    #--     <varname>:<format>:<minval>:<maxval>:<step>
    #--  out put is a map
    #--     (varname,format) => v
    #--  where v = {minval+k*step: k < (<maxval>-<minval>)/<step>}
    #--
    def make_arrays(self, var_descriptions):

        var_arrays = {}
        for v in var_descriptions:
            parts = v.split(":")
            if len(parts) == 5:
                #try:
                iStartPos = 0
                bMultiplicative = False
                if (parts[4][0] == '+'):
                    iStartPos = 1
                    bMultiplicative = False
                elif (parts[4][0] == '*'):
                    iStartPos = 1
                    bMultiplicative = True
                #-- end if
                if True:
                    vmin  = float(parts[2])
                    vmax  = float(parts[3])
                    step  = float(parts[4][iStartPos:])

                    vals = []
                    v = vmin
                    if (bMultiplicative):
                        vhi = vmax*(1+1/(4*step))
                    else:
                        vhi = vmax+(step/4)
                    #-- end if
                    while (v < vhi):
                        vals.append(v)
                        if (bMultiplicative):
                            v = v * step
                        else:
                            v = v + step
                        #-- end if
                    #-- end while
                    self.var_keys.append((parts[0],parts[1]))
                    self.var_arrays[(parts[0],parts[1])] = vals

                #except:
                else:
                    raise QHGError("[make_arrays] vmin, vmax, step must be real numbers [%s, %s, %s]" % (parts[2], parts[3], parts[4]))
                    # QHGException
                #-- end try    
            else:
                raise QHGError("[make_arrays] bad var description: [%s]" % (v))
                break
                # or QHGException
            #-- end if
        #-- end for
    #-- end def


    #----------------------------------------------------------------------
    #-- getCombinations
    #--   recursive generator - iterating through all possible
    #--   combinations of elments from all arrays
    #--
    def getCombinations(self, comb, i):
        if (i < len(self.var_arrays)):
            v = self.var_arrays[self.var_keys[i]]
            for x in v:
                comb[i] = x
                # we must pass on the yielded thing from the deeper levels
                for u in self.getCombinations(comb, i+1):
                    yield u
                #-- end for
            #-- end for
        else:
            #-- deepest level yields something new
            yield comb
        #-- end if
    #-- end if


    #----------------------------------------------------------------------
    #-- createVariatons
    #--  
    def createVariatons(self):
        comb = [0]*len(self.var_arrays)
        i    = 0

        a = self.getCombinations(comb, i)
        for v in a:
            self.buildQDF(v)
       #-- end for
      
    #-- end if


    #----------------------------------------------------------------------
    #-- createQDFName
    #--   create the name using the formats provided by the user
    #--   and the values of the correspondng attribute
    #-- 
    def createQDFName(self, v):
        s=self.prefix
        for i in range(len(v)):
            s = s + "_"+(self.var_keys[i][1] % (v[i]))
        s = s + ".qdf"
        return s
    #-- end def
    
         
    #----------------------------------------------------------------------
    #-- buildQDF
    #--  - create name for QDF
    #--  - copy base qdf to new name
    #--  - change attributes of copy
    #--
    def buildQDF(self, values):
        qdf_name = self.createQDFName(values)
        print("%s" % qdf_name)

        # make a copy of the base qdf
        copyfile(self.base_qdf, qdf_name)

        # change attribute values in copy
        qdf = h5py.File(qdf_name, 'r+')

        attrs = attr_tools.get_attrs(qdf, self.pop_name)
        if not attrs is None:
            for i in range(len(values)):
            
                attr_name  = self.var_keys[i][0]
                attr_value = values[i]
                attr_tools.change_attr(attrs, attr_name, attr_value)
            #-- end for
            qdf.flush()
            qdf.close()
        else:
            printf("Problem opening attributes")
            #QHGException
        #-- end if
    #-- end def


    #----------------------------------------------------------------------
    #-- display
    #--
    def display(self):
        for x in self.var_arrays:
            s = "(%s,%s):[" % (x[0], x[1])
            v = self.var_arrays[x]
            for i in range(len(v)):
                s= s + ("%f "%v[i])
            #-- end for
            s = s + "]"
            print("%s" % (s))

        #-- end for
    #-- end def
#-- end class


#-----------------------------------------------------------------------------
#-- usage
#--
def usage(name):
    print("%s - variating attributes of qdf files" % (name))
    print("usage:")
    print("  %s <base_qdf> <out_prefix> <var_descr>*" % (name))
    print("where")
    print("  base-qdf    template qdf to be copied and varied")
    print("  out_prefix  prefix for output qdf files")
    print("  var_descr   description of attribute variation")
    print("              format")
    print("                <attr_name>:<format>:<vmin>:<vmax>:[\"+\"|\"*\"]<delta>")
    print("  attr_name   name of attribute to change")
    print("  format      print format for variable in output name")
    print("              e.g. \"C%.3f\" or \"K%03d\"")
    print("  vmin        minimum value to use")
    print("  vmax        maximum value to use")
    print("  delta       step: additive (\"+\") or multiplicative (\"*\")")
    print("")
    print("%s iterates through all possible combinations" % (name))
    print("of values from the arrays defined by the <var_descr> strings")
    print("and creates a qdf file with a name based on")
    print("the changed attribute values.") 
    print("Example:")
    print("   variate.py base.qdf gnom Verhulst_b0:\"b%.2f\":0.2:0.3:0.1  NPPCap_coastal_factor:\"C%.3f\":0.15:0.3:0.08")
    print("This creates 6 qdf files:") 
    print("   gnom_b0.20_C0.150.qdf")
    print("   gnom_b0.20_C0.230.qdf")
    print("   gnom_b0.20_C0.310.qdf")
    print("   gnom_b0.30_C0.150.qdf")
    print("   gnom_b0.30_C0.230.qdf")
    print("   gnom_b0.30_C0.310.qdf")

#-- end def


#-----------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    if (len(argv) > 1):
        try:
            base_qdf = argv[1]
            prefix   = argv[2]
            var_descr = []
            for i in range(len(argv)-3):
                var_descr.append(argv[i+3])
            #-- end for

            vava = QDFVariator(base_qdf, prefix, var_descr)
            #vava.display()

            vava.createVariatons()
            
        except QHGError as qhge:
            print("Error: [%s]" % (qhge.message))
        #-- end try

    else:
        usage(argv[0])
    #-- end if
#-- end main
