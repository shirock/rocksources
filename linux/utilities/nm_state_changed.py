#!/usr/bin/python
### BEGIN INIT INFO
# Provides: eth0.1.up
# Required-Start: network-manager   
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description: Raise eth0:1 network interface.
### END INIT INFO

#
# usage:
#   加上參數 -daemon (或 --daemon) 會自動放到背景執行。
#
 
ETHTOOL_UP="/sbin/ifconfig %s"
 
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
            print "%s activated" % self.name
            os.system(ETHTOOL_UP % self.name)
 
DBusGMainLoop(set_as_default=True)
loop = gobject.MainLoop()
 
bus = dbus.SystemBus()
try:
    nm_instance = bus.get_object(NetworkManagerServiceName, NetworkManagerObjectPath)
except dbus.DBusException:
    print "connect to NetworkManager error."
    sys.exit(1)
 
handlers = {}

devices = nm_instance.GetDevices(dbus_interface=NetworkManagerInterface)
for device in devices:
    device_instance = bus.get_object(NetworkManagerServiceName, device)
    props = device_instance.GetAll(NetworkManagerDeviceInterface, 
        dbus_interface=DBusPropertiesInterface)
    device_name = props['Interface'] # eg. "eth0"
    if props['State'] == NM_DEVICE_STATE_ACTIVATED:
        print "%s activated" % device_name
        os.system(ETHTOOL_UP % device_name)
    handlers[device_name] = ActivatedHandler(device_instance)

devices = False

#print "Working..."
loop.run()

