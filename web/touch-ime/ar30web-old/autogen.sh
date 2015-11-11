#!/bin/sh

for REQUIRED in php5-cli ; do
    echo -n "checking required package $REQUIRED ... "
    dpkg -l $REQUIRED | grep ^ii > /dev/null
    if [ $? -ne 0 ]; then
        echo "[no]"
        echo "Please install required packages."
        exit 1
    fi
    echo "[yes]"
done

echo -n "generating main word tables... "
php ar30_gen.php
if [ $? -ne 0 ]; then
    echo "[failed]"
    exit 1
fi
echo "[succeed]"

echo -n "generating hard word tables... "
php ar30_hw_gen.php
if [ $? -ne 0 ]; then
    echo "[failed]"
    exit 1
fi
echo "[succeed]"

echo -n "generating index2.html (word table is embedded)... "
php ar30_embed.php
if [ $? -ne 0 ]; then
    echo "[failed]"
    exit 1
fi
echo "[succeed]"

cd tbl
ls *.tab > list

echo 
echo "Configure ok."
echo 

exit 0

