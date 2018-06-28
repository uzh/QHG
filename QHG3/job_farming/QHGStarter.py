#!/usr/bin/python
from sys import argv, stdout
import os
import shutil
import subprocess as sp
import random
import socket

MAX_IDX = 10
TEMP_DIR = "tempwork"

class QHGStarter:

    #-----------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, arg_map, dbg_out):
        self.this_host = socket.gethostname()
        if dbg_out is None:
            self.dbg_out = stdout
        else:
            self.dbg_out = dbg_out
        #-- end if
        self.outlog("starting QHGStarter")
        print("sssstarting QHGStarter")
        self.arg_map = arg_map
    #-- end def

     #------------------------------------------------------------------------
    #-- outlog
    #--
    def outlog(self, mess):
        self.dbg_out.write("[%s] %s\n"%(self.this_host, mess))
        self.dbg_out.flush()
        print("[%s] %s" % (self.this_host, mess))
        stdout.flush()
    #-- end def 


  
    #-----------------------------------------------------------------------------
    #-- find_unused_dir
    #--
    def find_unused_dir(self, qdir, num):
        out_main = ''
        i = 1
        while (out_main == '') and (i < num):
            temp = qdir + '_' + str(i)
            if not os.path.exists(temp):
                out_main = temp
            else:
                i = i + 1
            #-- end if
        #_- end while
        return out_main
    #-- end def 


    #-----------------------------------------------------------------------------
    #-- find_map_value
    #--
    def find_map_value(self, key):
        val= ''
        if key in self.arg_map:
            val = self.arg_map[key]
        #-- end if
        return val
    #-- end def


    #-----------------------------------------------------------------------------
    #-- delete_map_value
    #--
    def delete_map_value(self, key):
        if key in self.arg_map:
            del self.arg_map[key]
        #-- end if
    #-- end def


    #-----------------------------------------------------------------------------
    #-- map_to_arglist
    #--
    def map_to_arglist(self):
        arglist = []
        for x in self.arg_map:
            if x.startswith("--"):
                arglist.append("%s=%s" % (x, self.arg_map[x]))
            #-- end if
        #-- end for
        return arglist
    #-- end def

    #-----------------------------------------------------------------------------
    #-- build_standard_args
    #--
    def call_standard_QHG(self):
        self.outlog("extracting basic arguments")
        # collect params we need to create names for output directory,logfile and out file
        pop_qdf  = self.find_map_value('--pops')
        shuffle  = self.find_map_value('--shuffle')
        out_dir  = self.find_map_value('--output-dir')
        qhg_main = self.find_map_value('qhg_main')
        
        self.outlog("removing some arguments")
        # remove any '--log-file=', '--output-dir=', '--seed='; we're gonna make our own
        self.delete_map_value('--log-file')
        self.delete_map_value('--output_dir')
        self.delete_map_value('--seed')
        
        self.outlog("trying to make dir [%s]" % (out_dir))
        # create the bas output directory (ok if it already exists)
        try:
            os.makedirs(out_dir)
        except:
            pass
        #-- end try
    

        # find a new output directory name and create names for logfile and out file
        out_main_real = self.find_unused_dir(out_dir + '/' + pop_qdf.split('/')[-1], MAX_IDX)
        # but use a temp name until simulation is finished
        out_main_temp = out_dir + '/' + TEMP_DIR
        shutil.rmtree(out_main_temp)
        if (out_main_real != ''):
            out_qdf  = out_main_temp + '/output'
            logfile  = out_main_temp + '/ooa_' + shuffle + '.log'
            outfile  = out_main_temp + '/ooa_' + shuffle + '.out'
            
            self.outlog("out main: " + out_main_temp + "\n")
            os.makedirs(out_main_temp)
            self.outlog("out qdf:  " + out_qdf + "\n")
            os.makedirs(out_qdf)
            self.outlog("log file: " + logfile + "\n")
            self.outlog("out file: " + outfile + "\n")
            
            arguments = [qhg_main, '--log-file='+logfile, '--output-dir='+out_qdf]
            arguments.extend(self.map_to_arglist())
            
            self.outlog("\n")
            self.outlog("starting %s:\n" % (qhg_main))
            self.outlog("with "+str(arguments) + "\n")
            
            fOut = open(outfile, 'wt')
            proc = sp.Popen(arguments,
                            stdout=fOut,
                            stderr=sp.PIPE)
            sOut, sError = proc.communicate()
            iResult = proc.returncode
            fOut.close()
            if (iResult == 0):
                self.outlog("run finished ok\n")
                os.rename(out_main_temp, out_main_real)
                self.outlog("runmed output dir to [%s]\n" % out_main_real)
            else:
                self.outlog("%s failed with %d" % (qhg_main, iResult))
            #-- end if
        
        else:
            self.outlog("Couldn't find unused dir of the form '%s_N' with N<%d\n"% (pop_qdf, MAX_IDX))
        #-- end if
    #-- end def 

#-- end class

if __name__ == "__main__":
    if (len(argv) > 1):
        pop_file=argv[1]
        if (len(argv) > 2):
            num_iters = argv[2]
        else:
            num_iters = "5000"
        #-- end if
        cfg_file = "Worker.cfg"
        cfg_data = {}
        try:
            f = open(cfg_file, "rt")
        except:
            print("couldn't open cfg file [%s]" % (cfg_file))
        else:
            for line in f:
                if not line.startswith('#'):
                    a = line.split('=')
                    if (len(a) == 2):
                        cfg_data[a[0]] = a[1].strip()
                    #-- end if
                #-- end if
            #-- end for
            cfg_data["--num-iters"] = num_iters
            cfg_data["--shuffle"] = "%06d" % (random.randint(0,32767))
            cfg_data["--pops"] = pop_file
            print cfg_data
            print("---------")
            qs = QHGStarter(cfg_data, None)
            qs.call_standard_QHG()
