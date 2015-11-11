#!/bin/sh

dbus-send --system --print-reply --dest=raspberry.pi.OMXPlayer \
    $1 raspberry.pi.OMXPlayer.SeekBack string:"$2"

