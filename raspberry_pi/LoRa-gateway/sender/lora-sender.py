#!/usr/bin/python3
# -*- coding: utf-8 -*-
"""
send a message

Usage: <device_path> <my_id> <target_id>
    device_path: LoRa device. 
        linux: /dev/serial0, etc.
        Windows: COM1, etc.
    my_id: set Header From
    target_id: set Header To
"""
import sys, os
import time
import array
import serial
# the path to pyrfm library.
sys.path.append(os.path.join(os.path.dirname(__file__),'..'))
import pyrfm

lora_settings = {
    'll':{
        'type':'rfm95'
    },
    'pl':{
        'type':	'serial_seed',
        'port':	'/dev/serial0'
    }
}

if len(sys.argv) < 4:
    print("Usage: <device_path> <my_id> <target_id>")
    sys.exit(0)

lora_settings['pl']['port'] = sys.argv[1]
my_id = int(sys.argv[2])
target_id = int(sys.argv[3])

ll=pyrfm.getLL(lora_settings)

print('HW-Version: ', ll.getVersion())
if ll.setOpModeSleep(True,True):
    ll.setFiFo()
    ll.setOpModeIdle()
    ll.setModemConfig('Bw125Cr45Sf128');
    #ll.setModemConfig('Bw31_25Cr48Sf512');  # will 3s timeout. maybe too slow.
    ll.setPreambleLength(8)
    ll.setFrequency(434)
    ll.setTxPower(13)
    
    ll.setHeaderTo(target_id)
    ll.setHeaderFrom(my_id)
    # <to> <from> <id> <flags>

    while True:
        data = time.strftime('%H:%M:%S')
        print("send %d bytes: %s" % (len(data), data))
        ll.sendStr(data)
        ll.waitPacketSent()

        time.sleep(3)
