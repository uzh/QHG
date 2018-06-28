#!/usr/bin/python

from sys import argv,stdout
import socket
import time
import random
import os
import shutil

import QHGStarter2
import StringSock2 as ssock

DEF_DIR = "/home/jody/progs/QHG3/work2"
DEF_ITERS = '85000'

class Worker:
    
    #------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, server_port):
        self.this_host =  socket.gethostname()
        self.cfg_data = {}
        self.read_cfg("/home/jody/utils/Worker.cfg")
        if ("--output-dir" in self.cfg_data):
            self.work_dir = self.cfg_data["--output-dir"]
        else:
            self.work_dir = DEF_DIR
        #-- end if
        self.log_name = "/tmp/%s_%06d.log" % (self.this_host, random.randint(0,999999))
        self.fout = open(self.log_name, "wt")
        a  = server_port.split(':')
        if (len(a) == 2):
            self.server = a[0]
            self.port   = int(a[1])
            self.outlog("Have server [%s] and port [%d]"%(self.server,self.port))
        else:
            self.outlog("bad host:port string\n")
	    # exception or so
        #-- end if
    #-- end def


    #------------------------------------------------------------------------
    #-- cleanup
    #--
    def cleanup(self):
 	self.fout.close()
        if os.path.exists(self.work_dir):
            shutil.copyfile(self.log_name, "%s/%s.log" % (self.work_dir, self.this_host))
	else:
            print("+++++ log file for %s : [%s] +++++" % (self.this_host, self.log_name))
        #-- end if
    #-- end def


    #------------------------------------------------------------------------
    #-- outlog
    #--
    def outlog(self, mess):
        self.fout.write("[%s] %s\n"%(self.this_host, mess))
        self.fout.flush()
        print("[%s] %s" % (self.this_host, mess))
        stdout.flush()
    #-- end def 


    #------------------------------------------------------------------------
    #-- read_cfg
    #--
    def read_cfg(self, cfg_file):
        try:
            f = open(cfg_file, "rt")
            for line in f:
                if not line.startswith('#'):
                    a = line.split('=')
                    if (len(a) == 2):
                        self.cfg_data[a[0]] = a[1].strip()
                    #-- end if
                #-- end if
            #-- end for
        except:
	    outlog("couldn't open cfg file [%s]" % (cfg_file))
        #-- end try
    #-- end def


    #------------------------------------------------------------------------
    #-- establish_connection
    #--
    def establish_connection(self):
        conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.outlog("Trying to connect to [%s] on port %d" % (self.server, self.port))
        bGoOn = True
        while bGoOn:
            try:
                conn.connect((self.server, self.port))
                self.strsock = ssock.StringSock2(conn)
                bGoOn = False
            except:
                time.sleep(0.1)
            #-- end try
        #-- end while
        if not bGoOn:
            self.outlog("connected ok")
            self.outlog("sending hostname [%s] and dir [%s]" % (self.this_host, self.work_dir))
            self.strsock.put_string("%s:%s"%(self.this_host, self.work_dir))
        else:
            self.outlog("timed out")
        #-- end if 
    #-- end def


    #------------------------------------------------------------------------
    #-- do_work
    #--
    def do_work(self):
        self.outlog("establishing connection")
        self.establish_connection()
        time.sleep(0.5)
        bGoOn = True
        # enter work cycle
        while bGoOn:
            self.strsock.put_string("ready") 
            cmd = self.strsock.get_string()
            if (cmd.startswith("job:")):
                num_iters = DEF_ITERS
                num_procs = ''

                self.outlog("got job command: [%s]"%(cmd))
                a = cmd.split(':')
                self.outlog("split:[%s] (%d)"%(str(a), len(a)))
                pop_name = a[1]
                if (len(a) > 2):
                    num_iters = a[2]
                    if (len(a) > 3):
                        num_procs = a[3]
                    #-- end if
                #-- end if
                
                # process job

                # set config variables
                if num_procs != '':
                    self.cfg_data["num_procs"] = num_procs
                #-- end 
                self.cfg_data["--num-iters"] = num_iters
                self.cfg_data["--pops"] = self.work_dir+'/'+pop_name

                self.cfg_data["--shuffle"] = "%06d" % (random.randint(0,32767))
                # and start
                qs = QHGStarter2.QHGStarter2(self.cfg_data, self.fout)
                qs.call_standard_QHG()

            elif (cmd == "die!"):
                self.outlog("got kill command")
                bGoOn = False
            else:
                self.outlog("unknown command: [%s]" % (cmd))
            #-- end if
        #-- end while
        self.strsock.close()
    #-- end def
#-- end class


#-----------------------------------------------------------------------------
#-- main app
#--
if __name__ == "__main__":
    if (len(argv) > 1):
        worker = Worker(argv[1])
        worker.do_work()
        worker.cleanup()
    else:
        print("Worker - accepts job commands and runs QHGStarter")
        print("Usage:")
        print("  Worker <host>:<port>")
    #-- end if
#-- end main
