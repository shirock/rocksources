#!/bin/sh

if [ "$1" = "help" ]; then
    echo "  Usage: $0 <CAMERA_ID>"
    echo
    exit 0
fi

if [ ! -z "$1" ]; then
    CAMERA=$1
fi

dbus-send --session --print-reply --dest=rock.guvcview.camera /rock/guvcview/camera rock.guvcview.camera.GetVideoSize string:"$CAMERA"
