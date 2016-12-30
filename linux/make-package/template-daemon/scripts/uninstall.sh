#!/bin/sh
SERVICE=mydaemon
INITD=/etc/init.d/$SERVICE

$INITD stop

if [ -x /sbin/insserv ]; then
  insserv -r $SERVICE
else
  update-rc.d -f $SERVICE remove
fi

rm -f /etc/logrotate.d/$SERVICE

exit 0
