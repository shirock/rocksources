#!/usr/bin/python

import os, sys, time
import dbus

Camera_service = 'rock.guvcview.camera'
Camera_object = '/rock/guvcview/camera'
Camera_interface = 'rock.guvcview.camera'

def usage():
    print("%s <camera id> <filepath>" % sys.argv[0])
    print("""
Arguments:
    * camera id:
      for example '/dev/video0'.
    * filepath:
      where to save snapshot image?

Notice:
    * If camera id is 'list', it will list available cameras.
    * If camera is not opened, it will open and close after captured.
""" )
    sys.exit(0)

if len(sys.argv) < 2 :
    usage()

bus = dbus.SessionBus()

camera = bus.get_object(Camera_service, Camera_object)
camera_iface = dbus.Interface(camera, Camera_interface)

camera_list = camera_iface.List()

if sys.argv[1] == 'list':
    for c in camera_list:
        print(c)
    sys.exit(0)

camera_id = sys.argv[1]
filepath = sys.argv[2]

close_camera = False
if camera_id not in camera_list:
    camera_iface.StartCapture(camera_id)
    close_camera = True
    time.sleep(1.0)

result = camera_iface.SaveFrame(camera_id, filepath)
#result = camera_iface.GetFrame(camera_id, "jpg", byte_arrays=True)

if close_camera:
    camera_iface.StopCapture(camera_id)

if result:
    #with open(filepath, 'w') as fh:
    #    fh.write(result)
    print("Save snapshot to %s." % filepath)
    sys.exit(0)
else:
    print("Failed to save snapshot.")
    sys.exit(1)

