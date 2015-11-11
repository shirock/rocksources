#!/bin/sh

dbus-send --session --print-reply --dest=raspberry.pi.OMXPlayer \
    /raspberry/pi/OMXPlayer raspberry.pi.OMXPlayerList.Play \
    string:"$1" int32:$2 int32:$3 int32:$4 int32:$5 boolean:false string:"$6"

