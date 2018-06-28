#!/usr/bin/python

from sys import argv,stdout
import socket
import StringSock2 as ssock

if len(argv) > 3:
    host = argv[1]
    port = int(argv[2])
    cmd  = argv[3]
    
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((host, port))
    strsock = ssock.StringSock2(conn)
    
    cmd = "cmd:"+cmd
    print("command:[%s]" % (cmd))
    strsock.put_string(cmd)

    smess = strsock.get_string()
    #-- end for

    print(smess)
else:
    print("command interface for Controller2")
    print("usage:")
    print("  %s <host> <port> <command>" % argv[0])
    print("where:")
    print("  host     host to connect to")
    print("  port     port over which to connect")
    print("  command  command to be sent:")
    print("             stop:all            stop all comms")
    print("             stop:<host>         stop the comm for <host>")
    print("             job:list            list all jobs")
    print("             job:reload          reload jobs from same directory as before")
    print("             job:add:dir         load jobs from directory dir")
    print("             job:drop:[min,max]  drop entries from min to and including max from job list")
#-- end if
