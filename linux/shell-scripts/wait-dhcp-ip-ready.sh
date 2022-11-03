#!/bin/sh
# Execute program after dynamic IP is ready.
#
# For rc.d, rc.local
# Required: NetworkManager

/usr/bin/nm-online -s --timeout=30

# show assigned IP.
ip -4 -br address | grep UP

exit 0