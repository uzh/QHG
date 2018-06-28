
from sys import argv
import glob
import time
import threading

#------------------------------------------------------------------------
#-- JobManager2
#--
class JobManager2:

    #------------------------------------------------------------------------
    #-- __init__
    #--
    def __init__(self, jobdir, jobmask):
        self.job_dir  = jobdir 
        self.jobmask = jobmask
        self.finished = []
        self.job_list = []

        self.lock = threading.Lock()
        self.reload()

        self.load_dir = self.job_dir
    #-- end def


    #------------------------------------------------------------------------
    #-- get_next_job
    #--
    def get_next_job(self, host):
        reply = None

        self.lock.acquire()

        # get the next job if there is any
        if len(self.job_list) > 0:
           reply = self.job_list[-1]
           self.finished.append("%s@%s"% (reply,host))
           del self.job_list[-1]
        #-- end if

        self.lock.release()
        # just in case a reload() is waiting:
        # give it time to do its stuff before getting the next job
        time.sleep(0.1)
        return reply
    #-- end def

        
    #-----------------------------------------------------------------------
    #-- reload
    #--   update current list with new entries from the load directory
    #--   (only new entries will be added)
    #--
    def reload(self):
        print("[JobManager2] reload")
        sOut = self.do_reload(self.job_dir)
        return sOut
    #-- end def


    #-----------------------------------------------------------------------
    #-- add
    #--   add entries from directory jobdir
    #--
    def add(self, new_dir):
        print("[JobManager2] add %s"%(new_dir))
        sOut = self.do_reload(new_dir)
        return sOut
    #-- end def


    #-----------------------------------------------------------------------
    #-- do_reload
    #--
    def do_reload(self, load_dir):
        # collect entries from job_dir
        temp_list = glob.glob(load_dir + "/" + self.jobmask)
        # remove duplicates
        new_items = set(temp_list) - (set(self.job_list) | set(self.finished))
        #print("Waiting for lock")
        #- we must make sure nobody is getting a new job
        while(self.lock.locked()):
            #- wait until not busy anymore
            pass
        #-- end while
        #print("lock open")
        
        # now we have 0.1 s to do this (cf.get_next_job())
        self.job_list.extend(new_items)

        # ok
        s = "Found %d new jobs - now have %d" % (len(new_items), len(self.job_list))
        print(s)
        return s
    #-- end def


    #-----------------------------------------------------------------------
    #-- remove
    #--   remove entries in new_dir from current list 
    #--
    def remove(self, kill_dir):
        print("[JobManager2] remove %s"%(kill_dir))

        kill_list = glob.glob(kill_dir + "/" + self.jobmask)
        remaining_items = set(self.job_list) - set(kill_list)
        # these two are only for output
        removed_items = set(kill_list) & set(self.job_list)
        processed_items = set(kill_list) & set(self.finished)
        
        #print("Waiting for lock")
        #- we must make sure nobody is getting a new job
        while(self.lock.locked()):
            #- wait until not busy anymore
            pass
        #-- end while
        #print("lock open")
        
        # now we have 0.1 s to do this (cf.get_next_job())
        self.job_list = list(remaining_items)

        sOut = "removed %d jobs - now have %d  (%d already processed)"%(len(removed_items), len(self.job_list), len(processed_items))
        return sOut
    #-- end def


    #-----------------------------------------------------------------------
    #-- splitInterval
    #--   splits interval into min and max indexes
    #--   returns tuple(min,.max) or errorstring
    #--
    def splitInterval(self, interval):
        if (interval.startswith('[') and interval.endswith(']')):
            pos = interval.find(',')
            if (pos >= 0):
                # strip square brackets
                interval = interval[1:-1]
                a = interval.split(',')
                minV = -1
                maxV = -1
                if a[0] == '':
                    minV = 0
                else:
                    try:
                        minV = int(a[0])
                    except:
                        minV = -1
                    #-- end try
                #-- end if
                if a[1] == '':
                    maxV = len(self.job_list)
                else:
                    try:
                        maxV = int(a[1])+1
                    except:
                        maxV = -1
                    #-- end try
                #-- end if
                
                if (minV >= 0) and (maxV >= 0) and (minV < maxV ):
                    output = (minV, maxV)
                else:
                    output = "invalid boundaries [%d,%d]" % (minV, maxV)
                #-- end if
            else:
                output = "expected a comma separating min and max values"
            #-- end if
        else:
            output = "expected square-bracket enclosed interval definitiion"
        #-- end if
        return output
    #-- end def
    

    #-----------------------------------------------------------------------
    #-- drop
    #--   drop entries defined by interval from current list 
    #--   [a,b] drop from a to and including b
    #--   [a,]  drop from a to the end
    #--   [,b]  drop from beginning to and including b
    #--   [,]    drop everything
    #--   negative numbers count fromthe back
    #-- 
    def drop(self, interval):
        print("[JobManager2] drop %s"%(interval))

        res = self.splitInterval(interval)
        
        if type(res) is tuple:
            (minV, maxV) = res  
            #print("Waiting for lock")
            #- we must make sure nobody is getting a new job
            while(self.lock.locked()):
                #- wait until not busy anymore
                pass
            #-- end while
            #print("lock open")
            
            # now we have 0.1 s to do this (cf.get_next_job())
            removed = self.job_list[minV:maxV]
            self.job_list = self.job_list[0:minV]+ self.job_list[maxV:]
            
            
            sOut = "removed:%d" % (maxV-minV)
            for x in removed:
                sOut = sOut + ":%s" % x
            #-- end for
        else:
            sOut = res
        #--- end if
        
        return sOut
    #-- end def


    #------------------------------------------------------------------------
    #-- list
    #--
    def list(self):
        s = "finished:%d" % len(self.finished)
        for x in self.finished:
            s = s+":%s" % x
        #-- end for
        s = s + "|todo:%d" % len(self.job_list)
        for x in self.job_list:
            s = s+":%s" % x
        #-- end for
        return s
    #-- end def

    
#-- end class


#-----------------------------------------------------------------------------
#-- test application
#--
if __name__ == '__main__':
    
    #jm = JobManager2(".", "*.py")
    jm = JobManager2("/home/jody/progs/QHG3/trunk/cloud_prepare/test_sims_fullC", "*.qdf")
    a=jm.splitInterval("[,]")
    print(a)
    print("---")
    s = jm.list()
    print(s)
    x=jm.drop("[3,7]")
    print(jm.job_list)
    bok = True
    while (bok):
        x = jm.get_next_job("groehl")
        if x is None:
           bok = False
        else:
           print("got job [%s]" % (x))
        #-- end if
    #-- end while
#-- end if
