import os, sys, pickle

def gmap (fn, list, degree=0):

    pid_list = []
    children = 0
    processed = 0
    pipe_list = []
    
    fed = 0
    if degree <= 0:
        degree = len (list)
        
    while processed < len (list):
        while children < degree and fed < len (list):
            pipe = os.pipe ()
            pid = os.fork ()
            if pid == 0:
                os.close (pipe[0])
                pickle.dump (fn (list[fed]), os.fdopen (pipe[1], "w"))
                sys.exit (0)
            else:
                os.close (pipe[1])
                children += 1
                pid_list.append (pid)
                pipe_list.append (pipe[0])
                fed += 1
            
        if children > 0 :
            try:
                wpid, sts = os.waitpid (-1, 0)
            except OSError:
                continue
            if wpid > 1:
                idx = pid_list.index (wpid)
                list[idx] = pickle.load (os.fdopen (pipe_list[idx]))
                children -= 1
                processed += 1

    return list


