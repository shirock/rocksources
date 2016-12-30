#!/bin/sh
SERVICE=mydaemon
INITD=/etc/init.d/$SERVICE

cp -n ../data/$SERVICE-logrotate    /etc/logrotate.d/$SERVICE
cp -n ../data/$SERVICE.conf         /etc/

if [ -x /sbin/insserv ]; then
  insserv $SERVICE
else
  update-rc.d $SERVICE defaults 90
fi

$INITD start

exit 0
