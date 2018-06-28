#!/usr/bin/python


from sys import argv
import os.path
import glob
import subprocess
from shutil import copyfile
import PCOCreator

MODE_CLOUD  = "cloud"
MODE_TRIOPS = "triops"
MODE_KRAKEN = "kraken"
modes=[MODE_CLOUD, MODE_TRIOPS, MODE_KRAKEN]

CLOUD_QHGDIR   = "/home/centos/qhg_data/QHG3"
KRAKEN_QHGDIR  = "/data/jw_simulations/qhgtests/src/QHG3"
TRIOPS_QHGDIR  = "/home/jody/progs/QHG3/trunk"
KRAKEN_BASE    = "/data/jw_simulations/qhgtests"

DEF_POP_PATTERN  = "ooa_pop-Sapiens_ooa__###000.qdf"
DEF_STAT_PATTERN = "ooa_SGCVNM_###000.qdf"
DEF_OUT_NAME     = "analysis"

COL_RED   = "\033[0;31m"
COL_BLUE  = "\033[0;34m" 
COL_OFF   = "\033[0m"

#-----------------------------------------------------------------------------
#--
#--
class QHGError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class

#-----------------------------------------------------------------------------
#--
#--
class ParamError(Exception):
    def __init__(self, where, message):
        Exception.__init__(self, "[%s] %s"% (where, message))
    #-- end def
#-- end class




#-------------------------------------------------------------------------
#-- readParams
#--  paramdefs: dictionary opt => [name, default]
#--             
def readParams(param_defs):
    i = 1
    results = {}
    
    keys = param_defs.keys()
    
    # set default values
    for k in keys:
        results[param_defs[k][0]] = param_defs[k][1]
    #-- end for
    
    while i < len(argv):
        if argv[i] in keys:
            if (i+1) < len(argv):
                results[paramdefs[argv[i]][0]] = argv[i+1]
                i = i + 2
            else:
                raise ParamError("readParams", "expected value for [%s]" % (argv[i]))
            #-- end if
        else:
            raise ParamError("readParams", "unknown parameter [%s]" % (argv[i]))
        #-- end if
    #-- end while

    for k in paramdefs:
        if param_defs[k][1] is None:
            if (not param_defs[k][0] in results.keys()) or (results[param_defs[k][0]] is None):
                raise ParamError("readParams", "required parameter missing [%s]" % (param_defs[k][0]))
            #-- end if
        #-- end if
    #-- end for
        
    return results;
    
#-- end def
    

#-------------------------------------------------------------------------
#-- usage
#--
def usage():
    print("")
    print("%s - creating PCOs for a simulation" % (argv[0]))
    print("Usage")
    print("  %s -m <mode> -d <data-dir> -t <times> -o <outdir>" % (argv[0]))
    print("       [-p <pop_pattern>] [-s <stat_pattern>] [-n <outname>]")
    print("       [-l <logdir>] [-z <zipstate>]")
    print("       [-x <seq-type>] [-r <samp-file>]")
    print("or\n")
    print("  %s -q <qhgdir> -d <data-dir> -l >logdir>" % (argv[0]))
    print("        -d <data-dir> -l <logdir> -t <times> -o <outdir>")
    print("       [-p <pop_prefix>] [-s <stat_prefix>] [-n <outname>]")
    print("       [-z <zipstate>] [-x <seq-type>] [-r <samp-file>]")
    print("where")
    print("  mode         machine name: 'triops' | 'kraken' | 'cloud'")
    print("  qhgdir       QHG root directory")
    print("  datadir      directory containing the [gzipped] pop and stat files")
    print("  times        quoted list of  3-digit times (representing multiples of 1000)")
    print("  outdir       top output directory (in which a subdirectory with simulation name will be created)")
    print("  pop-pattern  pattern for pop qdfs (use '#' as placeholder for numbers) (default  %s)" % (DEF_POP_PATTERN))
    print("  stat-pattern prefix for stat qdfs (use '#' as placeholder for numbers) (default  %s)" % (DEF_STAT_PATTERN))
    print("  outname      prefix for output files in <outdir> (default %s)" % (DEF_OUT_NAME))
    print("  logdir       directory containing the .log and .out files");
    print("               default triops: the QHG3/app directory")
    print("               default kraken: the qhgtest directory")
    print("               default cloud:  the QHGMain output directory")
    print("  zipstate     1: gunzipping and gzipping required")
    print("  seq-type     sequencetype: 'genes' or 'phenes' (default: 'genes')\n")
    print("  samp-file    name of previously created sample file (default: None)\n")
    print("")
#-- end def 


#-------------------------------------------------------------------------
#-- determineQHGDir
#--
def determineQHGDir(mode):
    if mode == MODE_CLOUD:
        qhgdir     = CLOUD_QHGDIR
        cloudstyle = True
    elif mode == MODE_KRAKEN:
        qhgdir     = KRAKEN_QHGDIR
        cloudstyle = False
    elif mode == MODE_TRIOPS:
        qhgdir     = TRIOPS_QHGDIR
        cloudstyle = False
    elif os.path.exists(mode):
        qhgdir     = mode
        cloudstyle = False
    #-- end if
    return qhgdir,cloudstyle
#-- end def


#-------------------------------------------------------------------------
#-- determineLogDir
#--
def determineLogDir(mode, qhgdir, datadir, logdir):
    if logdir == "":
        if mode == MODE_CLOUD:
            logdir = datadir.split("output")[0]
        elif mode == MODE_KRAKEN:
            logdir = KRAKEN_BASE
        elif mode == MODE_TRIOPS:
            logdir = qhgdir + "/app"
        #-- end if
    #-- end if
    return logdir
#-- end def

   
#-------------------------------------------------------------------------
#-- main
#--
if __name__ == '__main__':
    try:
        if (len(argv) <= 8):
            usage()
        else:
            paramdefs = {
                "-m" : ["mode", None],
                "-q" : ["qhqdir", ""],
                "-d" : ["datadir", None],
                "-p" : ["pop_prefix", DEF_POP_PATTERN],
                "-s" : ["stat_prefix", DEF_STAT_PATTERN],
                "-z" : ["zipped", False],
                "-l" : ["logdir", ""],
                "-t" : ["times", None],
                "-o" : ["outdir", None],
                "-n" : ["outname", DEF_OUT_NAME],
                "-x" : ["seq_type", "genes"],
                "-r" : ["in_samp", ""]
                }

            results = readParams(paramdefs)

            if not results["mode"] is None:
                qhgdir,cloudstyle = determineQHGDir(results["mode"])
                print("Have qhgdir [%s], cloudstyle:%s" % (qhgdir, str(cloudstyle)))
                logdir = determineLogDir(results["mode"], qhgdir, results["datadir"], results["logdir"])
                print("Have logdir [%s]" % (logdir))
            else:
                if (not results["qhgdir"] is None) and (not results["logdir"]  is None):
                    qhgdir = results["qhgdir"]
                    logdir = results["logdir"]
                else:
                    raise ParamError("Either -m  or both -q and -l must be specified")
                #-- end if
            #-- end of
            
            pcoc = PCOCreator.PCOCreator(qhgdir,
                                         results["datadir"],
                                         results["pop_prefix"],
                                         results["stat_prefix"],
                                         results["zipped"],
                                         logdir,
                                         results["times"],
                                         results["outdir"],
                                         results["outname"],
                                         cloudstyle)
        
            pcoc.createOuputDir()

            if pcoc.zipped:
                pcoc.gunzipFiles()
            #-- end if

            
            pcoc.doQDFSampler(results["seq_type"], results["in_samp"])

            pcoc.doBinSeqMerge2(results["seq_type"])

            pcoc.doSeqDist(results["seq_type"])

            pcoc.doCalcPCO()

            pcoc.doColumnMerger()

            pcoc.doAttributes()

            pcoc.doArrivals()

            pcoc.doCopyLogs()

            if pcoc.zipped:
                pcoc.gzipFiles()
            #-- end if
        #-- end if
        
    except QHGError as e:
        print("\n%sQHGError: %s%s" % (COL_RED, e, COL_OFF))
    except ParamError as e:
        print("\n%sParamError: %s%s" % (COL_RED, e, COL_OFF))
        print(COL_BLUE)
        usage()
        print(COL_OFF);
#    except  Exception as e:
#        print("Exception: %s" % (e))
#        raise(e)
    #-- end try

#-- end if
        
