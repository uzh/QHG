#!/usr/bin/python


import sys
import os.path
import glob
import subprocess as sp
from shutil import copyfile

KRAKEN_BASE    = "/data/jw_simulations/qhgtests"

DEF_POP_PREFIX  = "ooa_pop-Sapiens_ooa_"
DEF_STAT_PREFIX = "ooa_SGCVNM"
DEF_OUT_NAME    = "analysis"

COL_RED   = "\033[0;31m"
COL_BLUE  = "\033[0;34m" 
COL_OFF   = "\033[0m"

SEQ_GENE   =  "genes"
SEQ_PHENE  =  "phenes"
GEN_SAMP   =  "QDFGenomeSampler2"
GEN_MERG   =  "BinGeneMerge2"
GEN_DIST   =  "GeneDist2"

PHEN_SAMP  =  "QDFPhenomeSampler2"
PHEN_MERG  =  "BinPheneMerge2"
PHEN_DIST  =  "PheneDist2"

PROC_LOG   =  "PCOCreator.log"
PROC_SEP   =  "-------------------------------\n"

seq_apps ={}
seq_apps[SEQ_GENE]  = {"samp":GEN_SAMP,  "merg":GEN_MERG,  "dist":GEN_DIST}
seq_apps[SEQ_PHENE] = {"samp":PHEN_SAMP, "merg":PHEN_MERG, "dist":PHEN_DIST}


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
#-- b2s
#--   bytes to string
#--
def b2s(b):
    return "".join(map(chr,b))
#-- end def


#-----------------------------------------------------------------------------
#--
#--
class PCOCreator:
    
    #-------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, qhgdir, datadir, pop_pat, stat_pat, zipped, logdir, times, outtop, outname, cloudstyle=False):
        self.QHGdir      = qhgdir
        self.datadir     = datadir.rstrip('/')
        self.logdir      = logdir
        self.pop_pat     = pop_pat
        self.stat_pat    = stat_pat
        self.times       = times.split(" ")
        self.zipped      = zipped
        self.outtop      = outtop
        self.outname     = outname
        self.cloudstyle  = cloudstyle

        self.proclogfile = open(PROC_LOG, "wt")
        self.popfile  = self.datadir + "/" + self.pop_pat
        self.statfile = self.datadir + "/" + self.stat_pat

        i0Pop=self.pop_pat.find('#')                  
        i1Pop=self.pop_pat.rfind('#')                  
        n = i1Pop - i0Pop +1
        self.pat=""
        for i in range(n):
            self.pat = self.pat + '#'
            #-- end for
        print("pat for[%s]:  %d [%s]" %(self.pop_pat, n, self.pat))

        #-- 
        if self.cloudstyle:
            self.simname = self.datadir.split("output")[0].strip("/").split("/")[-1]
        else:
            self.simname = self.datadir.split("/")[-1]
        #-- end if
        print("datadir [%s], simname [%s]\n" % (self.datadir, self.simname))
        try:
            print("logdir: [%s]" % self.logdir)
            
            # check if required directories exist
            iResult = self.checkExistence()
            
            self.gridfile = self.QHGdir + "/resources/grids/EmptyGridSGCV_256.qdf"
            self.locfile  = self.QHGdir + "/genes/LocationListGridsRed.txt"

            self.outdir= self.outtop + "/" + self.simname
            print("outdir [%s], outname  [%s]\n" %( str(self.outdir), str(self.outname)))
            self.outbody=self.outdir + "/" + self.outname
        except ParamError as e:
            raise e
        except Exception as e:
            raise QHGError("constructor", "File existence check failed. Exception: %s" % (e))
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- checkExistence
    #--
    def checkExistence(self):
    
        if os.path.exists(self.datadir) and os.path.isdir(self.datadir):

            self.suffix = ".gz" if self.zipped else ""
            self.laststat = self.statfile.replace(self.pat, self.times[-1])
            if os.path.exists(self.laststat+self.suffix) and os.path.isfile(self.laststat+self.suffix):
                exist_ok = True
                curqdf=""
                for t in self.times:
                    curqdf = self.popfile.replace(self.pat, t) + self.suffix                 

                    if not (os.path.exists(curqdf) and os.path.isfile(curqdf)):
                        exist_ok = False
                        break
                    #-- end if
                #-- end for
                        
                if exist_ok:
                    if os.path.exists(self.logdir) and os.path.isdir(self.logdir):
                        print("existence ok")
                    else:
                        print("aa8")
                        raise QHGError("checkExistence", "logdir [%s] does not exist or isn't a directory" % (self.logdir))
                    #-- end if
                    
                else:
                    raise QHGError("checkExistence", "popfile [%s] does not exist" % (curqdf))
                #-- end if
            else:
                raise QHGError("checkExistence", "statfile [%s] does not exist" % (self.laststat+self.suffix))
            #-- end if
            
        else:
            raise QHGError("checkExistence", "datadir [%s] does not exist or isn't a directory" % (self.datadir))
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    # gunzip_file
    #  gunzip gz_file to out_file
    #
    def gunzip_file(gz_file, out_file):
    
        fIn = gzip.open(gz_file, "rb")
        fOut = open(out_file, "wb")
        fOut.write(fIn.read())
        fIn.close()
        fOut.close()
    #-- end def


    #-----------------------------------------------------------------------------
    # gzip_file
    #  gzip in_file to gz_file
    #
    def gzip_file(in_file, gz_file):
    
        fIn = open(in_file, "rb")
        fOut = gzip.open(gz_file, "wb")
        fOut.write(fIn.read())
        fIn.close()
        fOut.close()
    #-- end def


    #-------------------------------------------------------------------------
    #-- gunzipFiles
    #--
    def gunzipFiles(self):
        print("gunzipping")
        self.proclogfile.write("%sgunzipping\n", PROC_SEP)
        
        iResult = 0

        try:
            gunzipFile(self.laststat+".gz",  self.laststat)
            for t in self.times:
                try:
                    curqdf = self.popfile.replace(self.pat, t)
                    gunzipFile.call(curqdf+".gz", curqdf)
                except:
                    iResult = -1
                    print("couldn't gunzip [%s]" % (curqdf))
                    self.proclogfile.write("couldn't gunzip [%s]\n" % (self.laststat+".gz"))
                    raise QHGError("gunzipFiles", "couldn't gunzip [%s]" % (self.laststat+".gz"))
                    break
                #-- end try
            #-- end for
        except:
            iResult = -1
            self.proclogfile.write("couldn't gunzip [%s]\n" % (self.laststat+".gz"))
            raise QHGError("gunzipFiles", "couldn't gunzip [%s]" % (self.laststat+".gz"))
        #-- end try
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- gzipFiles
    #--
    def gzipFiles(self):
        print("gzipping")
        self.proclogfile.write("%sgzipping\n", PROC_SEP)
        
        iResult = 0
        try:
            gzipFile(self.laststat,  self.laststat+".gz")
            for t in self.times:
                try:
                    curqdf = self.popfile.replace(self.pat, t)
                    gzipFile(curqdf, curqdf+".gz")
                except:
                    iResult = -1
                    self.proclogfile.write("couldn't gzip [%s]\n" % (curqdf))
                    raise QHGError("gzipFiles", "couldn't gzip [%s]" % (curqdf))
                    break
                #-- end try
            #-- end for
        except:
            iResult = -1
            self.proclogfile.write("couldn't gzip [%s]\n" % (self.laststat))
            raise QHGError("gzipFiles", "couldn't gzip [%s]" % (self.laststat))
        #-- end try   

        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- createOuputDir
    #--
    def createOuputDir(self):
        self.proclogfile.write("%screateOutputDir\n" % (PROC_SEP))

        if not os.path.exists(self.outtop):
            os.mkdir(self.outtop)
        else:
            if (os.path.isdir(self.outtop)):
                self.proclogfile.write("[%s] already exists\n" % (self.outtop))
            else:
                self.proclogfile.write("createOuputDir failed: [%s] is not a directory\n" % (self.outtop))
                raise QHGError("createOuputDir", "[%s] is not a directory" % (self.outtop))
            #-- end if
        #-- end if

        if not os.path.exists(self.outdir):
            os.mkdir(self.outdir)
        else:
            if (os.path.isdir(self.outdir)):
                print("[%s] already exists" % (self.outdir))
            else:
                self.proclogfile.write("createOuputDir failed: [%s] is not a directory\n" % (self.outdir))
                raise QHGError("createOuputDir", "[%s] is not a directory" % (self.outdir))
            #-- end if
            
        #-- end if
        
    #-- end def

    
    #-------------------------------------------------------------------------
    #-- doQDFSampler
    #--
    def doQDFSampler(self, seq_type, samp_in):
        app_name = seq_apps[seq_type]["samp"]

        for t in self.times:

            print("%s for [%s]" % (app_name, t))
            self.proclogfile.write("%s%s for [%s]\n" % (PROC_SEP, app_name, t))
            
            curpop = self.popfile.replace(self.pat, t)
            curout = self.outbody + "_" + t
            if (samp_in != ""):
                sampline = "--read-samp=%s_%s.smp" % (samp_in, t)
            else:
                sampline = ""
            #-- end if
            
            self.proclogfile.write("outbody[%s]->[%s]\n" % (self.outbody, curout))
            command = [self.QHGdir + "/genes/" + app_name,
                       "-i", curpop,
                       "-s", "Sapiens_ooa",
                       "-o", curout,
                       "-f","bin:num",
                       "--location-file="+self.locfile,
                       "--write-samp="+curout+".smp",
                       "-g", self.gridfile]#,
                       #"-q"]
            if (sampline != ""):
                command.append(sampline)
            #-- end if

            proc = sp.Popen(command,  stdout=sp.PIPE,  stderr=sp.PIPE)
            sOut, sError = proc.communicate()
            self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
            iResult = proc.returncode
            if (iResult == 0):
                print("+++ success +++")
            else:
                self.proclogfile.write("*** %s failed for [%s]:\n  %s\n error: %s\n" % (app_name, curpop, str(command), b2s(sError)))
                raise QHGError("doQDFSampler","%s failed for [%s]:\n  %s\n error: %s" % (app_name, curpop, str(command), b2s(sError)))
            #-- end if
            sys.stdout.flush()
        #-- end for
        return iResult
    #-- end def


    
    #-------------------------------------------------------------------------
    #-- doBinSeqMerge
    #--
    def doBinSeqMerge2(self, seq_type):
        app_name = seq_apps[seq_type]["merg"]
        print(app_name)
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))
        
        infiles = []
        for t in self.times:
            infiles.append(self.outbody + "_" + t + ".bin")
        #-- end for
        command = [self.QHGdir + "/genes/" + app_name,
                   self.outbody+".all.bin"] + infiles
                
        proc = sp.Popen(command, stdout=sp.PIPE, stderr=sp.PIPE)
                    
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** %s failed:\n  %s\nerror: %s\n" % (app_name, str(comm), b2s(sError)))
            raise QHGError("doBinSeqMerge2", "%s failed:\n  %s\nerror: %s" % (app_name, str(comm), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doGeneDist
    #--
    def doSeqDist(self, seq_type):
        app_name = seq_apps[seq_type]["dist"]
        print(app_name)
        self.proclogfile.write("%s%s\n" % (PROC_SEP, app_name))
        
        command = [self.QHGdir+"/genes/" + app_name,
                   "-g", self.outbody + ".all.bin",
                   "-o", self.outbody,
                   "-m", self.laststat,
                   "-G", self.gridfile]
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** %s failed:\n  %s\nerror %s\n" % (app_name, str(command), b2s(sError)))
            raise QHGError("doSeqDist", "%s failed:\n  %s\nerror %s" % (app_name, str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doCalcPCO
    #--
    def doCalcPCO(self):
        print("CalcPCO")
        self.proclogfile.write("%sCalcPCO\n" % (PROC_SEP))
        
        command = [self.QHGdir+"/genes/calcpco_R.sh",
                   self.outbody+".full.mat",
                   self.outbody+".full.pco"]

        self.proclogfile.write("%s\n" % (str(command)))
                                  
        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** CalcPCO failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doCalcPCO", "CalcPCO failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doColumnMerger
    #--
    def doColumnMerger(self):
        print("ColumnMerger")
        self.proclogfile.write("%sColumnMerger\n" %(PROC_SEP))
        
        command = ["python", self.QHGdir+"/genes/ColumnMerger.py",
                   self.outbody+".tagdists",
                   "all",
                   self.outbody+".full.pco",
                   "1-20",
                   self.outbody+".tab"]

        proc = sp.Popen(command,  stdout=sp.PIPE, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        self.proclogfile.write("command %s\noutput:\n%s\n" % (str(command), b2s(sOut)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** ColumnMerger failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doColumnMerger", "ColumnMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def


    #-------------------------------------------------------------------------
    #-- doAttributes
    #--
    def doAttributes(self):
        print("Attributes")
        self.proclogfile.write("%sAttributes\n" % (PROC_SEP))
        attrfile = open(self.outbody + ".attr", "wt")
        curpop = self.popfile.replace(self.pat,  self.times[-1])
        command = [self.QHGdir+"/useful_stuff/show_attr",
                   "all",
                   curpop]

        proc = sp.Popen(command,  stdout=attrfile, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        attrfile.close()
        self.proclogfile.write("command %s\n" % (str(command)))
        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** doAttributes failed:\n  %s\nerror %s\n" % (str(command), b2s(sError)))
            raise QHGError("doAttributes", "ColumnMerger failed:\n  %s\nerror %s" % (str(command), b2s(sError)))
        #-- end if

        sys.stdout.flush()
        return iResult
    #-- end def

        
    #-------------------------------------------------------------------------
    #-- doArrivals
    #--
    def doArrivals(self):
        print("arrival times")
        self.proclogfile.write("%sarrival times\n" % (PROC_SEP))
        
        arrfile = open(self.outbody + ".arr", "wt")
        command = [self.QHGdir+"/genes/ArrivalCheck",
                    "-g", self.laststat,
                    "-l", self.locfile,
                    "-n"]
        proc = sp.Popen(command,  stdout=arrfile, stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode
        arrfile.close()
        self.proclogfile.write("command %s\n" % (str(command)))

        if (iResult == 0):
            print("+++ success +++")
        else:
            self.proclogfile.write("*** doArrivals failed:\n  %s\n" % (str(command)))
            raise QHGError("doArrivals", "doArrivals failed:\n  %s" % (str(command)))
        #-- end if
        sys.stdout.flush()
        return iResult
    #-- end def

        
    #-------------------------------------------------------------------------
    #-- doCopyLogs
    #--   in our cloud setup, the name of log and out are not known,
    #--   but they are the only ones in the log dir
    #--
    def doCopyLogs(self):
        print("CopyLogs")
        self.proclogfile.write("%sCopyLogs\n" % (PROC_SEP))
        
        if self.cloudstyle:
            logfile = glob.glob(self.logdir + "/*.log")[0]
            outfile = glob.glob(self.logdir + "/*.out")[0]
        else:
            logfile = self.logdir + "/" + self.simname + ".log"
            outfile = self.logdir + "/" + self.simname + ".out"
        #-- end if
        logtarg = logfile.split("/")[-1]
        outtarg = outfile.split("/")[-1]
        self.proclogfile.write("copying [%s] to [%s]\n" % (logfile,  self.outdir+"/"+logtarg))
        self.proclogfile.write("copying [%s] to [%s]\n" % (outfile,  self.outdir+"/"+outtarg))
        try:
            copyfile(logfile, self.outdir+"/"+logtarg)
            copyfile(outfile, self.outdir+"/"+outtarg)
        except Exception as e:
            self.proclogfile.write("*** doCopyLogs failed:\n  %s\n" % (str(e)))
            raise QHGError("doCopyLogs", "doCopyLogs failed:\n  %s" % (str(e)))
        else:
            print("+++ success +++")
        #-- end try
    #-- end def


    #-------------------------------------------------------------------------
    #-- doProcess
    #--
    def doProcess(self, seq_type):
        self.createOuputDir()
        
        if self.zipped:
                self.gunzipFiles()
            #-- end if

        self.doQDFSampler(seq_type)

        self.doBinSeqMerge2(seq_type)
        
        self.doSeqDist(seq_type)

        self.doCalcPCO()

        self.doColumnMerger()

        self.doAttributes()

        self.doArrivals()

        self.doCopyLogs()

        if self.zipped:
            self.gzipFiles()
        #-- end if
    #-- end def


    #-------------------------------------------------------------------------
    #-- __del__
    #--
    def __del__(self):
        self.proclogfile.write("ending PCOCreator\n")
        self.proclogfile.close()
        print("Outputs written to [%s]\n" % (PROC_LOG))
    #-- end def
    
#-- end class

