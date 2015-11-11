#!/bin/sh

dbus-send --session --print-reply --dest=raspberry.pi.OMXPlayer \
  /raspberry/pi/OMXPlayer raspberry.pi.OMXPlayerList.List
