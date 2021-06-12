#!/usr/bin/python3
'''
MQTT publish flow.

Author: rock <shirock.tw@gmail.com>
Web site: https://rocksaying.tw/
License: LGPL or CC BY-SA.

這是針對 NB-IoT 裝置寫的 MQTT 訊息發佈範例。
這些裝置可能沒有完整的 MQTT 函數庫；
或者不具有乙太網路介面，只能透過通訊模組的 AT 指令送出資料封包。

通訊模組使用者，請將本範例的 stream_socket_client(), fwrite(), fread() 換成你的 AT 指令。

http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/errata01/os/mqtt-v3.1.1-errata01-os-complete.html

以中華電信 IoT 平台為例: https://iot.cht.com.tw/iot/developer/mqtt
'''
import sys, os, time
import struct
import random
import socket
import json

mqtt_broker = 'iot.cht.com.tw:1883'
username = 'PROJECT_KEY or DEVICE_KEY' # 填入你的專案鑰匙或設備金鑰匙
password = 'PROJECT_KEY or DEVICE_KEY'
username = 'rock'
password = 'password'
client_id = "{}-{}".format(socket.gethostname(), os.getpid())

# 填入你的設備編號
topic = '/v1/device/${device_id}/rawdata'

msg = json.dumps([{
    'id': '123321', # 填入感測器編號
    'value': ["123", "456", "789"],
    'time': time.strftime('%Y-%m-%dT%H:%M:%S')
}])

qos = 0

#### 開始 MQTT 訊息發佈流程

data_count = 0

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_STREAM) # TCP
host, port = mqtt_broker.split(':')
sock.connect((host, int(port)))

def str_content(binstr):
    slen = len(binstr)
    msb = slen >> 8
    lsb = slen & 0xff
    return bytearray((msb, lsb)) + binstr

#==== begin CONNECT

variable_header = bytearray(b"\x00\x04MQTT\x04")

# 註: MQTT協定文件， byte 以 1 為基底， bit 以 0 為基底。
# byte 8: Flags
# bit 7: username
# bit 6: password
# bit 1: clean session
variable_header.append(128+64+2)

# byte 9 ~ 10: keep alive
# 10 seconds.
variable_header += b"\x00\x0A"

variable_header += str_content(client_id.encode())
variable_header += str_content(username.encode())
variable_header += str_content(password.encode())

# CONNECT cmd: 0x10
fixed_header = bytearray((0x10, len(variable_header)))

sock.send(fixed_header)
sock.send(variable_header)

rb = sock.recv(4)

# rb[0] is CONNACK cmd 0x20.
# rb[3] is error code.
if rb[0] == 0x20 and rb[3] == 0:
    print("connected")
else:
    print("failed to connect.")
    sys.exit(1)

data_count = len(fixed_header) + len(variable_header) + 4
#==== end CONNECT

#==== begin PUBLISH
payload = bytearray(str_content(topic.encode()))

if qos > 0:
    rand_id = random.randrange(1000, 60000)
    packet_id = bytearray((rand_id >> 8, rand_id & 0xFF))
    print("packet_id {} {}".format(packet_id[0], packet_id[1]))
    payload += packet_id

payload += msg.encode()

# publish 的 Remaining Length 是取摘要值。
payload_len = len(payload)
digest = bytearray()
while payload_len > 0:
    digit = payload_len % 128
    payload_len >>= 7
    if payload_len > 0:
        digit |= 0x80
    digest.append(digit)

# PUBLISH cmd: 0x30
# bit1~bit2 = qos
# 0x30 publish qos 0
# 0x32 publish qos 1
# 0x34 publish qos 2
cmd = 0x30 + (qos << 1)
#fixed_header = bytearray(struct.pack('B', cmd))
fixed_header = bytearray(chr(cmd).encode())
fixed_header += digest

sock.send(fixed_header)
sock.send(payload)
data_count += len(fixed_header) + len(payload)
#echo "message $msg \n";

if qos > 0:
    rb = sock.recv(4)
    #print(rb)
    data_count += 4
    # PUBACK 0x40 for qos 1
    # PUBREC 0x50 for qos 2
    if rb[0] == ((0x4 + (qos >> 1)) << 4) and packet_id == rb[2:4]:
        if qos == 1:
            print("published (qos 1)")
        else:
            print("publish rec (qos 2)")
    else:
        print("failed to publish")
        sys.exit(1)

    if qos == 2:
        fixed_header = b'\x62\x02' # PUBREL
        sock.send(fixed_header)
        sock.send(packet_id)
        rb = sock.recv(4)
        data_count += 8
        # 0x70 PUBCOMP
        if rb[0] == 0x70 and packet_id == rb[2:4]:
            print("publish complete (qos 2)")
else:
    print("publish (qos 0)")

#==== end PUBLISH

# DISCONNECT cmd: 0xE0
fixed_header = b"\xE0\x00"
sock.send(fixed_header)
data_count += 2;
print("disconnect")

print("總共使用了 {} bytes".format(data_count))
