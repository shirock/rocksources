#!/bin/sh

# dialout for R/W /dev/serial*.
usermod -a -G dialout pi

# systemctl enable lora-rf95-services

pip3 install paho-mqtt

exit 0