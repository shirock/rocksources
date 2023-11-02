#!/bin/sh
IP=$(ip address show eth0|grep 'inet ' | awk '{print $2}'| awk -F/ '{print $1}')
echo $IP
