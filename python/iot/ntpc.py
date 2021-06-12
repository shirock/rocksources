#!/usr/bin/python3
'''
NTP client code.
NTP Protocol: UDP, port 123

Author: rock <shirock.tw@gmail.com>
Web site: https://rocksaying.tw/
License: LGPL or CC BY-SA.
'''
import socket
import struct
import time

def NTP_get_seconds(host="pool.ntp.org", port=123):
    '''
    @return seconds (UTC time)
    '''
    unix_epoch = 2208988800  # 1970-01-01 00:00:00

    # ntp packet: at least 48 bytes
    # head = 0x1B => 00,011,011 (li = 0, vn = 3, mode = 3)
    ntp_packet = b'\x1B' + b'\0' * 47 

    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client.sendto(ntp_packet, (host, port))
    data, _ = client.recvfrom(48)

    seconds = struct.unpack("!12I", data)[10] - unix_epoch
    return seconds

if __name__ == "__main__":
    try:
        seconds = NTP_get_seconds(host="time.stdtime.gov.tw")
    except Exception as e:
        print(e)
    else:
        gm_time = time.gmtime(seconds)
        local_time = time.localtime(seconds)
        time_format = "%Y:%m:%d %H:%M:%S"
        print("UTC Time:   {}".format(time.strftime(time_format, gm_time)))
        print("Local Time: {} {}".format(
            time.strftime(time_format, local_time),
            time.strftime("%z", local_time)))
