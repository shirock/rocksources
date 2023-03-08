#!/bin/sh
for SD in $(ls /dev/sd* | grep -E "sd[a-z][0-9]+"); do
    fsck -fy $SD
done
