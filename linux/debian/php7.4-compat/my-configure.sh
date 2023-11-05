#!/bin/sh
SRCDIR=php-7.4.33
if [ -d $SRCDIR ]; then
    echo change directory to $SRCDIR
    cd $SRCDIR
fi
if [ ! -x configure ]; then
    echo configure is not executable.
    echo You should put $0 in $SRCDIR.
    exit 1
fi

export CFLAGS="-O2"
export OPENSSL_CFLAGS="-I/usr/local/include/openssl"
export OPENSSL_LIBS="-L/usr/local/lib/openssl -lcrypto -lssl"

./configure --prefix=/usr/local --enable-fpm \
    --with-apxs2 \
    --disable-debug \
    --enable-gd \
    --with-webp \
    --with-jpeg \
    --enable-bcmath \
    --enable-intl \
    --enable-mbstring \
    --with-openssl=/usr/local \
    --with-curl \
    --with-mysqli \
    --with-pdo-mysql \
    --enable-calendar \
    --enable-exif \
    --with-ffi \
    --with-gettext \
    --enable-opcache \
    --enable-pcntl \
    --with-pgsql \
    --with-readline \
    --enable-shmop \
    --enable-sockets \
    --with-sodium \
    --enable-sysvmsg \
    --enable-sysvsem \
    --enable-sysvshm \
    --with-zip \
    --with-zlib
