#!/usr/bin/python

from sys import argv, exit
import numpy as np
import h5py
from QHGError import QHGError


#-----------------------------------------------------------------------------
#-- np.dtype kinds and names used in doChange()
#--
typenames={'b':	'boolean',
           'i':	'(signed) integer',
           'u': 'unsigned integer',
           'f': 'floating-point',
           'c': 'complex-floating point',
           'm': 'timedelta',
           'M': 'datetime',
           'O': '(Python) object',
           'S': '(byte-)string',
           'a': '(byte-)string',
           'U': 'Unicode',
           'V': 'raw data (void)'}


#-----------------------------------------------------------------------------
#--  get_attrs 
#--    get attributes for a specific population
#--    if no population name is given, the first population is used
#--
def get_attrs(hf, pop_name):
    attr = None
    if 'Populations' in list(hf.keys()):
        gp= hf['Populations']
        if (len(list(gp.keys())) > 0):
            if pop_name == '':
                attr=gp[list(gp.keys())[0]].attrs
            else:
                if pop_name in list(gp.keys()):
                    attr=gp[pop_name].attrs
                else:
                    raise QHGError("[get_attrs] No population ["+pop_name+"] found")
                #-- end if               
        else:
            raise QHGError("[get_attrs] No population subgroups found")
        #-- end if
    else:
        raise QHGError("[get_attrs] No group ['Populations'] found")
    #-- end if 
    return attr
#-- end def


#-----------------------------------------------------------------------------
#-- is_numeric_attr
#--
def is_numeric_attr(attr):
    k = attr.dtype.kind
    return k in  ['b', 'i', 'u', 'f']
#-- end def


#-----------------------------------------------------------------------------
#-- cast_numeric
#--   convert a string into a number of the desired type
#--   b : bool
#--   i : integer
#--   u : unsigned
#--   f : float
#--
def cast_numeric(valstr, kind):
    valnum = None
    try:
        if kind == 'b':
            valnum = valstr.lower() in ("yes", "true", "t", "1")
        elif kind == 'i':
            valnum = int(valstr)
        elif kind == 'u':
            v = int(valstr)
            if (v > 0):
                valnum = v
            #-- end if
        elif kind == 'f':
            valnum = float(valstr)
        else:
            raise QHGError("[cast_numeric] Unknown type char ["+kind+"]")
        #-- end of
    except:
        pass # print("Conversion failed")
    #-- end try
    return valnum
#-- end def


#-----------------------------------------------------------------------------
#-- change_attr
#--
def change_attr(attrs, sattr_names, sattr_vals):
    attr_names = sattr_names.split()
    attr_vals  = sattr_vals.split()
    if (len(attr_names) ==len(attr_vals)):
        for i in range(len(attr_names)):
            vprev = attrs.get(attr_names[i])
            if (not vprev is None):
                # attribute must be numeric
                if (is_numeric_attr(vprev)):
                    # try to cast to vprev's general type
                    new_valc = cast_numeric(attr_vals[i], vprev.dtype.kind)
                    if (not new_valc is None):
                        new_valarr = np.array([new_valc])
                        # make sure types match
                        if (new_valarr.dtype.kind == vprev.dtype.kind):
                            attrs.modify(attr_names[i], new_valarr)
                        else:
                            raise QHGError("[change_attr] attribute ["+attr_names[i]+"] requires a value of type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+"), and not '"+new_valarr.dtype.kind+"'")
                        #-- end if
                    else:
                        raise QHGError("[change_attr] Couldn't cast ["+attr_vals[i]+"] to type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+")")
                    #-- end if
                else:
                    raise QHGError("[change_attr] attribute ["+attr_names[i]+"] is not numeric (type '"+vprev.dtype.kind+"' ("+typenames[vprev.dtype.kind]+")")
                #-- end if
            else:
                raise QHGError("[change_attr] No attribute ["+attr_names[i]+"] found in population group")
            #-- end if
        #-- end for
    else:
        raise QHGError("[change_attr] Number of attribute names and values differ")
    #-- end if
#-- end def


#-----------------------------------------------------------------------------
#-- del_attr
#--
def del_attr(attrs, attr_name):

    vprev = attrs.get(attr_name)
    if (not vprev is None):
        del attrs[attr_name]
    else:
        raise QHGError("[del_attr] No attribute ["+attr_name+"] found in group ["+pop_name+"]")
    #-- end if
#-- end def


#-----------------------------------------------------------------------------
#-- add_attr
#--
def add_attr(attrs, attr_name, attr_val, val_type):
    val = np.array([attr_val],dtype=val_type)
    attrs.create(attr_name, val)
#-- end def


#-----------------------------------------------------------------------------
#-- show_attrs
#--
def show_attrs(attrs, attr_name, bAll):
   
    aa0={}
    aa1={}
    l0=0
    l1=0
    for x in attrs:
        if (type(attrs[x])==np.ndarray) and (len(attrs[x]) > 1):
            aa1[x]=attrs[x]
            if len(x) > l1:
                l1 = len(x)
            #-- end if
        else:   
            aa0[x]=attrs[x]
            if len(x) > l0:
                l0 = len(x)
            #-- end if
        #-- end if
                    
    #-- end for
    found = {}
    if (attr_name == ''):
        for n in sorted(aa0):
            print(n.ljust(l0)+"\t"+str(aa0[n]))
            
        #-- end for
    else:
        attrs=attr_name.split()
        for attr in attrs:
            if (attr in aa0):
                print(attr.ljust(l0)+"\t"+str(aa0[attr]))
                if attr not in found:
                    found[attr] = True
                #-- end if 
            else:
                if attr not in found:
                    found[attr] = False
                #-- end if 
            #-- end if
        #-- end for
    #-- end if
    
    if (bAll):
        if (attr_name == ''):
            for n in sorted(aa1):
                print(n.ljust(l1)+"\t"+str(aa1[n]).replace("\n", " "))
            #-- end for
        else:
            attrs=attr_name.split()
            for attr in attrs:
                if (attr in aa1):
                    print(attr.ljust(l0)+"\t"+str(aa1[attr]))
                    if (attr not in found) or (not found[attr])  :
                        found[attr] = True
                    #-- end if 
                else:
                    if attr not in found:
                        found[attr] = False
                    #-- end if 
                #-- end if 
            #-- end for 
        #-- end if
    #-- end if
    for key in found:
        if (not found[key]):
            print(key.ljust(l0)+"\t"+"<-- does not exist -->")
        #-- end if
    #-- end for
#-- end def

