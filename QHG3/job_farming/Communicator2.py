from sys import argv
from sys import stdout

import threading
import subprocess as sp
import JobManager2
import StringSock2 as ssock

MAX_MSG_SIZE = 1024

CMD_JOB = "job:%s:%s:%s"
CMD_DIE = "die!"
class Communicator2:

    #------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, strsock, job_manager, num_iters, num_procs, host, host_dir, logfile, processed_list):
        self.strsock     = strsock
        self.job_manager = job_manager
        self.num_iters   = num_iters
        self.num_procs   = num_procs
        self.host        = host
        self.host_dir    = host_dir
        self.fLog = open(logfile, "wt")
        self.done = False
        self.processed_list = processed_list
        print("Created comm for [%s]" % (logfile))
    #-- end def


    #------------------------------------------------------------------------
    #-- getMessageFromSock
    #--
    def getMessageFromSock(self):
        sMessage = self.strsock.get_string()
        return sMessage
    #-- end def 


    #------------------------------------------------------------------------
    #-- sendJobToSock
    #--
    def sendJobToSock(self, job_name):
        self.strsock.put_string(CMD_JOB % (job_name, self.num_iters, self.num_procs))
    #-- end def 


    #------------------------------------------------------------------------
    #-- stop_loop
    #--
    def stop_loop(self, host):
        if (self.host == host) or (self.host == "all"):
            self.strsock.put_string(CMD_DIE)
        #-- end if
    #-- end def 


    #------------------------------------------------------------------------
    #-- scpJob
    #--
    def scpJob(self, cur_job):
	#-- execute scp command to copy cur_job to the worker
        args = ["scp", cur_job, self.host+":"+self.host_dir]
        # now do popen stuff and return sonething
        proc = sp.Popen(args,
                        stdout=sp.PIPE,
                        stderr=sp.PIPE)
        sOut, sError = proc.communicate()
        iResult = proc.returncode

        mylock = threading.Lock()
        mylock.acquire() 
        s = " ".join(args)
        print("%s:  %d" % (s, iResult))
        stdout.flush()
        mylock.release()
    #-- end def


    #------------------------------------------------------------------------
    #-- __call__
    #--  this function is necessary to make Communcator callable
    #--
    def __call__(self):
        self.bGoOn = True
        while self.bGoOn:
            msg = self.getMessageFromSock()
            if msg == "ready":
                cur_job = self.job_manager.get_next_job(self.host)
                print("curjob: ["+str(cur_job)+"]")
                if not cur_job is None:
                    self.scpJob(cur_job)
                    #-- maybe only continue if scp was ok
                    cur_job = cur_job.split('/')[-1]
                    self.sendJobToSock(cur_job)
                    self.fLog.write("sent off job [%s]\n" % (cur_job))
                    self.processed_list.append("[%s] %s" % (self.host, cur_job))
                else:
                    self.stop_loop(self.host)
                #-- end if
            elif msg == "err":
               # set error state
               bGoOn = False
            #-- end if
        #-- end while
        self.done = True
        self.fLog.close()

    #-- end def 

#-- end class


#-----------------------------------------------------------------------------
#-- test application
#--
if __name__ == '__main__':
    jm = JobManager.JobManager('.', '*.txt')
    s=jm.list()
    print(s)
    print("creating comms")
    comms = []
    proclist = {}
    for i in range(10):
        proclist[i] = []
        comms.append(Communicator2(None, jm, "4000", "4", "nudibranch", "/home/jody/progs/QHG3/work", "log_%02d.log", proclist[i]))
    #-- end for
    print("creating  threads")
    my_threads = []
    for x in comms:
        thread_cur = threading.Thread(target=x)
        my_threads.append(thread_cur)
    #-- end for
    print("starting threads")    
    for t in my_threads:
        t.start()
    #-- end for

    print("waiting for threads")
    finished = 0
    while finished < len(my_threads):
        finished = 0
        for t in my_threads:
             if not t.isAlive():
                  finished = finished + 1
             #-- end if
        #-- end for	    
    #-- end while
    print("all done")

