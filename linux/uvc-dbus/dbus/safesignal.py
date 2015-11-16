#!/usr/bin/python
# coding: utf-8
import os, sys, time, signal, multiprocessing
reload(sys)
sys.setdefaultencoding('utf-8')

class SignalSafeConnection(object):
    def __init__(self, handle):
        self.handle = handle

    def send(self, obj):
        # this is a traditional skill of unix programming,
        # but be careful to use it.
        while True:
            try:
                rc = self.handle.send(obj)
            except (OSError, IOError) as e:
                if e.errno == 4:
                    #print("SIGIO, Interrupted system call, restart")
                    continue
                else:
                    # unexcepted error, re-raise
                    raise
            else:
                break
        return rc

    def recv(self):
        while True:
            try:
                rc = self.handle.recv()
            except (OSError, IOError) as e:
                if e.errno == 4:
                    #print("SIGIO, Interrupted system call, restart")
                    continue
                else:
                    # unexcepted error, re-raise
                    raise
            else:
                break
        return rc

    def fileno(self):
        return self.handle.fileno()
    
    def close(self):
        self.handle.close()
        self.handle = None
    
    def poll(self, timeout=0):
        end_time = time.time() + timeout
        while True:
            try:
                rc = self.handle.poll(timeout)
            except (OSError, IOError) as e:
                if e.errno == 4:
                    current_time = time.time()
                    if current_time > end_time:
                        rc = False
                        break
                    #print("SIGIO, Interrupted system call, restart")
                    timeout = end_time - current_time
                    continue
                else:
                    # unexcepted error, re-raise
                    raise
            else:
                break
        return rc
    
    def send_bytes(self, buffer):
        while True:
            try:
                rc = self.handle.send_bytes(buffer)
            except (OSError, IOError) as e:
                if e.errno == 4:
                    #print("SIGIO, Interrupted system call, restart")
                    continue
                else:
                    # unexcepted error, re-raise
                    raise
            else:
                break
        return rc

    def recv_bytes(self):
        while True:
            try:
                rc = self.handle.recv_bytes()
            except (OSError, IOError) as e:
                if e.errno == 4:
                    #print("SIGIO, Interrupted system call, restart")
                    continue
                else:
                    # unexcepted error, re-raise
                    raise
            else:
                break
        return rc


def SignalSafePipe(duplex):
    """
    產生一組不會被 signal 中斷的 pipe 。
    """
    pipe1, pipe2 = multiprocessing.Pipe(duplex)
    sspipe1 = SignalSafeConnection(pipe1)
    sspipe2 = SignalSafeConnection(pipe2)
    return sspipe1, sspipe2

