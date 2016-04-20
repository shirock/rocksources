#!/usr/bin/python
# -*- coding: utf-8 -*-
# system packages
import os, sys, time, signal
reload(sys)
sys.setdefaultencoding('utf-8')

import gtk.glade
import gtk
import pygtk
pygtk.require('2.0')
import gobject, dbus, dbus.mainloop.glib

#CameraId = "file:///home/rock/test.mp4"
Camera_service = 'rock.guvcview.camera'
Camera_object = '/rock/guvcview/camera'
Camera_interface = 'rock.guvcview.camera'

mainloop = None

class View:
    def __init__(self):
        global mainloop
        glade_path = "/usr/local/lib/dbus-camera-uvc/uvc-camera-demo.glade"
        self.snap_count = 1

        try:
            self.UI = gtk.glade.XML(glade_path)
        except RuntimeError:
            glade_path = "%s/uvc-camera-demo.glade" % os.path.dirname(sys.argv[0])
            self.UI = gtk.glade.XML(glade_path)

        self.window = self.UI.get_widget('window1')
        self.UI.get_widget('drawingarea1').set_size_request(800, 600)
        self.UI.get_widget('drawingarea1').modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color(0, 0, 0))

        #錯誤。在 View 元件可見化(show)之前，沒有 xid 屬性。
        #self.UI.get_widget('label_xid').text = self.UI.get_widget('drawingarea1').window.xid

        event_map = {
            'on_window1_destroy': self.quit,
            'on_btn_quit_clicked': self.quit,
            'on_btn_open_clicked': self.on_btn_open_clicked,
            'on_btn_snapshot_clicked': self.on_btn_snapshot_clicked
        }
        self.UI.signal_autoconnect(event_map)

        try:
            self.bus = dbus.SessionBus()
            self.camera = self.bus.get_object(Camera_service, Camera_object)
            self.camera_iface = dbus.Interface(self.camera, Camera_interface)
            self.bus.add_signal_receiver(self.event_handler, dbus_interface = Camera_interface, signal_name = "Event")
        except Exception as e:
            print("Error: %s" % e)
            print("Please ensure your dbus-camera had started.")
            sys.exit(1)

        self.camera_id = None
        entry_mrl = self.UI.get_widget('entry_mrl')
        #entry_mrl.set_text('file:///tmp/電視王.mp4')
        #entry_mrl.set_text('file:///tmp/tvk.mp4')

    def on_btn_open_clicked(self, widget):
        entry_mrl = self.UI.get_widget('entry_mrl')
        entry_options = self.UI.get_widget('entry_options')

        if len(entry_mrl.get_text()) < 1:
            return

        if self.camera_id:
            self.camera_iface.StopCapture(self.camera_id)
            self.camera_id = None
            entry_mrl.set_text('')
            entry_options.set_text('')
            widget.set_label('Open')
            return

        self.camera_id = entry_mrl.get_text()
        print self.camera_id

        drawingarea1 = self.UI.get_widget('drawingarea1')
        if sys.platform == 'win32':
            visual_area_id = str(drawingarea1.window.handle)
        else:
            visual_area_id = str(drawingarea1.window.xid)
        print visual_area_id

        media_options = str(entry_options.get_text())
        self.camera_iface.EmbedMonitor(self.camera_id, str(visual_area_id), media_options)

        height, width = self.camera_iface.GetVideoSize(self.camera_id)
        if height:
            self.UI.get_widget('drawingarea1').set_size_request(height, width)
            widget.set_label('Close')


    def event_handler(self, camera_id, state):
        print("%s state: %s" % (camera_id, state))


    def on_btn_snapshot_clicked(self, widget):
        try:
            imgdata = self.camera_iface.GetFrame(self.camera_id, "jpg", byte_arrays=True)
        except IOError, err:
            print(err)
        else:
            # If you want byte arrays returned as dbus.ByteArray (also a subclass of str)
            # pass the keyword parameter byte_arrays=True to the proxy method.
            # See http://dbus.freedesktop.org/doc/dbus-python/doc/tutorial.html#basic-types
            #s = ''.join(chr(b) for b in imgdata)
            filepath = "snapshot-%d.jpg" % self.snap_count
            fh = open(filepath, 'wb')
            fh.write(imgdata)
            fh.close()
            print("Save snapshot to %s" % filepath)
            self.snap_count += 1

        #imgdata = self.camera_iface.GetFrame(self.camera_id, "jpg90", byte_arrays=True)
        #s = ''.join(chr(b) for b in imgdata)
        #filepath = "snapshot-90.jpg"
        #fh = open(filepath, 'wb')
        #fh.write(imgdata)
        #fh.close()

    def set_label_xid(self):
        #if sys.platform == 'win32':
        #    self.player.set_hwnd(self.window.handle)
        #else:
        #    self.player.set_xwindow(self.window.xid)
        drawingarea1 = self.UI.get_widget('drawingarea1')
        if sys.platform == 'win32':
            visual_area_id = str(drawingarea1.window.handle)
        else:
            visual_area_id = str(drawingarea1.window.xid)
        self.UI.get_widget('label_xid').set_text(visual_area_id)

    def quit(self, widget):
        print "Quit"
        if self.camera_id:
            self.camera_iface.StopCapture(self.camera_id)
        #gtk.main_quit()
        mainloop.quit()
        sys.exit(0)

if __name__ == "__main__":
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    mainloop = gobject.MainLoop()

    view = View()
    view.window.show_all()
    view.set_label_xid()

    #gtk.main()
    mainloop.run()

