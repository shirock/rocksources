#!/usr/bin/python
# coding: utf-8
"""
Python DBus example

Python DBus Tutorial:
http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html

Python DBus API:
http://dbus.freedesktop.org/doc/dbus-python/api/

Python DBus Source:
http://dbus.freedesktop.org/releases/dbus-python/

不幸的是，這份官方文件有許多疏漏。令人難以相信這是 Python 文件。
Tutorial 中的範例程式，放在 Source 中。需下載 Source 才能觀看官方教學文件的完整
範例程式。
"""

import os,sys
reload(sys)
sys.setdefaultencoding('utf-8')

import dbus, dbus.service

# setting up an event loop
from dbus.mainloop.glib import DBusGMainLoop
import gobject


class Hello(dbus.service.Object):
    """
    Hello service
    Inheriting from dbus.service.Object
    
    Service name: tw.idv.rock.sample.Hello
    
    http://dbus.freedesktop.org/doc/dbus-python/api/dbus.service.Object-class.html
    API文件範例打錯了，此類別的名稱是首字大寫 dbus.service.Object ，不是 
    dbus.service.object
    """

    name = "tw.idv.rock.sample.Hello"
    path = '/' + name.replace('.', '/')
    interface = name

    def __init__(self, event_loop):
        """
        http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#inheriting-from-dbus-service-object
        教學文件打錯了，第三行的 path 應該是 object_path。
        同時，它漏了 BusName 。
        若按照教學文件的寫法，因為沒有指定 bus name ，故實際上是無用的。
        """
        self.bus = dbus.SessionBus()
        #self.bus = dbus.SystemBus()
        self.event_loop = event_loop

        bus_name = dbus.service.BusName(Hello.name, bus=self.bus)
        dbus.service.Object.__init__(self, bus_name, Hello.path)
        
    ####
    #
    # Exporting method
    #
    # Tutorial: http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#exporting-methods-with-dbus-service-method
    # API: http://dbus.freedesktop.org/doc/dbus-python/api/dbus.service-module.html#method
    # Signature of arguments: http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#data-types
    ####
    @dbus.service.method('tw.idv.rock.sample.Hello',
                         in_signature='s', out_signature='s')
    def SayS(self, message):
        """
        Say message

        教學與範例文件打錯了，API 文件寫明 dbus.service.method 的第一個參數是位
        置參數，不是關鍵字參數，所以不能寫成 interface="..."
        """
        #self._notify(title="say", message=message, timeout=3)

        # emmit signal
        #self.SignalSay(message, 3)
        #return "I say " + message
        return "Size %d" % len(message)

    @dbus.service.method('tw.idv.rock.sample.Hello',
                         in_signature='ay', out_signature='s')
    def SayAy(self, message):
        """
        Say message

        教學與範例文件打錯了，API 文件寫明 dbus.service.method 的第一個參數是位
        置參數，不是關鍵字參數，所以不能寫成 interface="..."
        """
        #self._notify(title="say", message=message, timeout=3)

        # emmit signal
        #self.SignalSay(message, 3)
        #return "I say " + message
        return "Size %d" % len(message)

    @dbus.service.method('tw.idv.rock.sample.Hello',
                         in_signature='s', out_signature='ay')
    def RetAy(self, filepath):
        fh = open(filepath);
        data = fh.read()
        fh.close()
        return data

    @dbus.service.method('tw.idv.rock.sample.Hello')
    def Stop(self):
        """
        Stop
        """
        self.event_loop.quit()

    ####
    #
    # Exporting signal
    # 
    # Tutorial: http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#emitting-signals-with-dbus-service-signal
    # API: http://dbus.freedesktop.org/doc/dbus-python/api/dbus.service-module.html#signal
    ####
    @dbus.service.signal('tw.idv.rock.sample.Hello', signature='su')
    def SignalSay(self, message, timeout):
        """
        教學與範例文件打錯了，API 文件寫明 dbus.service.signal 的第一個參數是位
        置參數，不是關鍵字參數，所以不能寫成 interface="..."
        """
        print message
        pass


class SignalRecipient:
    def __init__(self):
        """
        Signal 是廣播訊息。
        向 DBus 註冊訊息接收者時，通常會設定訊息過濾條件，否則所有 signal 都會
        灌過來。
        一般指定 signal of service (by dbus_interface and signal_name) 為過濾條
        件。
        """
        #self.dbus_object.connect_to_signal("SignalSay", self._ss, dbus_interface=Hello.interface, arg0="Hello")
        self.bus = dbus.SystemBus()
        self.bus.add_signal_receiver(self.handler, dbus_interface=Hello.interface, signal_name = "SignalSay")
        
    def handler(self, message, timeout):
        print "Signal recivied: %s, %d" % (message, timeout)


if __name__ == "__main__":
    # You must do this before connecting to the bus.
    # http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#setting-up-an-event-loop
    DBusGMainLoop(set_as_default=True)
    loop = gobject.MainLoop()

    service = Hello(loop)
    recipient = SignalRecipient()

    print "Working..."

    # startup event loop
    loop.run()


