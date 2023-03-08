#!/bin/sh
# show lastest log archive.

# log.*.zip are log.20220101.zip, log.20220102.zip, etc.
LASTEST=`ls -r log.*.zip | head -1`

# unzip with -p will extract file to pipe.
unzip -p $LASTEST | more
