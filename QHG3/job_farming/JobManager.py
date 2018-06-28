
from sys import argv
import glob
import threading

class JobManager:

    #------------------------------------------------------------------------
    #-- __init__
    #--
    def __init__(self, jobdir, jobmask):
        self.jobdir  = jobdir 
        self.jobmask = jobmask

        self.job_list = glob.glob(self.jobdir + "/" + self.jobmask)
        print("Found job [%s]" % (len(self.job_list)))
        self.lock = threading.Lock()
    #-- end def

    #------------------------------------------------------------------------
    #-- get_next_job
    #--
    def get_next_job(self):
        reply = None
        self.lock.acquire()
        if len(self.job_list) > 0:
           reply = self.job_list[-1]
           del self.job_list[-1]
        #-- end if
        self.lock.release()
        return reply
    #-- end def

        
#-- end class

if __name__ == '__main__':
    jm = JobManager(".", "*.py")
    print(jm.job_list)
    bok = True
    while (bok):
        x = jm.get_next_job()
        if x is None:
           bok = False
        else:
           print("got job [%s]" % (x))
        #-- end if
    #-- end while
#-- end if
