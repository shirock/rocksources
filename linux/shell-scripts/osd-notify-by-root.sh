#!/bin/sh
DESKTOPUSER=rock
DESKTOPUID=1000

# if root runs a shell script and wants to notify the user on the desktop,
# it should specify environment DISPLAY and DBUS_SESSION_BUS_ADDRESS.
NOTIFY="sudo -u $DESKTOPUSER DISPLAY=:0 DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$DESKTOPUID/bus notify-send Summary "

EUID=`id -u`

if [ $EUID = "0" ]; then
    $NOTIFY "This script is run by root"
else
    notify-send Summary "This script is run by $USER"
fi
