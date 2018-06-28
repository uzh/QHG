

from sys import argv
import socket

DEF_SIZE=1024
SEP='\0'

#------------------------------------------------------------------------
#-- class StringSock2
#-- 
class StringSock2:

    #------------------------------------------------------------------------
    #-- constructor
    #-- 
    def __init__(self, conn, recv_size=DEF_SIZE):
        self.conn  = conn
        self.recv_size = recv_size
        self.lines = []
        self.sRest = ''
        
    #-- end if

 
    #------------------------------------------------------------------------
    #-- show_buf
    #-- 
    def show_buf(self):
        for line in self.lines:
            print("  [%s]" % line)
        #-- end for
    #-- emd def


    #------------------------------------------------------------------------
    #-- close
    #-- 
    def close(self):
        self.conn.close()
    #-- emd def

    
    #------------------------------------------------------------------------
    #-- get_next_part
    #-- 
    def get_next_part(self):
        print("Have %d lines ready" % len(self.lines))
        while (not SEP in self.sRest):
            sNew = self.conn.recv(self.recv_size)
            self.sRest = self.sRest +sNew
        #-- end while
        
        new_lines = self.sRest.split(SEP)
        self.sRest = new_lines[-1]
        del new_lines[-1]
        self.lines.extend(new_lines)
        print("Now i have %d lines ready" % len(self.lines))
    #-- end def


    #------------------------------------------------------------------------
    #-- get_string
    #-- 
    def get_string(self):
        if (len(self.lines) == 0):
            self.get_next_part()
        #-- end if
        s = self.lines[0]
        del self.lines[0]
        return s
    #-- end if


    #------------------------------------------------------------------------
    #-- put_string
    #-- 
    def put_string(self, sMessage):
        print("(sending message [%s])" % (sMessage))
        
        sMessage = sMessage + SEP
        remaining = len(sMessage)
        sent = 0
        while (remaining > 0):
            sent = self.conn.send(sMessage[sent:])
            remaining = remaining - sent
        #-- end while
        print("sent it")
    #-- end if    

#-- end class
