#!/usr/bin/python3
# -*- coding: utf-8 -*-
'''
waits for incomming message and publish
configuration: /etc/rf95_server.conf or $HOME/etc/rf95_server.conf

若要以一般使用者身份開啟 serial port:
group dialout for Read/Write /dev/serial*, run command:
$ sudo usermod -a -G dialout pi

'''
import sys, os, signal, time
import array
import threading, queue
import configparser, logging
import paho.mqtt.publish as publish
from socket import gethostname
# the path to pyrfm library.
#sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
import pyrfm

if os.getuid() > 0:
    os.chdir(os.path.expanduser('~'))
else:
    os.chdir('/') # root

mqtt_settings = {
    'host': "localhost",
    'topic_base': "tw/rocksaying/lora",
    'username': "guest", 
    # username should not be empty. MQTT will throw 'no username key'.
    'password': "",
    'retry_interval': 180
}

lora_settings = {
    'id': 255, # my lora id.
    'll':{
        'type':'rfm95'
    },
    'pl':{
        'type': 'serial_seed',
        'port': '/dev/serial0'
    }
}

log_settings = {
    'log_dir': '/var/log',
    'pid_dir': '/var/run',
    'debug': 'no'
}

if os.getuid() > 0:
    log_settings['log_dir'] = 'log'
    log_settings['pid_dir'] = 'log'

'''
[LoRa]
id = 255
port = /dev/serial0

[MQTT]
host =
topic_base =
username = guest
# username should not be empty
password =

[Log]
debug = no
;log_dir = log
;pid_dir = log
'''
config = configparser.ConfigParser()
config.read('etc/rf95_server.conf')

if 'MQTT' in config:
    for k in config['MQTT']:
        mqtt_settings[k] = config['MQTT'][k]
    if 'retry_interval' in config['MQTT']:
        mqtt_settings['retry_interval'] = config.getint('MQTT', 'retry_interval')

if 'LoRa' in config:
    for k in config['LoRa']:
        lora_settings['pl'][k] = config['LoRa'][k]
    if 'id' in config['LoRa']:
        lora_settings['id'] = config.getint('LoRa', 'id')

log_lv = logging.INFO
if 'Log' in config:
    for k in config['Log']:
        log_settings[k] = config['Log'][k]
    try:
        if config.getboolean('Log', 'debug'):
            log_lv = logging.DEBUG
    except:
        pass

log_fmt = '[%(asctime)s][%(levelname)s] %(message)s'
log_datefmt = '%Y-%m-%d %H:%M:%S'
log_dir = log_settings['log_dir']
if os.access(log_dir, os.W_OK):
    log_filepath = '%s/rf95_server.log' % (log_dir)
    logging.basicConfig(filename=log_filepath, level=log_lv, format=log_fmt, datefmt=log_datefmt)
else:
    logging.basicConfig(level=log_lv, format=log_fmt, datefmt=log_datefmt)


publishing_messages = queue.Queue(10000)
service_looping = False

def publish_state(data):
    global mqtt_settings
    topic = '%s/gateway' % mqtt_settings['topic_base']
    msg = "%s %s %s" % (gethostname(), time.strftime('%m%d%H%M'), data)
    try:
        publish.single(topic, msg, qos=0,
            hostname=mqtt_settings['host'], client_id=gethostname(), auth=mqtt_settings)
    except Exception as e:
        print(e)


def heartbeat():
    while True:
        print("heartbeat")
        publish_state('alive')
        time.sleep(mqtt_settings['retry_interval'])


def append_publishing_message(topic, id, data):
    if publishing_messages.full():
        # drop oldest message.
        publishing_messages.get()
        publishing_messages.task_done()
    publishing_messages.put((topic, id, data))


def publish_worker():
    '''
    daemon thread.
    它會從 publishing_messages 中取出一筆待發佈訊息 (如果沒有則等待)。
    接著會嘗試發佈這一筆訊息，直到成功或 publishing_messages 滿了為止。
    在這一筆訊息發佈成功前，主執行緒仍然可以持續往 publishing_messages 中增加訊息。
    若發佈失敗，且 publishing_messages 已滿，就會放棄這筆訊息，取出下一筆訊息。
    否則等待重試。
    '''
    topic = None
    data = None
    sensor_id = 0

    while True:
        if data == None or publishing_messages.full(): # 佇列已滿時，則放棄尚未發佈的舊訊息
            topic, sensor_id, data = publishing_messages.get()
            publishing_messages.task_done()
            print("publish...")
            print(data)
            # 待發佈訊息已由 data 保存，故現在就可釋放佇列，讓主執行緒繼續堆放訊息。

        if not topic:
            publish_state(data)
            data = None
            continue

        full_topic = "%s/sensor/%s/%d" % (mqtt_settings['topic_base'], topic, sensor_id)
        # mqtt_auth = {
        #     'username': mqtt_settings['username'],
        #     'password': mqtt_settings['password']
        # }
        try:
            publish.single(full_topic, data, qos=1,
                hostname=mqtt_settings['host'], client_id=gethostname(), auth=mqtt_settings)
            print("published")
            topic = None
            data = None
            sensor_id = 0
        except Exception as e:
            logging.error("%s Retry publish later." % e)
            time.sleep(mqtt_settings['retry_interval'])


def stop_all(*args):
    global service_looping
    service_looping = False


signal.signal(signal.SIGTERM, stop_all)
signal.signal(signal.SIGINT,  stop_all)  # Ctrl-C

publish_thread = threading.Thread(target=publish_worker, daemon=True)
publish_thread.start()

heartbeat_thread = threading.Thread(target=heartbeat, daemon=True)
heartbeat_thread.start()


if len(sys.argv) >= 2:
    lora_settings['pl']['port'] = sys.argv[1]


ll = pyrfm.getLL(lora_settings)

try:
    print('HW-Version: ', ll.getVersion())
except Exception as e:
    #print(e)
    logging.info(e)
    sys.exit(0)
    
logging.info('LoRa HW-Version: %s' % ll.getVersion())
logging.info('LoRa Port: %s' % lora_settings['pl']['port'])
#print("LoRa ID: %d" % lora_settings['id'])
logging.info('LoRa ID: %d' % lora_settings['id'])
if log_lv == logging.DEBUG:
    logmsg = "Debug mode: On"
else:
    logmsg = "Debug mode: Off"
logging.info(logmsg)

if ll.setOpModeSleep(True, True):
    ll.setFiFo()
    ll.setOpModeIdle()
    ll.setModemConfig('Bw125Cr45Sf128');
    #ll.setModemConfig('Bw31_25Cr48Sf512');  # slow/long, but 3 seconds timeout
    ll.setPreambleLength(8)
    ll.setFrequency(434)
    ll.setTxPower(13) # useRFO default is false.

    service_looping = True
    logmsg = 'working [pid:%d]' % os.getpid()
    print(logmsg)
    logging.info(logmsg)
    try:
        open(log_settings['pid_dir'] + '/rf95_server.pid', 'w').write("%d" % os.getpid())
    except:
        pass

    while service_looping:
        if ll.waitRX(timeout=3):
            data = ll.recv()

            header = data[0:4]
            # <to> <from> <id> <flags>
            msg = array.array('B', data[4:]).tostring() 
            # NOTICE type of msg is bytes, not str.
            print("read %d bytes" % len(data))
            print("header: ", header)
            print("message: ", msg)
            #publishing_messages.put((None, 0, "read %d bytes" % len(data)))

            from_id = header[1]

            if header[0] != lora_settings['id']:
                print("recv_id is not matched")
                continue

            #print("TODO received task")

            # NOTICE 多數 sensor 未內建 RTC 不能保證時鐘準確。
            # 加上一行時間記錄，以 gateway 時鐘為準。
            #tz_offset = time.strftime('%z')
            #tz_offset = tz_offset[0:3] + ':' + tz_offset[3:]
            #msg += b'\n' + '{0}{1}'.format(time.strftime('%Y-%m-%dT%H:%M:%S'), tz_offset).encode()

            append_publishing_message("raw", from_id, msg)
        #print("...")

    try:
        os.remove(log_settings['pid_dir'] + '/rf95_server.pid')
    except:
        pass
    sys.exit(0)
