#!/bin/sh

dbus-send --session --print-reply --dest=raspberry.pi.OMXPlayer \
    $1 raspberry.pi.OMXPlayer.Stop
