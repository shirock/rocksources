#!/bin/sh
# bookworm does not have libssl1.1 (it has libssl3)
BULLSEYE_VER=1.1.1w-0+deb11u1
if [ ! -e libssl-dev_$BULLSEYE_VER_amd64.deb ]; then
    # compiled .so
	# apt-get download libssl1.1=$BULLSEYE_VER  
    wget https://free.nchc.org.tw/debian/pool/main/o/openssl/libssl1.1_1.1.1w-0+deb11u1_amd64.deb

    # header files.
	# apt-get download libssl-dev=$BULLSEYE_VER
    wget https://free.nchc.org.tw/debian/pool/main/o/openssl/libssl-dev_1.1.1w-0+deb11u1_amd64.deb
fi

dpkg -x libssl-dev_${BULLSEYE_VER}_amd64.deb libssl
dpkg -x libssl1.1_${BULLSEYE_VER}_amd64.deb  libssl

# copy to /usr/local
PREFIX=/usr/local

if [ ! -d /usr/local/include ]; then
    sudo mkdir -p /usr/local/include
fi
sudo cp -r libssl/usr/include/*  /usr/local/include

if [ ! -d /usr/local/lib/openssl ]; then
    sudo mkdir -p /usr/local/lib/openssl
fi
sudo cp -r libssl/usr/lib/x86_64-linux-gnu/* /usr/local/lib/openssl

echo gcc flags: -I/usr/local/openssl
echo ld flags: -L/usr/local/lib/openssl -lcrypto -lssl
