#!/bin/sh

dbus-send --system --print-reply --dest=raspberry.pi.OMXPlayer \
    $1 org.freedesktop.DBus.Properties.GetAll string:""
