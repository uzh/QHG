#!/usr/bin/python

from sys import argv
import socket
import subprocess as sp
import time
import threading

import JobManager
import Communicator

LAUNCHER_REMOTE    = "/home/jody/utils/launcher.sh"
APPLICATION_REMOTE = "/home/jody/utils/Worker.py"

PORT = 50007
HOST = ''
MAX_SECS = 60
DEF_MASK = "*.qdf"
DEF_ITERS = 85000

#------------------------------------------------------------------------
#-- class Controller
#-- 
class Controller:
 
    #------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, host_string, port, num_iters, job_dir, job_mask, timeout):
        self.job_manager = JobManager.JobManager(job_dir, job_mask)
        self.host_list = host_string.split(':')
        self.port      = port
        self.timeout   = timeout
        self.comms     = []
        self.threads   = []
        self.num_iters = num_iters     
        self.this_host = socket.gethostname()
        self.proc_list = []
    #-- end def

                
    #------------------------------------------------------------------------
    #-- do_work
    #--
    def do_work(self):
        self.spawn_remote_apps(LAUNCHER_REMOTE, APPLICATION_REMOTE)
        self.create_comms()
	self.create_threads()
     
        self.start_threads()
        
        self.join_threads()
    #-- end def

 
    #------------------------------------------------------------------------
    #-- create_comms 
    #--
    def create_comms(self):
        this_host = socket.gethostname()
        numComms = len(self.host_list)
        self.comms = []
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((HOST, self.port))
        s.listen(5)
        print("[%s]listening to port %d" % (self.this_host, self.port))
        t0 = time.clock()
        self.proc_list = [[]] * numComms
        i = 0
        while (len(self.comms) < numComms) and ((time.clock() - t0) < self.timeout): 
            conn, addr = s.accept()
            print("[%s] Connected by %s" % (self.this_host, addr))
            name_dir = conn.recv(1024)
	    a = name_dir.split(':')
	    if len(a) == 2:
                logfile = "controller_%s.lst" % (a[0])
                print("[%s] new comm: Communicator(conn, self.job_manager, %s, %s, %s, %s" % (self.num_iters, self.this_host, a[0], a[1], logfile))
                new_comm = Communicator.Communicator(conn, self.job_manager, self.num_iters, a[0], a[1], logfile, self.proc_list[i])
                self.comms.append(new_comm)
	        #self.comms.append(Communicator(conn, self.job_manager, a[0], a[1], "controller_%s.lst" % (a[0])))    
                i = i + 1
            else:
                print("[%s] bad request [%s]"%(self.this_host, str(a)))
            #-- end if
        #-- end while
        s.close()
    #-- end def


    #------------------------------------------------------------------------
    #-- spawn_remote_apps
    #--
    def spawn_remote_apps(self, launcher_remote, application_remote):
        print("[%s] Starting to spawn" % (self.this_host))
        this_host = socket.gethostname()
        for host in self.host_list:
  	    #args = ["ssh", "-f", host, launcher_remote, application_remote, "%s:%d"%(this_host,self.port)]
  	    args = ["ssh", host, launcher_remote, application_remote, "%s:%d"%(this_host,self.port)]
            print(" ".join(args))
            proc = sp.Popen(args)
            # we hope the application has been spawned - no easy way to check this is the case
        #-- end for
    #-- end def
     

    #------------------------------------------------------------------------
    #-- create_threads
    #--
    def create_threads(self):
        print("[%s] Creating threads" % (self.this_host))
        self.threads = []
        for x in self.comms:
            thread_cur = threading.Thread(target=x)
            self.threads.append(thread_cur)
        #-- end for
    #-- end def


    #------------------------------------------------------------------------
    #-- start_threads
    #--
    def start_threads(self):
        print("[%s] Starting threads" % (self.this_host))
        for t in self.threads:
            t.start()
        #-- end for
    #-- end def


    #------------------------------------------------------------------------
    #-- join_threads
    #--
    def join_threads(self):
        print("[%s] Waiting for threads" % (self.this_host))
        finished = 0
        while finished < len(self.threads):
            finished = 0
            for t in self.threads:
                if not t.isAlive():
                      finished = finished + 1
                #-- end if
            #-- end for         
        #-- end while
        print("[%s] all done" % (self.this_host))
        for x in self.proc_list:
            print("---")
            for y in x:
                print("  %s" % y)
            #-- end for
        #-- end for
    #-- end def 

#-- end class

if __name__ == '__main__':
  
    # set default values
    host_list = ""
    port      = PORT
    timeout   = MAX_SECS
    job_dir   = ""
    job_mask  = DEF_MASK
    num_iters = DEF_ITERS

    # get definitive values from argv
    for x in argv:
        if x.startswith("--host="):
             host_list = x.split('=')[1].strip()
        elif x.startswith("--port="):
             port = int(x.split('=')[1].strip())
        elif x.startswith("--timeout="):
             timeout = int(x.split('=')[1].strip())
        elif x.startswith("--job-dir="):
             job_dir = x.split('=')[1].strip()
        elif x.startswith("--job-mask="):
             job_mask = x.split('=')[1].strip()
        elif x.startswith("--num-iters="):
             num_iters = int(x.split('=')[1].strip())
        #-- end if
    #-- end for
    if ((host_list != '') or (job_dir != '')):
        #create and run
        controlli = Controller(host_list, port, str(num_iters), job_dir, job_mask, timeout)
        controlli.do_work() 
    else:
        print("launch QHGMain on remote hosts")
        print("usage:")
        print("  %s --host=<hostlist> --job_dir=<jobdir>" % (argv[0]))
        print("     [--port=<portnum>]  [--job_mask=<jobmask>]")
        print("     [--num-iters=numiters>] [--timeout=<timeout>]")
        print("where:")
        print("  hostlist   ':'-separated list of host names")
        print("  jobdir     full path to directory containing intial qdfs")
        print("  portnum    port to connect over (default: %d)" % (PORT))
        print("  jobmask    mask for job fils (default: '%s'])" % (DEF_MASK))
        print("  numiters   number of simulation iterations (default: %d)" % (DEF_ITERS))
        print("  timeout    timeout for listening and accepting (default: %ds)" % (MAX_SECS))
    #-- end if
#-- end main
