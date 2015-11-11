#!/bin/sh
APACHE_SESSION_NAME=$LOGNAME
if [ -z "$APACHE_SESSION_NAME" ]; then
    APACHE_SESSION_NAME=$USER
fi

UID=`id -u`
if [ $UID -eq 0 ] || [ -z "$APACHE_SESSION_NAME" ]; then
    echo "No session and No root."
    exit 1
fi

export APACHE_LISTEN_PORT="1080"
export APACHE_LOG_DIR="$HOME/log"
export APACHE_LOG_LEVEL="error"
export APACHE_DOC_DIR="$HOME/Public"

ENVVARS=$HOME/.config/apache-envvars
if [ -e $ENVVARS ]; then
    . $ENVVARS
fi

PIDFILE="$APACHE_LOG_DIR/apache2.pid"
GET_PID="cat $PIDFILE"

/usr/sbin/apache2 -v|grep "Apache/2.4" > /dev/null
if [ $? -eq 0 ]; then
    CONFIG_FILE="apache24.conf"
else
    CONFIG_FILE="apache22.conf"
fi

case $1 in
	start)
	    if [ -e $PIDFILE ]; then
            kill `$GET_PID`
            sleep 1
        fi
        #/usr/sbin/apache2 -DFOREGROUND -DNO_DETACH /etc/apache2/session/${CONFIG_FILE}
        /usr/sbin/apache2 -f /etc/apache2/session/${CONFIG_FILE}
	;;
	stop)
        kill `$GET_PID`
	;;
	reload | restart)
	    kill -HUP `$GET_PID`
	;;
	*)
	    echo "$0 [start|stop|reload]"
	    echo
	;;
esac

