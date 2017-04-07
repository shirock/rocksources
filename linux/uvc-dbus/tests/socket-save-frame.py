#!/usr/bin/python
import os, sys, socket

def usage():
    print("%s <camera id> <filepath>" % sys.argv[0])
    print("""
Arguments:
    * camera id:
      for example 'video0'.
    * filepath:
      where to save snapshot image?

Notice:
    * If camera id is 'list', it will list available cameras.
    * If camera is not opened, it will open and close after captured.
""" )
    sys.exit(0)

if len(sys.argv) < 2 :
    usage()

camera_id = sys.argv[1][-1]
filepath = sys.argv[2]

if filepath[0] != '/':
    filepath = '%s/%s' % (os.getcwd(), filepath);

#print '/tmp/uvc-service-socket-%s' % camera_id
#print 'save %s\n' % filepath

try:
    s = socket.socket(socket.AF_UNIX)
    s.connect('/tmp/uvc-service-socket-%s' % camera_id)
    s.recv(1024) # +OK
except socket.error as msg:
    print("Failed to use camera.")
    print msg
    if s:
        s.close()
    sys.exit(1)

try:
    s.sendall('save %s\n' % filepath)
    result = s.recv(1024)
    s.close()
except socket.error as msg:
    result = 'ERR'


if result.startswith('+OK'):
    print("Save snapshot to %s." % filepath)
    sys.exit(0)
else:
    print("Failed to save snapshot.")
    sys.exit(1)
