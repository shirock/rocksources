#!/usr/bin/python
'''
List device path and product name of USB video devices (WebCam).
'''
import os
from glob import glob

def list_uvc_devices():
    uvc_root = '/sys/bus/usb/drivers/uvcvideo'

    results = []
    for fn in glob('%s/*' % uvc_root):
        v4l_path = '%s/video4linux' % fn
        if not os.path.isdir(v4l_path):
            continue

        _ = glob('%s/video*' % v4l_path)
        dev_name = os.path.basename(_[0])
        dev_path = '/dev/%s' % dev_name

        _ = '%s/%s/name' % (v4l_path, dev_name)
        product_name = open(_).readline().strip()

        results.append((dev_path, product_name))

    return results


for dev_path, product_name in list_uvc_devices():
    print '* %s is "%s"' % (dev_path, product_name)
