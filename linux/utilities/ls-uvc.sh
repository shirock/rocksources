#!/bin/sh
# list device path and product name of USB video devices (WebCam).
UVCROOT=/sys/bus/usb/drivers/uvcvideo

if [ ! -d $UVCROOT ]; then
    echo "No UVC device!"
    exit 0
fi

for FILE in `ls $UVCROOT`; do
    V4LPATH=$UVCROOT/$FILE/video4linux
    if [ -d $V4LPATH ]; then
        # there should be only one folder.
        DEVNAME=`ls $V4LPATH | grep video`
        PRODUCT_NAME=`cat $V4LPATH/$DEVNAME/name`
        echo "* /dev/$DEVNAME is \"${PRODUCT_NAME}\""
    fi
done
