#!/usr/bin/python

from sys import argv
import socket
import subprocess as sp
import time
import threading

import JobManager2
import Communicator2
import StringSock2 as ssock

LAUNCHER_REMOTE    = "/home/jody/utils/launcher.sh"
APPLICATION_REMOTE = "/home/jody/utils/Worker2.py"

PORT = 50007
HOST = ''

DEF_MASK  = "*.qdf"
DEF_ITERS = 85000

CMD      = "cmd:"
CMD_STOP = "stop:"
CMD_JOB  = "job:"
SUBCMD_RELOAD = "reload"
SUBCMD_ADD    = "add:"
SUBCMD_KILL   = "kill:"
SUBCMD_DROP   = "drop:"
SUBCMD_LIST   = "list"

#------------------------------------------------------------------------
#-- class Controller2
#-- 
class Controller2:

    #------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, host_string, port, num_iters, num_procs, job_dir, job_mask, iSpawn):
        self.job_manager = JobManager2.JobManager2(job_dir, job_mask)
        self.host_list = [] if host_string == '' else host_string.split(':')
        self.port      = port
        self.comms     = []
        self.threads   = []
        self.num_iters = num_iters     
        self.num_procs = num_procs     
        self.this_host = socket.gethostname()
        self.proc_list = {}
        self.bGoOn = True
        self.bSpawnProcesses = (iSpawn != 0)
        self.strsock = None
    #-- end def

                
    #------------------------------------------------------------------------
    #-- do_work
    #--
    def do_work(self):
        s=self.job_manager.list()
        print("Joblist")
        print(s)
        
        if self.bSpawnProcesses:
            self.spawn_remote_apps(LAUNCHER_REMOTE, APPLICATION_REMOTE)
        #-- end if
        self.start_listen_loop()
    #-- end def


    #------------------------------------------------------------------------
    #-- start_listen_loop 
    #--
    def start_listen_loop(self):
        this_host = socket.gethostname()
        numComms = len(self.host_list)
        print("hostlist: %s (%d)"%(str(self.host_list), numComms))
        self.comms = []
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((HOST, self.port))
        s.listen(5)
        print("[%s]listening to port %d" % (self.this_host, self.port))
        
        self.bGoOn = True
        while (self.bGoOn):
            conn, addr = s.accept()
            self.strsock = ssock.StringSock2(conn)
            print("[%s] Connected by %s" % (self.this_host, addr))
            first_message = self.strsock.get_string()
            print("[%s] message [%s]" % (self.this_host, first_message))
            if first_message.startswith(CMD):
                self.process_command(first_message)
            else:
                a = first_message.split(':')
                if len(a) == 2:
                    self.run_communicator(self.strsock, a[0], a[1])
                else:
                    print("[%s] bad request [%s]"%(self.this_host, str(a)))
                #-- end if
            #-- end if

            # count living threads - exit loop if no one left
            num_alive = 0
            for t in self.threads:
                if t.isAlive():
                    num_alive = num_alive + 1
                #-- end if
            #-- end for
            if (num_alive == 0):
                print("No threads left - exiting listen loop")
                self.bGoOn = self.bGoOn and (numComms == 0);
            #-- end if
        #-- end while
        
        s.close()

        # ahow processed jobs
        print("processed jobs:")
        for x in sorted(self.proc_list):
            print("---")
            print("%s:" % x)
            for y in self.proc_list[x]:
                print("  %s" % y)
            #-- end for
        #-- end for

        
    #-- end def


    #------------------------------------------------------------------------
    #-- spawn_remote_apps
    #--
    def spawn_remote_apps(self, launcher_remote, application_remote):
        print("[%s] Starting to spawn" % (self.this_host))
        this_host = socket.gethostname()
        for host in self.host_list:
            if (host != ""):
                args = ["ssh", host, launcher_remote, application_remote, "%s:%d"%(this_host,self.port)]
                print(" ".join(args))
                proc = sp.Popen(args)
                # we hope the application has been spawned - no easy way to check this is the case
            #-- end if
        #-- end for
    #-- end def


    #------------------------------------------------------------------------
    #-- run_communicator
    #--   create communicator and start thread for it
    #--
    def run_communicator(self, strsock, host, host_dir):
        logfile = "controller_%s.lst" % (host)
        self.proc_list[host]=[]
        new_comm = Communicator2.Communicator2(strsock, self.job_manager, self.num_iters, self.num_procs, host, host_dir, logfile, self.proc_list[host])
        self.comms.append(new_comm)
    
        thread_cur = threading.Thread(target=new_comm)
        self.threads.append(thread_cur)

        thread_cur.start()
    #-- end def


    #------------------------------------------------------------------------
    #-- process_command
    #--  we know cmd_string starts with "cmd:"
    #--  th rest should be
    #--    "stop"
    #--    "job:reload"
    #--    "job:add:"<dirname>
    #--    "job:list
    #- 
    def process_command(self, cmd_string):
        sOut = ''
        cmd = cmd_string[len(CMD):]
        print("processing command [%s]" % cmd)
        if cmd.startswith(CMD_STOP):
            arg = cmd[len(CMD_STOP):]
            for c in self.comms:
                c.stop_loop(arg)
            #-- end for
            
            if (arg == "all"):
                self.bGoOn = False
                sOut = "stopped comm loops"
            #-- end if
        elif cmd.startswith(CMD_JOB):
            arg = cmd[len(CMD_JOB):]
            
            if arg == SUBCMD_RELOAD:
                sOut = self.job_manager.reload()
                
            elif arg.startswith(SUBCMD_ADD):
                sOut = self.job_manager.add(arg[len(SUBCMD_ADD):])

            elif arg.startswith(SUBCMD_KILL):
                sOut = self.job_manager.remove(arg[len(SUBCMD_KILL):])

            elif arg.startswith(SUBCMD_DROP):
                sOut = ''
                s = self.job_manager.drop(arg[len(SUBCMD_DROP):])
                saa = s.split(':')
                title = saa[0]
                num   = int(saa[1])
                sOut = sOut + "%s(%d)\n" % (title, num)
                for i in range(2,len(saa)):
                    sOut = sOut + "  %s\n" % (saa[i])
                #-- end for
                
            elif arg == SUBCMD_LIST:
                sOut = ''
                s = self.job_manager.list()
                sa = s.split('|')
                for x in sa:
                    saa = x.split(':')
                    title = saa[0]
                    num   = int(saa[1])
                    sOut = sOut + "%s(%d)\n" % (title, num)
                    for i in range(2,len(saa)):
                        sOut = sOut + "  %s\n" % (saa[i])
                    #-- end for
                    sOut = sOut + "\n"
                #-- end for
                
            else:
                sOut = "unknown job subcommand [%s]" % arg
                print(sOut)
            #-- end if
        else:
            sOut = "unknown command [%s]" % cmd
        #-- end if

        if (sOut != ""):
            self.strsock.put_string(sOut)
            self.strsock.close()
        #-- end if 
    #-- end def

#-- end class


#-----------------------------------------------------------------------------
#-- main app
#--
if __name__ == '__main__':
  
    # set default values
    host_list = ""
    port       = PORT
    job_dir    = ""
    job_mask   = DEF_MASK
    num_iters  = DEF_ITERS
    num_procs  = ''
    bSpawn     = 1
    remote_app = APPLICATION_REMOTE
    
    # get definitive values from argv
    for x in argv:
        if x.startswith("--host="):
             host_list = x.split('=')[1].strip()
        elif x.startswith("--port="):
             port = int(x.split('=')[1].strip())
        elif x.startswith("--job-dir="):
             job_dir = x.split('=')[1].strip()
        elif x.startswith("--job-mask="):
             job_mask = x.split('=')[1].strip()
        elif x.startswith("--num-iters="):
             num_iters = int(x.split('=')[1].strip())
        elif x.startswith("--num-procs="):
             num_procs = int(x.split('=')[1].strip())
        elif x.startswith("--spawn-procs="):
             bSpawn = int(x.split('=')[1].strip())
        elif x.startswith("--remote-app="):
             remote_app = x.split('=')[1].strip()
        #-- end if
    #-- end for
    #if ((host_list != '') or (job_dir != '')):
    if  (job_dir != ''):
        if (job_dir.startswith('~'):
            print("DO NOT USE '~' IN PATH\n")
        else:
            #create and run
            controlletti = Controller2(host_list, port, str(num_iters), str(num_procs), job_dir, job_mask, bSpawn)
            controlletti.do_work()
        #-- end if
    else:
        print("launch QHGMain on remote hosts")
        print("usage:")
        print("  %s --job_dir=<jobdir>  [--job_mask=<jobmask>]" % (argv[0]))
        print("     [-host=<hostlist>] [--port=<portnum>]")
        print("     [--remote-app=<apppath>] [--spawn-procs=<val>]")
        print("     [--num-iters=<numiters>] [--num-procs=<numprocs>]")
        print("where:")
        print("  jobdir     full path to directory containing intial qdfs")
        print("             (USE FULL PATHNAME WITHOUT '~'!)")
        print("  jobmask    mask for job fils (default: '%s'])" % (DEF_MASK))
        print("  hostlist   ':'-separated list of host names (if not given, no remote apps can be launched)")
        print("  portnum    port to listen onr (default: %d)" % (PORT))
        print("  apppath    path to application to be launched on remote host (default: [%s])" % APPLICATION_REMOTE)
        print("  val        1: spawn remote Worker processes; 0: no spawning (default: 1)")
        print("  numiters   number of simulation iterations (default: %d)" % (DEF_ITERS))
        print("  numproc    number of processors to use (OMP_NUM_THREADS); (default: all)")
    #-- end if
#-- end main
