#!/bin/sh
ldconfig
systemctl enable php7.4-fpm.service
systemctl start  php7.4-fpm.service