#!/bin/sh

dbus-send --session --print-reply --dest=rock.guvcview.camera \
    /rock/guvcview/camera rock.guvcview.camera.StartMonitor \
    string:"$1" int32:$2 int32:$3 int32:$4 int32:$5 string:""
