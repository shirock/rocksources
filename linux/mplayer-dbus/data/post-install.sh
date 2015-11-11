#!/bin/sh

for pidf in service.pid service-system.pid service-session.pid ; do
    #echo /tmp/omxplayer-dbus-$pidf
    if [ -e /tmp/omxplayer-dbus-$pidf ]; then
        kill `cat /tmp/omxplayer-dbus-$pidf`
    fi
done

LOG=/var/log/omxplayer-dbus

chmod a+w $LOG

exit 0
