#!/bin/sh

dbus-send --session --print-reply --dest=raspberry.pi.OMXPlayer \
  /raspberry/pi/OMXPlayer org.freedesktop.DBus.Introspectable.Introspect

