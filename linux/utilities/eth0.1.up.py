#!/usr/bin/python
### BEGIN INIT INFO
# Provides: eth0.1.up
# Required-Start: network-manager   
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description: Raise eth0:1 network interface.
### END INIT INFO

# usage:
#   -daemon (or --daemon): run in background

LAN_SETTINGS = {
    'eth0': {
        'address': "10.1.2.3",
        'netmask': "255.255.255.0"
    }
}

ETH_V1_UP="/sbin/ifconfig %s:1 inet %s netmask %s up"

import os,sys
import dbus, gobject
from dbus.mainloop.glib import DBusGMainLoop
 
if len(sys.argv) > 1 and sys.argv[1].endswith("-daemon"):
    #run this program in background.
    try:
        pid = os.fork()
    except OSError:
        sys.exit(1)
    if pid > 0:
        sys.exit(0)

NetworkManagerServiceName = "org.freedesktop.NetworkManager"
NetworkManagerObjectPath = "/org/freedesktop/NetworkManager"
NetworkManagerInterface = "org.freedesktop.NetworkManager"
NetworkManagerDeviceInterface = "org.freedesktop.NetworkManager.Device"
NetworkManagerDeviceWiredInterface = "org.freedesktop.NetworkManager.Device.Wired"
DBusPropertiesInterface="org.freedesktop.DBus.Properties"
 
NM_DEVICE_STATE_ACTIVATED=8
 
class ActivatedHandler(object):
    def __init__(self, device_instance):
        self.device_instance = device_instance
        props = device_instance.GetAll(NetworkManagerDeviceInterface, 
            dbus_interface=DBusPropertiesInterface)
        self.name = props['Interface']
        self.device_instance.connect_to_signal("StateChanged", 
            self.handler, 
            dbus_interface=NetworkManagerDeviceInterface)

    def handler(self, new_state, old_state, reason):
        if new_state == NM_DEVICE_STATE_ACTIVATED:
            #print "%s activated" % self.name
            os.system(ETH_V1_UP % (self.name, 
                LAN_SETTINGS[self.name]['address'], 
                LAN_SETTINGS[self.name]['netmask']))
 
DBusGMainLoop(set_as_default=True)
loop = gobject.MainLoop()
 
bus = dbus.SystemBus()
try:
    nm_instance = bus.get_object(NetworkManagerServiceName, NetworkManagerObjectPath)
except dbus.DBusException:
    print "connect to NetworkManager error."
    sys.exit(1)
 
handlers = {}

def IntToDottedQuad(n):
    "convert long int to dotted quad string"
    
    d = 256 * 256 * 256
    q = []
    while d > 0:
        m,n = divmod(n,d)
        q.append(str(m))
        d = d/256
    q.reverse()
    return '.'.join(q)

def IntToNetmaskAddr(keep_bits):
    n = (0xffffffff >> (32 - keep_bits )) << (32 - keep_bits)
    d = 256 * 256 * 256
    q = []
    while d > 0:
        m,n = divmod(n,d)
        q.append(str(m))
        d = d/256
    return '.'.join(q)

address_list = []

devices = nm_instance.GetDevices(dbus_interface=NetworkManagerInterface)
for device in devices:
    device_instance = bus.get_object(NetworkManagerServiceName, device)
    device_props = device_instance.GetAll(NetworkManagerDeviceInterface, 
        dbus_interface=DBusPropertiesInterface)
    wired_props = device_instance.GetAll(NetworkManagerDeviceWiredInterface, 
        dbus_interface=DBusPropertiesInterface)
    device_name = str(device_props['Interface']) # eg. "eth0"
    ipv4_address = int(device_props['Ip4Address'])
    mac_address = str(wired_props['HwAddress'])

    ipv4config_instance = bus.get_object(NetworkManagerServiceName, device_props['Ip4Config'])
    ipv4_props = ipv4config_instance.GetAll('org.freedesktop.NetworkManager.IP4Config', 
        dbus_interface=DBusPropertiesInterface)

    print("%s %s %s" % (device_name, IntToDottedQuad(ipv4_address), mac_address))
    print(ipv4_props['Addresses'][0])
    for addr in ipv4_props['Addresses']:
        print(IntToDottedQuad(addr[0]))
        print(IntToDottedQuad(addr[1]))
        print(IntToDottedQuad(addr[2]))
    print(IntToDottedQuad(ipv4_props['Addresses'][0][2]))
    netmask = int(ipv4_props['Addresses'][0][1])
    #netmask = IntToNetmaskAddr(kb)

    address_list.append(
        [device_name, IntToDottedQuad(ipv4_address), IntToNetmaskAddr(netmask), mac_address])
    #if device_name not in LAN_SETTINGS:
    #    continue
    #if props['State'] == NM_DEVICE_STATE_ACTIVATED:
    #    #print "%s activated" % device_name
    #    os.system(ETH_V1_UP % (device_name, 
    #        LAN_SETTINGS[device_name]['address'], 
    #        LAN_SETTINGS[device_name]['netmask']))
    #handlers[device_name] = ActivatedHandler(device_instance)

#devices = False

#print "Working..."
#loop.run()
print address_list

