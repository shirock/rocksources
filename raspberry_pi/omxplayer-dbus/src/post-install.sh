#!/bin/sh

for pidf in service.pid service-system.pid service-session.pid ; do
    #echo /tmp/omxplayer-dbus-$pidf
    if [ -e /tmp/omxplayer-dbus-$pidf ]; then
        kill `cat /tmp/omxplayer-dbus-$pidf`
    fi
done

LOG=/var/log/omxplayer-dbus

chown nobody $LOG
chgrp nogroup $LOG

echo "Add user 'nobody' to group 'video'."
addgroup nobody video  # It need to be video group to write vchiq.

exit 0
