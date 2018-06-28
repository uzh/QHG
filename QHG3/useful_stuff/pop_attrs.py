#!/usr/bin/python

from sys import argv, exit
import numpy as np
import h5py
import attr_tools
from QHGError import QHGError

                
#-----------------------------------------------------------------------------
#-- usage
#--
def usage(progname):
    print("")
    print(progname + " - modifying numeric population attributes in a QDF file")
    print("usage:")
    print("  " + progname + " show  [all] <pop-qdf>[:<pop-name>] [<attr-name>]*")
    print("or:")
    print("  " + progname + " mod   <pop-qdf>[:<pop-name>] (<attr-name> <attr-val>)*")
    print("or:")
    print("  " + progname + " add   <pop-qdf>[:<pop-name>] <attr-name> <attr-val> [<attr_type>]")
    print("or:")
    print("  " + progname + " del   <pop-qdf>[:<pop-name>] <attr-name>")
    print("where:")
    print("  pop-qdf    QDF file with populations")
    print("  pop-name   name of population (if omitted, the first population is used)")
    print("  attr-name  name of attribute")
    print("  attr-val   new value for attribute (the value must have the correct type")
    print("  attr_type  a numpy type such as 'float64', 'int32', 'S2' etv")
    print("")
#-- end def
    

#-----------------------------------------------------------------------------
#-- splitFileNamePopName
#--
def splitFileNamePopName(file_pop, params):
    
    a=file_pop.split(":")
    if (len(a) < 3):
        params['file_name'] = a[0]
        if len(a) > 1:
            params['pop_name'] = a[1]
        else:
            params['pop_name'] = ''
        #-- end if
    else:
        params = {}
    #-- end if
    return params
#-- end def


#-----------------------------------------------------------------------------
#-- getChangeParams
#--   mod_attr change <pop-qdf>[:<pop-name>] <attr-name> <attr-val>")
#--
def getChangeParams(argv, params):
    if len(argv) > 4:
        params = splitFileNamePopName(argv[2], params)
        if (len(params) > 0):
            params['attr_name'] = ''
            params['attr_val']  = ''
            if (len(argv) % 2 == 1): 
                for i in range(3,len(argv),2):
                    params['attr_name'] = params['attr_name'] + ' ' + argv[i]
                    params['attr_val']  = params['attr_val']  + ' ' + argv[i+1]
                #-- end for
            else:
                print("number of names differs from number of vals")
                params = {}
            #-- end if
                
        else:
            print("too many ':': expected <pop-qdf>[:<pop-name>]")
            params = {}
        #-- end if
    else:
        print("bad number of parameters for action [change]")
        params = {}
    #-- end if
    return params
#-- end if


#-----------------------------------------------------------------------------
#-- getDelParams
#--  mod_attrs del <pop-qdf>[:<pop-name>] <attr-name>")
def getDelParams(argv, params):

    if len(argv) > 3:
        params = splitFileNamePopName(argv[2], params)
        if (len(params) > 0):
            params['attr_name'] = argv[3]
        else:
            print("too many ':': expected <pop-qdf>[:<pop-name>]")
            params = {}
        #-- end if
    else:
        print("bad number of parameters for action [del]")
        params = {}
    #-- end if

    return params
#-- end if


#-----------------------------------------------------------------------------
#-- getShowParams
#--    mod_attr show [all] <pop-qdf>[:<pop-name>] [<attr-name>]
#-- 
def getShowParams(argv, params):

    if (argv[2] == "all"):
        bAll = True
        params["all"] = True
        i = 1
    else:
        bAll = False
        params["all"] = False
        i = 0
    #-- end if
    
    if len(argv) > i+2:
        params = splitFileNamePopName(argv[i+2], params)
        if (len(params) > 0):
            if len(argv) > i+3:
                params['attr_name'] = ''
                for j in range(i+3, len(argv)):
                   params['attr_name'] = params['attr_name']+ ' ' + argv[j]
            else:
                params['attr_name'] = ''
            #-- end if
        else:
            print("too many ':': expected <pop-qdf>[:<pop-name>]")
            params = {}
        #-- end if
    else:
        print("bad number of parameters for action [show%s]" % ("all" if (bAll) else ""))
        params = {}
    #-- end if
    return params
#-- end def


#-----------------------------------------------------------------------------
#-- getAddParams
#--   mod_attrs add <pop-qdf>[:<pop-name>] <attr-name> <attr-val> [<attr_type]")
def getAddParams(argv, params):
    if len(argv) > 4:
        params = splitFileNamePopName(argv[2], params)
        if (len(params) > 0):
            params['attr_name'] = argv[3]
            params['attr_val']  = argv[4]
            if (len(argv) > 5):
                params['attr_type'] = argv[5]
            else:
                params['attr_type'] = 'float64'
            #-- end if
        else:
            print("too many ':': expected <pop-qdf>[:<pop-name>]")
            params = {}
        #-- end if
    else:
        print("bad number of parameters for action [add]")
        params = {}
    #-- end if
    
    return params
#-- end if


#-----------------------------------------------------------------------------
#-- main
#--
if (len(argv) > 2):
    iResult = 0
    
    action = argv[1]
    params = {}
    params['file_name'] = ''
    params['pop_name']  = ''
    params['attr_name'] = ''
    params['attr_val']  = ''
    params['attr_type'] = ''
    params['all']       = ''

    try:
        if (action == "show"):
            params = getShowParams(argv, params)
        elif (action == "del"):
            params = getDelParams(argv, params)
        elif (action == "mod"):
            params = getChangeParams(argv, params)
        elif (action == "add"):
            params = getAddParams(argv, params)
        else:
            print("unknown action: [%s]" % (action))
            params = {}
        #-- end if


        if (len(params) > 0):

            try:
                qdf = h5py.File(params['file_name'], 'r+')
            except:
                raise QHGError("[main] Couldn't open ["+params['file_name']+"] as QDF file")
            else:
                attrs = attr_tools.get_attrs(qdf, params['pop_name'])
                if (not attrs is None):
                    if (action == "show"):
                        attr_tools.show_attrs(attrs, params['attr_name'], params['all'])
                    elif (action == "mod"):
                        attr_tools.change_attr(attrs, params['attr_name'], params['attr_val'])
                    elif (action == "add"):
                        attr_tools.add_attr(attrs, params['attr_name'], params['attr_val'], params['attr_type'])
                    elif (action == "del"):
                        attr_tools.del_attr(attrs, params['attr_name'])
                    else:
                        # should not happen
                        print("unknown action: [%s]" % (action))
                    #-- end if
                else:
                    #-- error message already displayed by getAttrs/()
                    pass
                #-- end if
                qdf.flush()
                qdf.close()
            #-- end try
        else:
            usage(argv[0])
        #-- end if
    except QHGError as qhge:
        print("Error: [%s]" % (qhge.message))
    #-- end try
else:
    usage(argv[0])
#-- end main
