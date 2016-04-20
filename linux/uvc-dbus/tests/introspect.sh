#!/bin/sh

dbus-send --session --print-reply --dest=rock.guvcview.camera /rock/guvcview/camera org.freedesktop.DBus.Introspectable.Introspect

