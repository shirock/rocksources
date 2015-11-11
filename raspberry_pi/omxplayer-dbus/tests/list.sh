#!/bin/sh

dbus-send --system --print-reply --dest=raspberry.pi.OMXPlayer \
  /raspberry/pi/OMXPlayer raspberry.pi.OMXPlayerList.List
