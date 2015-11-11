#!/bin/sh

for pidf in service.pid service-system.pid service-session.pid ; do
    #echo /tmp/omxplayer-dbus-$pidf
    if [ -e /tmp/omxplayer-dbus-$pidf ]; then
        kill `cat /tmp/omxplayer-dbus-$pidf`
    fi
done

#if [ -e /tmp/omxplayer-dbus-service.pid ]; then
#    kill `cat /tmp/omxplayer-dbus-service.pid`
#fi

rm -f /var/log/omxplayer-dbus/*

exit 0
