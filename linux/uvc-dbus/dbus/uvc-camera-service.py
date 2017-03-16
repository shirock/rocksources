#!/usr/bin/python
# coding: utf-8
#
# D-Bus Camera Service with guvcview
# Copyright (C) 2013 shirock.tw@gmail.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
MY_LIB_PATH='/usr/local/lib/dbus-camera-uvc'

import os, sys, time, getopt, signal, threading, multiprocessing, subprocess
reload(sys)
sys.setdefaultencoding('utf-8')
sys.path.append(MY_LIB_PATH)
import gobject, dbus, dbus.service
from dbus.mainloop.glib import DBusGMainLoop  # setting up an event loop
import base64, re, json
from safesignal import *
import ctypes
from ctypes.util import find_library


MAIN_PID = os.getpid()

Service = None

class CameraService(dbus.service.Object):
    '''
    Camera DBus service

    DBus type: session
    Service name: rock.guvcview.camera
    Object path: /rock/guvcview/camera
    Interface: rock.guvcview.camera
    '''
    Name =      "rock.guvcview.camera"
    Path =     "/rock/guvcview/camera"
    Interface = "rock.guvcview.camera"

    BusType = 'session'

    Max_Wait_Seconds = 5
    Alive_Detect_Interval = 10

    def __init__(self, bus, event_loop):
        self.bus = bus
        self.event_loop = event_loop

        # It contains Camera objects.
        # cameras = { camera_id: Camera }
        self.cameras = {}

        # It contains pending processes.
        # { pid: start_time }
        self.pending_processes = {}

        # SIGVTALRM and SIGPROF do not work.
        signal.setitimer(signal.ITIMER_REAL, CameraService.Alive_Detect_Interval, CameraService.Alive_Detect_Interval)
        signal.signal(signal.SIGALRM, CameraService.actively_joining_players)

        signal.signal(signal.SIGCHLD, CameraService.actively_joining_players)

        bus_name = dbus.service.BusName(CameraService.Name, bus=self.bus)
        dbus.service.Object.__init__(self, bus_name, CameraService.Path)

        signal.signal( signal.SIGHUP,  terminate_self)
        signal.signal( signal.SIGTERM, terminate_self)
        signal.signal( signal.SIGINT, terminate_self)
        signal.signal( signal.SIGQUIT, terminate_self)
        signal.signal( signal.SIGSEGV, terminate_self)

    Request_Time = False
    Request_Count = 0

    @classmethod
    def start_request_alarm(self):
        debug("start request alarm")
        CameraService.Request_Time = time.time()

    @classmethod
    def stop_request_alarm(self):
        debug("end request alarm")
        CameraService.Request_Time = False

    @classmethod
    def is_camera_response_timeout(self):
        if not CameraService.Request_Time:
            return False
        if time.time() > CameraService.Request_Time + CameraService.Max_Wait_Seconds + 1:
            return True
        return False

    @classmethod
    def actively_joining_players(self, signum, func):
        #debug("active join players, per %d seconds" % CameraService.Alive_Detect_Interval)
        Service.joining_player()
        if CameraService.is_camera_response_timeout():
            log("Critical error, terminate service.")
            Service.terminate_all_cameras()
            #remove_pid_file()
            os._exit(1)

    def remove_camera(self, camera):
        camera_id = camera.id
        try:
            # unregister object path of this camera.
            path = self.camera_object_path(camera)
            self.cameras[camera_id].remove_from_connection(
                self.bus, path)
            del self.cameras[camera_id]
        except KeyError:
            # self.joining_player() 也隨時在清除 cameras，故此處刪除時，有可
            # 能因為 self.joining_player() 已經先移除此元素，而發生 KeyError.
            pass
        except Exception as e:
            log("Exception: %s" % e)
        else:
            Service.Event(camera_id, "camera_closed")

    def add_pending(self, pid):
        self.pending_processes[pid] = time.time()

    def del_pending(self, pid):
        try:
            del self.pending_processes[pid]
        except:
            pass

    def is_pending_timeout(self, pid):
        if time.time() > self.pending_processes[pid] + CameraService.Alive_Detect_Interval:
            return True
        return False

    def kill_camera_process(self, pid):
        try:
            os.kill(pid, signal.SIGKILL)
        except:
            pass

    def joining_player(self):
        #See http://docs.python.org/library/multiprocessing.html#all-platforms
        #    Joining zombie processes
        if MAIN_PID != os.getpid():
            return
        cl = multiprocessing.active_children()
            #debug("length of active children: %d; length of cameras: %d" %
            #    (len(cl), len(self.cameras)))
        for camera_id in self.cameras.keys():
            camera = self.cameras[camera_id]
            if not camera.is_alive():
                self.remove_camera(camera)

        for p in self.pending_processes.keys():
            if self.is_pending_timeout(p):
                self.kill_camera_process(p)
                self.del_pending(p)


    @dbus.service.method('rock.guvcview.camera', out_signature='a{su}')
    def List(self):
        '''
        列出目前運作中的 camera ID 以及使用數。
        '''
        self.joining_player()
        ids = self.cameras.keys()
        lists = {}
        for id in ids:
            lists[id] = self.cameras[id].reference
        return lists

    @dbus.service.method('rock.guvcview.camera', out_signature='a{os}')
    def ListPath(self):
        '''
        列出目前活動中的 object path 及其 camera ID。
        '''
        self.joining_player()
        ids = self.cameras.keys()
        lists = {}
        for id in ids:
            path = self.camera_object_path(id)
            if path:
                lists[path] = id
        return lists

    def camera_object_path(self, camera):
        if isinstance(camera, (multiprocessing.Process, multiprocessing.process.Process)):
            pid = camera.pid
        else:
            try:
                pid = self.cameras[camera].pid
            except:
                #log("can not get object path.")
                return None
        return CameraService.Path + "/" + str(pid)

    def connect_bus(self, camera):
        path = self.camera_object_path(camera)
        if path:
            camera.add_to_connection(self.bus, path)
        return path

    def _load_media(self, camera_id, visual_id, autoplay, options):
        if self.IsCapturing(camera_id):
            self.cameras[camera_id].increase_reference()
            log("Start %s: It is already connectting, just increase count of reference [Current:  %d]" % (camera_id, self.cameras[camera_id].reference))
            return self.cameras[camera_id], self.camera_object_path(camera_id)

        #if type(visual_id) != type(tuple()):
        #    visual_id = int(visual_id)
        camera = Camera(camera_id, visual = visual_id, autoplay=autoplay, options=options)
        self.cameras[camera_id] = camera

        path = self.connect_bus(camera)
        if not path:
            raise IOError("Can not play %s. Connect object path failed." % camera_id)
        log("Load Media %s" % camera_id)
        return camera, path

    @dbus.service.method('rock.guvcview.camera', in_signature='s', out_signature='o')
    def StartCapture(self, camera_id):
        '''
        Start capture without monitor window.
        ShowWindow/HideWindow/ResizeWindow will not work.
        '''
        camera_id = camera_id.encode('utf-8')
        self.joining_player()

        # cameras = { camera_id: Camera }
        if self.IsCapturing(camera_id):
            self.cameras[camera_id].increase_reference()
            log("Start %s: It is already connectting, just increase count of reference [Current:  %d]" % (camera_id, self.cameras[camera_id].reference))
            return self.camera_object_path(camera_id)

        camera = Camera(camera_id)
        self.cameras[camera_id] = camera

        path = self.connect_bus(camera)
        if not path:
            raise IOError("Can not play %s. Connect object path failed." % camera_id)
        log("Start Capture %s" % camera_id)
        return path

    @dbus.service.method('rock.guvcview.camera', in_signature='siiiis', out_signature='o')
    def StartMonitor(self, camera_id, x, y, width, height, options):
        camera_id = camera_id.encode('utf-8')

        vid = '0'
        options = "%s;window-x=%s;window-y=%s;window-width=%s;window-height=%s" % (options, x, y, width, height)
        camera, path = self._load_media(camera_id, vid, True, options)
        return path

    @dbus.service.method('rock.guvcview.camera', in_signature='sss', out_signature='o')
    def EmbedMonitor(self, camera_id, visual_id, options):
        camera_id = camera_id.encode('utf-8')
        camera, path = self._load_media(camera_id, visual_id, True, options)
        return path

    @dbus.service.method('rock.guvcview.camera', in_signature='s')
    def StopCapture(self, camera_id):
        '''
        StopCapture
        '''
        camera_id = camera_id.encode('utf-8')
        self.joining_player()

        camera = self.cameras.get(camera_id, False)

        if not camera:
            return

        camera.decrease_reference()
        log("Stop %s: decrease count of reference [Current:  %d]" % (camera_id, camera.reference))

        # no one references this player, delete.
        if camera.reference < 1:
            #camera.terminate()
            self.cameras[camera_id].close_camera()
            time.sleep(2)
            if camera.is_alive():
                # it may be locked, kill it.
                time.sleep(1)
                self.kill_camera_process(camera.pid)

            self.remove_camera(camera)
            log("Stop %s: No one references this, close." % camera_id)
        return

    @dbus.service.method('rock.guvcview.camera', in_signature='s', out_signature='b')
    def IsCapturing(self, camera_id):
        '''
        IsCapturing
        '''
        camera_id = camera_id.encode('utf-8')
        self.joining_player()
        camera = self.cameras.get(camera_id, False)
        if not camera:
            return False
        return camera.is_playing()

    @dbus.service.method('rock.guvcview.camera', in_signature='ss', out_signature='ay')
    def GetFrame(self, camera_id, image_format):
        '''
        GetFrame
        image_format:
         - png
         - bmp
         - tiff
         - jpeg, jpg: 若加上數字，則數字部份代表JPEG圖像品質
        字母不區分大小寫，數字部份可以用任何非英文字母之字元分隔，也可以不分隔。
        以下表達格式皆可接受:
         'png', 'PNG', 'jpeg 80', 'jpg-90', 'jpg85'.
        '''
        camera_id = camera_id.encode('utf-8')
        # pattern = re.compile('([a-zA-Z]+)\W?(\d+)?')
        # pattern.search('jpeg80'), pattern.search('jpg 70'), pattern.search('jpg-90')
        #  (jpeg, 80)                (jpg, 70)                 (jpg, 90)
        # pattern.search('png')
        #  (png, None)
        pattern = re.compile('(?P<format>[a-zA-Z]+)\W?(?P<quality>\d+)?')

        m = pattern.search(image_format)
        if m:
            format, quality = m.groups()

        if not format:
            format = 'png'
        if format == 'jpg':
            format = 'jpeg'

        # quality is only used by jpeg.
        if not quality:
            quality = 80
        else:
            quality = int(quality)

        img_data = None
        if self.cameras[camera_id].is_playing():
            img_data = self.cameras[camera_id].capture_frame(image_format)

        if img_data == None or len(img_data) == 0:
            raise IOError("Can't take snapshot of %s" % camera_id)

        return img_data

    @dbus.service.method('rock.guvcview.camera', in_signature='ss', out_signature='s')
    def GetFrameAsBase64Content(self, camera_id, image_format):
        '''
        GetFrame As Base64 content.
        它與 GetFrame() 的差別在於，它的回傳值型態是 string 而非 ByteArray 。
        它將影像的位元組資料編碼為 Base64 內容後回傳，故呼叫者需再以 Base64 函數解碼。
        Base64內容約比原始資料內容多 33% 資料量，若呼叫者可以原生型態直接處理
        ByteArray 內容 ，則應呼叫 GetFrame()。
        某些程式語言對 DBus 資料型別的支援較有限，沒有辦法在內部轉換 string 與 byte array，
        只能以迭代方式將 byte array 加入 string 或將 string 拆成 byte array，
        影嚮執行效率，故增加此方法因應之。
        呼叫者應評估迭代轉換 byte array 為 string 以及 base64 解碼函數兩者間的效能
        差異，決定是否呼叫 GetFrameAsBase64Content().

        notice: DBus string 不可包含 null (\0) 字元，故不能將位元組資料直接當成 string 回傳。
        '''
        data = base64.b64encode(self.GetFrame(camera_id, image_format))
        return data

    @dbus.service.method('rock.guvcview.camera', in_signature='ss', out_signature='b')
    def SaveFrame(self, camera_id, filepath):
        '''
        抓圖，儲存於指定路徑。

        !SaveFrame 一律儲存為 PNG 格式。
        注意，儲存路徑應為絕對路徑，否則將會以服務行程的工作目錄為基準點。
        若以服務行程的工作目錄為基準點時，由於客戶端通常不知道服務行程的工作目錄在哪，因此將找不到儲存的圖像內容。

        @return True 或 False
        @error:
        * IOError:: Can't take snapshot of camera_id
        '''
        camera_id = camera_id.encode('utf-8')
        try:
            file_ext = filepath[filepath.rindex('.')+1]
        except:
            raise ValueError("Unknown image format")

        if file_ext == 'b' or file_ext == 'B':
            img_fmt = "bmp"
        elif file_ext == 'p' or file_ext == 'P':
            img_fmt = "png"
        else:
            img_fmt = "jpg"

        if self.cameras[camera_id].is_playing():
            result = self.cameras[camera_id].save_frame(img_fmt, filepath)
        return result

    @dbus.service.method('rock.guvcview.camera', in_signature='s', out_signature='uu')
    def GetVideoSize(self, camera_id):
        '''
        Get video size.
        This size may be not same as monitor window size.
        '''
        camera_id = camera_id.encode('utf-8')
        #if not self.cameras.has_key(camera_id):
        #KeyError
        #print("GetVideoSize")
        video_size = self.cameras[camera_id].get_video_size()
        return video_size

    @dbus.service.method('rock.guvcview.camera', in_signature='sii')
    def ShowWindow(self, camera_id, x, y):
        camera_id = camera_id.encode('utf-8')
        self.cameras[camera_id].show_video_window(x, y)

    @dbus.service.method('rock.guvcview.camera', in_signature='s')
    def HideWindow(self, camera_id):
        camera_id = camera_id.encode('utf-8')
        self.cameras[camera_id].hide_video_window()

    @dbus.service.method('rock.guvcview.camera', in_signature='sii')
    def ResizeWindow(self, camera_id, width, height):
        camera_id = camera_id.encode('utf-8')
        self.cameras[camera_id].resize_video_window(width, height)


    def terminate_all_cameras(self):
        debug("terminate all cameras and broadcast closed signal.")
        for camera_id in self.cameras.keys():
            self.cameras[camera_id].terminate();
            self.Event(camera_id, "camera_closed")
        time.sleep(2.0)
        for camera_id in self.cameras.keys():
            os.kill(self.cameras[camera_id].pid, 9)

    @dbus.service.signal('rock.guvcview.camera', signature='ss')
    def Event(self, camera_id, event):
        """
        注意，事件發生在子行程空間，故無法觸及主行程空間中的資訊。
        """
        debug("Event: %s raise event [%s]" % (camera_id, event))
        if event == "end_reached" or event == "encountered_error":
            log("Event 'end_reached' catched, %s may be down. Terminate this camera." % camera_id)
            camera_exit() #Camera.exit()


LibC = None
LibUvc = None

def init_libguvcview():
    global LibC, LibUvc
    #print("ctypes load libc")
    LibC = ctypes.CDLL(ctypes.util.find_library("libc"))

    LibC.free.argtypes = [ctypes.c_void_p]
    LibC.free.restype = None

    #print("ctypes load libguvcview")
    try:
        LibUvc = ctypes.CDLL("../guvcview/src/libguvcview")
    except OSError:
        LibUvc = ctypes.CDLL(MY_LIB_PATH + "/libguvcview")

    LibUvc.lib_open_camera.argtypes = [
        ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    LibUvc.lib_open_camera.restype = ctypes.c_int

    LibUvc.lib_get_video_size.argtypes = [
        ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
    LibUvc.lib_get_video_size.restype = ctypes.c_bool

    LibUvc.lib_save_frame.argtypes = [
        ctypes.c_char_p, ctypes.c_char_p]
    LibUvc.lib_save_frame.restype = ctypes.c_bool

    LibUvc.lib_capture_frame.argtypes = [
        ctypes.c_char_p,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_int)]
    LibUvc.lib_capture_frame.restype = ctypes.c_bool

    LibUvc.lib_get_controls_values.restype = ctypes.c_char_p

    LibUvc.lib_get_control_value.argtypes = [ctypes.c_char_p]
    LibUvc.lib_get_control_value.restype = ctypes.c_char_p

    LibUvc.lib_set_control_value.argtypes = [
        ctypes.c_char_p,
        ctypes.c_char_p]
    LibUvc.lib_set_control_value.restype = ctypes.c_bool

    LibUvc.lib_continue_focus.argtypes = [ctypes.c_bool]
    LibUvc.lib_continue_focus.restype = None

    LibUvc.lib_auto_focus.argtypes = [ctypes.c_bool]
    LibUvc.lib_auto_focus.restype = None

    LibUvc.lib_video_window_show.argtypes = None
    LibUvc.lib_video_window_show.restype = None

    LibUvc.lib_video_window_hide.argtypes = None
    LibUvc.lib_video_window_hide.restype = None

    LibUvc.lib_video_window_move.argtypes = [ctypes.c_int, ctypes.c_int]
    LibUvc.lib_video_window_move.restype = None

    LibUvc.lib_video_window_resize.argtypes = [ctypes.c_int, ctypes.c_int]
    LibUvc.lib_video_window_resize.restype = None



# 在多行程模式下，每個 Camera 實體皆獨立存在於各自的行程空間中。
# 在這個獨立空間中，類別成員與實體成員並無差異。
# 為了方便事件處理函數取得camera_id，我在子行程空間中，將 camera_id 存一份到類
# 別成員 Camera.ID。
# it will be set by camera_wrapper (in sub process space).
class Camera(multiprocessing.Process, dbus.service.Object):
    ID = None
    PLAYER = None

    def __init__(self, camera_id, visual=False, autoplay=True, options=None, bus = None, path = None):
        global Service

        self._reference = 1
        self.id = camera_id
        self.visual = visual

        # duplex pipe for main and sub interactive.
        self.method_pipe, child_method_pipe = SignalSafePipe(True)

        multiprocessing.Process.__init__(self,
            target=camera_wrapper, args=(self, child_method_pipe, camera_id, visual, autoplay, options))
        self.start()
        debug("New camera: %s" % camera_id)

        debug("wait sub process init...")
        Service.add_pending(self.pid)
        if self.method_pipe.poll(CameraService.Alive_Detect_Interval):
            rc = self.method_pipe.recv()
            Service.del_pending(self.pid)
        else:
            self.method_pipe.close()
            child_method_pipe.close()
            log("Timeout for start %s, raise IOError." % camera_id)
            Service.kill_camera_process(self.pid)
            raise IOError("Can not play %s. Timeout." % camera_id)

        if isinstance(rc, Exception):
            self.method_pipe.close()
            child_method_pipe.close()
            log("Failed to start %s, raise the exception: %s" % (camera_id, rc))
            raise rc
        dbus.service.Object.__init__(self)
        child_method_pipe.close()

    @property
    def reference(self):
        return self._reference

    def increase_reference(self):
        self._reference += 1

    def decrease_reference(self):
        if self._reference > 0:
            self._reference -= 1

    def request_no_result(self, method_name, args = None):
        debug("request_no_result %s" % method_name)
        if CameraService.Request_Count > 200:
            log("Service busy!")
            raise IOError("Service busy!")

        CameraService.start_request_alarm()
        CameraService.Request_Count += 1

        request_args = {'method': method_name}
        if args != None:
            request_args['args'] = args
        debug("send request")
        self.method_pipe.send(request_args)

        CameraService.Request_Count -= 1
        CameraService.stop_request_alarm()
        #no result

    def request_with_result(self, method_name, args = None, recv_bytes = False):
        debug("request_with_results %s" % method_name)
        if CameraService.Request_Count > 200:
            log("Service busy!")
            raise IOError("Service busy!")

        CameraService.start_request_alarm()
        CameraService.Request_Count += 1

        request_args = {'method': method_name}
        if args:
            request_args['args'] = args
        debug("send request")
        self.method_pipe.send(request_args)

        try:
            if self.method_pipe.poll(CameraService.Max_Wait_Seconds):
                result = self.method_pipe.recv()
            else:
                debug("no response. poll failed")
                #result = None
                time.sleep(0.2)
                result = self.method_pipe.recv()
        except Exception as e:
            debug("request with result error: %s" % e)
            result = None

        CameraService.Request_Count -= 1
        CameraService.stop_request_alarm()
        return result

    def close_camera(self):
        self.request_no_result("close_camera")

    def is_playing(self):
        return True

    def get_mrl(self):
        return self.request_with_result("get_mrl")

    def get_video_size(self):
        w = h = 0
        w, h = self.request_with_result("get_video_size")
        return w, h

    def get_video_width(self):
        w, _ = self.get_video_size()
        return w

    def get_video_height(self):
        _, h = self.get_video_size()
        return h

    def capture_frame(self, img_fmt):
        imgdata = self.request_with_result("capture_frame", args=img_fmt, recv_bytes=True)
        if not imgdata:  # None
            imgdata = ""
        return imgdata

    def save_frame(self, img_fmt, filepath):
        result = self.request_with_result("save_frame", args=(img_fmt, filepath))
        return result

    def show_video_window(self, x, y):
        self.request_no_result("video_window_show", args=(x, y))

    def hide_video_window(self):
        self.request_no_result("video_window_hide")

    def resize_video_window(self, width, height):
        self.request_no_result("video_window_resize", args=(width, height))


    @dbus.service.method('rock.guvcview.camera', in_signature='')
    def RequireFocus(self):
        '''
        RequireFocus.
        '''
        print("RequireFocus")
        self.request_no_result("require_focus")

    @dbus.service.method('rock.guvcview.camera', in_signature='b')
    def ContinueFocus(self, value):
        '''
        ContinueFocus set On or Off.
        '''
        print("ContinueFocus")
        self.request_no_result("continue_focus", args=value)

    @dbus.service.method('rock.guvcview.camera', in_signature='b')
    def AutoFocus(self, value):
        '''
        Set hardware auto focus module (if this camera built-in) to On or Off.
        Only few cameras has this feature.
        '''
        print("AutoFocus")
        self.request_no_result("auto_focus", args=value)

    @dbus.service.method('rock.guvcview.camera', in_signature='', out_signature='a{sv}')
    def GetAllControlValues(self):
        json_str = self.request_with_result("get_controls_values")
        print(json_str)
        try:
            values = json.loads(json_str)
        except:
            values = {}
        return values

    @dbus.service.method('rock.guvcview.camera', in_signature='s', out_signature='v')
    def GetControlValue(self, propname):
        '''
        取得指定的屬性值。
        '''
        json_str = self.request_with_result("get_control_value", args=propname)
        try:
            value = json.loads(json_str)
        except:
            value = None
        return value

    @dbus.service.method('rock.guvcview.camera', in_signature='sv')
    def SetControlValue(self, propname, value):
        '''
        設定指定的屬性值。
        '''
        json_str = json.dumps(value)
        return self.request_with_result("set_control_value", args=(propname, json_str))


    def do_nothing(self, value = None):
        return

    src = property(get_mrl, do_nothing, do_nothing,
        "Source of media resource locator. Readonly.")
    currentSrc = property(get_mrl, do_nothing, do_nothing,
        "Source of media resource locator. Readonly.")
    videoWidth = property(get_video_width, do_nothing, do_nothing,
        "VideoWidth in px. Readonly.")
    videoHeight = property(get_video_height, do_nothing, do_nothing,
        "VideoHeight in px. Readonly.")

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='s', out_signature='a{sv}')
    def GetAll(self, interface):
        props = {}
        for k in ( 'src', 'currentSrc',
                    'videoWidth', 'videoHeight'
                  ):
            props[k] = getattr(self, k, False)
        return props

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='ss', out_signature='v')
    def Get(self, interface, propname):
        '''
        取得指定的屬性值。
        名稱首字元大寫(Dbus名稱慣例)或小寫(HTML5名稱慣例)都接受。
        例如 currentTime 或 CurrentTime 都可以取得同一個屬性的值。
        '''
        propname = propname[0].lower() + propname[1:]
        return getattr(self, propname, False)

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='ssv')
    def Set(self, interface, propname, value):
        '''
        設定指定的屬性值。
        名稱首字元大寫(Dbus名稱慣例)或小寫(HTML5名稱慣例)都接受。
        例如 currentTime 或 CurrentTime 都可以設定同一個屬性的值。
        '''
        propname = propname[0].lower() + propname[1:]
        try:
            setattr(self, propname, value)
        except:
            pass

def camera_wrapper(obj, pipe, camera_id, window_id, autoplay, options):
    """
    所有在此函數內配置的資源，都位於子行程的行程空間。
    """
    try:
        import gtk
    except:
        pass
    global method_pipe
    Camera.ID = camera_id
    method_pipe = pipe

    mrl = camera_id

    debug("New player: %s" % mrl)

    signal.signal(signal.SIGALRM, camera_terminate_signal_handler)

    camera_start_alarm()  # 預防 pipe 卡死，若時間內未完成，則行程自我毀滅。
    try:
        Camera.PLAYER = UvcPlayer(mrl, window_id, options)
    except Exception as err:
        debug("sub process failed to init")
        method_pipe.send(err)
        os._exit(0) #camera_exit()
    camera_stop_alarm()

    Service.Event(Camera.ID, "play_successed")
    # 重置繼承自父行程的狀態與資源
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)
    signal.signal(signal.SIGHUP, camera_terminate_signal_handler)
    signal.signal(signal.SIGTERM, camera_terminate_signal_handler)
    signal.signal(signal.SIGINT, camera_terminate_signal_handler)
    signal.signal(signal.SIGQUIT, camera_terminate_signal_handler)
    signal.signal(signal.SIGSEGV, camera_terminate_signal_handler)

    method_pipe.send("+INIT")

    while True:
        try:
            rc = method_pipe.recv()
            debug("-- child recv %s (at %s) --" % (rc, camera_id))

            camera_start_alarm()  # 預防 pipe 卡死，若時間內未完成，則行程自我毀滅。
            result = None
            method = getattr(Camera.PLAYER, rc['method'], None)
            if method == None:
                method = getattr(LibUvc, 'lib_%s' % rc['method'], None)

            if method:
                if rc.has_key('args'):
                    if type(rc['args']) == type(tuple()):
                        result = method(*rc['args'])
                    else:
                        result = method(rc['args'])
                else:
                    result = method()
            else:
                debug('%s method not found' % rc['method'])
                continue
#    LibUvc.lib_video_window_hide.argtypes = None
#    LibUvc.lib_video_window_hide.restype = None

            if result != None:
                method_pipe.send(result)
            #else:
            #    debug("result is None")
            camera_stop_alarm()
            debug("-- child responsed --")
        except Exception as e:
            debug("Pipe read/write exception: %s" % e)
            break

    debug("child stop!")
    camera_exit()

Managing_Process = None

def camera_exit():
    '終止 camera 子行程'
    if Managing_Process:
        debug("destory WindowPlay window")
        os.kill(Managing_Process.pid, signal.SIGTERM)
    camera_remove_file(get_managed_visual_id_filepath(os.getpid()))
    debug("camera_exit")
    LibUvc.lib_close_camera()
    os._exit(0)

def camera_remove_file(filepath):
    try:
        os.remove(filepath)
    except:
        pass

def camera_terminate_signal_handler(signum, func):
    'camera 結束訊號的處理程序'
    debug("terminate player")
    camera_exit() #Camera.exit()

def camera_start_alarm():
    '啟動操作計時器。若時限內未完成，則觸發 SIGALRM 訊號'
    debug("start alarm")
    signal.alarm(CameraService.Max_Wait_Seconds)

def camera_stop_alarm():
    '關閉操作計時器。'
    debug("end alarm")
    signal.alarm(0)

class UvcPlayer:

    options = []

    def __init__(self, mrl, window_id="No", options=""):
        if mrl.startswith('v4l2://'):
            self.mrl = mrl[7:]
        else:
            self.mrl = mrl
        if window_id:
            #if type(window_id) == type(tuple()):
            #    if len(window_id) < 4:
            #        raise IOError("Too few arguments to pass to WindowPlay")
            #    self.window_id = self.new_window(*window_id)
            #else:
            self.window_id = str(window_id)
        else:
            self.window_id = "No"
        if options:
            self.options = options
        else:
            self.options = ""
        log("UVC view %s with options %s" % (mrl, options))
        log("Render video in Window ID %s" % self.window_id)

        Service.Event(Camera.ID, "opening")
        #debug("type of self.window_id %s" % type(self.window_id))
        if LibUvc.lib_open_camera(self.mrl, self.window_id, self.options) != 0:
            log("UVC View can not open %s" % self.mrl)
            raise IOError("Can not view %s" % self.mrl)

        time.sleep(0.5)
        #Service.Event(self.mrl, "play_successed")
        log("UVC View %s success" % self.mrl)
        Service.Event(Camera.ID, "playing")

        def actively_player_detect(player):
            while(True):
                #debug("active detect player status, per %d seconds" % CameraService.Alive_Detect_Interval)
                if not player.is_playing():
                    break
                time.sleep(CameraService.Alive_Detect_Interval)

        actively_player_detector = threading.Timer(
            CameraService.Alive_Detect_Interval, actively_player_detect, args=(self,))
        actively_player_detector.start()

    def get_mrl(self):
        return self.mrl

    def is_playing(self):
        return True

    def close_camera(self):
        debug("close_camera %s" % self.mrl)
        camera_exit()
        Service.Event(Camera.ID, "camera_closed")

    def get_video_size(self):
        w = ctypes.c_int(0)
        h = ctypes.c_int(0)
        LibUvc.lib_get_video_size(ctypes.byref(w), ctypes.byref(h))
        #print("%d %d" % (w.value, h.value))
        return w.value, h.value

    def capture_frame(self, img_fmt):
        data = ctypes.c_void_p(None)
        data_len = ctypes.c_int(0)
        LibUvc.lib_capture_frame(str(img_fmt),
            ctypes.byref(data), ctypes.byref(data_len))
        #print("capture data: %x, data_len: %d" % (data.value, data_len.value))
        result = ctypes.string_at(data, data_len)
        #print("len of result : %d" % len(result))
        LibC.free(data)
        return result

    def video_window_show(self, x, y):
        LibUvc.lib_video_window_show()
        LibUvc.lib_video_window_move(x, y)

    def get_controls_values(self):
        print("get_controls_values")
        values_str = LibUvc.lib_get_controls_values()
        ret_str = ctypes.string_at(values_str)
        #LibC.free(values_str)
        return ret_str

    def get_control_value(self, name):
        value_str = LibUvc.lib_get_control_value(name)
        ret_str = ctypes.string_at(value_str)
        #LibC.free(value_str)
        return ret_str

    def set_control_value(self, name, value):
        print("set [%s] = [%s]" % (name, value))
        return LibUvc.lib_set_control_value(name, value)


#def estimate_visual_size(x, y, width, height, video_width, video_height):
    #'''
    #w, y: 目標視窗預定放置座標
    #width, height: 目標視窗最大長寬
    #video_width, video_height: 來源影像長寬
    #'''
    #wg = float(width) / float(video_width)
    #hg = float(height) / float(video_height)

    #if (wg >= hg):
        #visual_w = int(round(hg * video_width))
        #visual_h = height
    #else:
        #visual_w = width
        #visual_h = int(round(wg * video_height))
    ## 目標視窗實際使用長寬 (固定比例縮放)
    #debug("visual_w: %d, visual_h: %d" % (visual_w, visual_h))

    #center_vertical_offset = 0
    #center_horizon_offset = 0

    #if (visual_w != width):
        #center_horizon_offset = (width - visual_w) / 2
    #if (visual_h != height):
        #center_vertical_offset = (height - visual_h) / 2
    #visual_x = int(round(x + center_horizon_offset))
    #visual_y = int(round(y + center_vertical_offset))
    ## 目標視窗實際放置座標 (置中於預期區域)
    #debug("visual_x: %d; visual_y: %d" % (visual_x, visual_y))
    #return visual_x, visual_y, visual_w, visual_h

def terminate_self(signum, func):
    global Service
    if MAIN_PID != os.getpid():
        debug("terminate sub process")
        os._exit(0)

    Service.terminate_all_cameras()
    remove_pid_file()

    log("Terminate service (signal: %d)" % signum)
    #sys.exit(0) # this will be hang.
    os._exit(0)

#View 建立 window ，並將 xid 寫入 get_managed_visual_id_filepath() 所指定的檔案。
#CameraService 再從該檔案中取得 xid ，以便回傳給使用者。
def get_managed_visual_id_filepath(pid):
    filepath = "/tmp/uvc-camera-service-%d.vid" % pid
    return filepath

def get_homepath():
    try:
        homepath = os.path.expanduser("~")
    except:
        homepath = "/tmp"
    else:
        if not homepath or homepath == "":
            homepath = "/tmp"
    return homepath


def get_pid_filepath():
    pid_filepath = "%s/uvc-camera-service.pid" % get_homepath()
    return pid_filepath

def remove_pid_file():
    try:
        os.remove(get_pid_filepath())
    except:
        pass

def log_pid_file():
    pid = os.getpid()
    try:
        pidfile = open( get_pid_filepath(), "w")
    except:
        pass
    else:
        pidfile.write("%d" % pid)
        pidfile.close()

Log_Mode = True
Log_Max_Size = 10485760  # 1024 * 1024 * 10 (10MB)

def log(s):
    log_msg = "[%s] %s" % (time.asctime(), s)
    print(log_msg)

    if not Log_Mode:
        return

    log_filepath = "%s/uvc-camera-service.log" % get_homepath()

    if os.access(log_filepath, os.W_OK):
        if Debug_Mode:      # Debug_Mode will not limit size of log-file.
            open_mode = "a" # append to existence
        else:
            log_st = os.stat(log_filepath) # check size of log-file.
            if log_st.st_size > Log_Max_Size:
                open_mode = "w"
            else:
                open_mode = "a"
    else:
        open_mode = "w" # create

    try:
        log = open(log_filepath, open_mode)
    except:
        pass
    else:
        log.write(log_msg)
        log.write("\n")
        log.close()


def debug(s):
    if Debug_Mode == False:
        return
    log("[DEBUG][PID:%d] %s" %
        (os.getpid(), s))


def usage(status=0):
    print("""
Usage:
    -h, --help

    -d, --debug
        Enable debug mode. It will print debug message to stdout.

    -q, --quiet
        Enable quiet mode. It only prints log message, does not save in logfile.

    -s, --start=
        A file contents the list of cameras.
        These cameras will be started when service startup.
        The default list file path is /etc/camera/uvc/start-list
        format: mrl x y width height options
        example:
        =========
        # Auto start cameras
        /dev/video0 10 20 400 300
        /dev/video1 500 20 400 300 v4l2-width=800;v4l2-height=600
        =========

    """)
    os._exit(status)

def g_event_timer(*args):
    print("tick")
    return True

if __name__ == "__main__":
    start_list_filepath = "/etc/camera/uvc/start-list"
    debug_filepath = "/etc/camera/uvc/debug"

    os.chdir("/")
    try:
        opts, args = getopt.getopt(sys.argv[1:], "dhqs:",
            ["debug", "help", "quiet", "start="])
    except getopt.GetoptError, err:
        # print help information and exit:
        print(str(err)) # will print something like "option -a not recognized"
        usage(status=2)

    Debug_Mode = False
    if os.access(debug_filepath, os.R_OK):
        debug_file_content = json.loads(open(debug_filepath, 'r').readline())
        if debug_file_content:
            Debug_Mode = True

    for o, a in opts:
        if o in ("-d", "--debug"):
            Debug_Mode = True
        elif o in ("-q", "--quiet"):
            Log_Mode = False
        elif o in ("-h", "--help"):
            usage()
        elif o in ("-s", "--start"):
            start_list_filepath = a

    init_libguvcview()

    # You must do this before connecting to the bus.
    DBusGMainLoop(set_as_default=True) # if you use DBus.
    loop = gobject.MainLoop()

    # enable g_event_timer tick.
    #gobject.timeout_add(10000, g_event_timer)

    CameraService.BusType = 'session'
    bus = dbus.SessionBus()

    Service = CameraService(bus, loop)

    if Debug_Mode:
        _debug_text = "ON"
    else:
        _debug_text = "OFF"

    log("""dbus-camera; core: guvcview;
        Service startup. Debug mode [%s]"""
        % (_debug_text))

    log_pid_file()

    def auto_start_monitor(line):
        try:
            parts = s.split()
            if len(parts) == 6:
                mrl, x, y, w, h, opts = parts
            else:
                mrl, x, y, w, h = parts
                opts = ""
            x = int(x)
            y = int(y)
            w = int(w)
            h = int(h)
            Service.StartMonitorWindow(mrl, x, y, w, h, opts)
            log("Auto open camera: %s %d %d %d %d %s" % (mrl, x, y, w, h, opts))
        except Exception as e:
            #debug(e)
            pass

    if os.access(start_list_filepath, os.R_OK):
        with open(start_list_filepath, "r") as fh:
            while True:
                s = fh.readline()
                if not s:
                    break
                s = s.strip()
                if s == '' or s[0] == '#':  # comment
                    continue
                auto_start_monitor(s)

    try:
        loop.run()
    except:
        pass
    terminate_self(signal.SIGTERM, None)
    os._exit(0)
