from sys import argv
from sys import stdout

import threading
import subprocess as sp
import JobManager

MAX_MSG_SIZE = 1024

CMD_JOB = "job:%s:%s"
CMD_DIE = "die!"
class Communicator:

    #------------------------------------------------------------------------
    #-- constructor
    #--
    def __init__(self, commsock, job_manager, num_iters, host, host_dir, logfile, processed_list):
        self.commsock    = commsock
        self.job_manager = job_manager
        self.num_iters   = num_iters
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
        sMessage = self.commsock.recv(MAX_MSG_SIZE)
        return sMessage
    #-- end def 


    #------------------------------------------------------------------------
    #-- sendJobToSock
    #--
    def sendJobToSock(self, job_name):
        self.commsock.send(CMD_JOB % (job_name, self.num_iters))
    #-- end def 


    #------------------------------------------------------------------------
    #-- sendKillToSock
    #--
    def sendKillToSock(self):
        self.commsock.send(CMD_DIE)
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
    #--
    def __call__(self):
        bGoOn = True
        while bGoOn:
            msg = self.getMessageFromSock()
            if msg == "ready":
                cur_job = self.job_manager.get_next_job()
                print("curjob: ["+str(cur_job)+"]")
                if not cur_job is None:
                    self.scpJob(cur_job)
                    #-- maybe only continue if scp was ok
                    cur_job = cur_job.split('/')[-1]
                    self.sendJobToSock(cur_job)
                    self.fLog.write("sent off job [%s]\n" % (cur_job))
                    self.processed_list.append("[%s] %s" % (self.host, cur_job))
                else:
                    self.sendKillToSock()
                    bGoOn = False
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

if __name__ == '__main__':
    jm = JobManager.JobManager('.', '*.txt')

    #-- for all hosts: spawn worker
    #-- loop with server socket: commsock=s.accept()
    #-- comms.append(commsock, jm, host, host_dir, "log_"+host+".lof")    

    print("creating comms")
    comms = []
    for i in range(10):
        comms.append(Communicator(None, jm, "nudibranch", "4000", "/home/jody/progs/QHG3/work", "arfl_%02d.lst"%(i)))
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

