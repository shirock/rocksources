#!/usr/bin/python
# coding: utf-8
#
# D-Bus Service of omxplayer
# Copyright (C) 2013 Shih Yuncheng <shirock.tw@gmail.com>
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
'''
@author: rock <shirock.tw@gmail.com>
@version: 0.1.5 ($Rev)

DBus type: system
Service name: raspberry.pi.OMXPlayer

Object path of List: /raspberry/pi/OMXPlayer
Interface: raspberry.pi.OMXPlayerList
    * Play(mrl, x, y, width, height, keepratio, options)
    * Queue(mrl, x, y, width, height, keepratio, options)
    * List()
    * signal Event(player_path, event)

Object path of Player: /raspberry/pip/OMXPlayer/{id}
Interface: raspberry.pi.OMXPlayer
    * Stop()
    * Pause()
    * Resume()
    * Volume(tick)
    * SeekForward(distance)/SeekAhead
    * SeekBack(distance)
    * PreviousAudioStream()
    * NextAudioStream()
    * ToggleSubtitle()
    * PreviousSubtitle()
    * NextSubtitle()
    * PreviousChapter()
    * NextChapter()
Interface: org.freedesktop.DBus.Properties
    * GetAll('')
    * Get('', property_name)

'''

OMXPLAYER='/usr/bin/omxplayer.bin'
OMXPLAYER_LIB_PATH='/opt/vc/lib:/usr/lib/omxplayer'
LOCAL_LIB_PATH='/usr/local/lib'

import os, sys
reload(sys)
sys.setdefaultencoding('utf-8')
sys.path.append(LOCAL_LIB_PATH)
os.environ['LD_LIBRARY_PATH'] = OMXPLAYER_LIB_PATH
import time, getopt, re
import signal, multiprocessing, subprocess
import gobject, dbus, dbus.service
from dbus.mainloop.glib import DBusGMainLoop  # setting up an event loop

DBusServiceName = 'raspberry.pi.OMXPlayer'

IDBusProperties = 'org.freedesktop.DBus.Properties'
IList = 'raspberry.pi.OMXPlayerList'
IPlayer = 'raspberry.pi.OMXPlayer'

Log_Mode = True
Log_Max_Size = 10485760  # 1024 * 1024 * 10 (10MB)
Debug_Mode = False
Bus_Type = 'system'
Service = None

Xrefresh_Display = None

class ListService(dbus.service.Object):
    Path = "/raspberry/pi/OMXPlayer"

    Alive_Detect_Interval = 10

    def __init__(self, bus, event_loop):
        self.bus = bus
        self.event_loop = event_loop
        
        # It contains OMXPlayer objects.
        # players = { pid: player }
        self.players = {}

        # SIGVTALRM and SIGPROF do not work.
        signal.setitimer(signal.ITIMER_REAL, ListService.Alive_Detect_Interval, ListService.Alive_Detect_Interval)
        signal.signal(signal.SIGALRM, ListService.actively_joining_players)

        signal.signal(signal.SIGCHLD, ListService.actively_joining_players)

        bus_name = dbus.service.BusName(DBusServiceName, bus=self.bus)
        dbus.service.Object.__init__(self, bus_name, ListService.Path)

        signal.signal(signal.SIGHUP,  terminate_self)
        signal.signal(signal.SIGTERM, terminate_self)
        signal.signal(signal.SIGINT,  terminate_self)
        signal.signal(signal.SIGQUIT, terminate_self)
        signal.signal(signal.SIGSEGV, terminate_self)


    @classmethod
    def actively_joining_players(self, signum, func):
        #debug("active join players, per %d seconds" % ListService.Alive_Detect_Interval)
        Service.joining_player()


    def joining_player(self):
        #See http://docs.python.org/library/multiprocessing.html#all-platforms
        #    Joining zombie processes
        #debug('joining_player')
        cl = multiprocessing.active_children()

        for pid in self.players.keys():
            try:
                rc = os.waitpid(pid, os.WNOHANG)
                #print("rc: %d,%d" % rc)
            except OSError as e:
                debug('pid %d: %s' % (pid, e))
                del self.players[pid]
            else:
                if rc[0] > 0: # proc terminated.
                    #debug('%d is terminated.' % pid)
                    self.players[pid].close_player()
                    del self.players[pid]


    @dbus.service.method(IList, out_signature='a(os)')
    def List(self):
        '''
        列出目前開啟的 media 。
        '''
        self.joining_player()
        ret = []
        for player in self.players.values():
            ret.append((player.path, player.mrl))
        return ret


    def load_media(self, mrl, x, y, width, height, autoplay, keepratio, options):
        mrl = mrl.encode('utf-8')

        player = OMXPlayer(mrl, x, y, width, height, keepratio,
                    autoplay=autoplay, options=options, bus=self.bus)
        self.players[player.proc.pid] = player
        return player.path
    

    @dbus.service.method(IList, in_signature='siiiibs', out_signature='o')
    def Play(self, mrl, x, y, width, height, keepratio, options):
        '''
        Play. 

        @param keepratio Will auto estimate visual region and fill video in.
        @param options pass to omxplayer.
        @return object_path
        '''
        return self.load_media(mrl, x, y, width, height, True, keepratio, options)


    @dbus.service.method(IList, in_signature='siiiibs', out_signature='o')
    def Queue(self, mrl, x, y, width, height, keepratio, options):
        '''
        Queue. (open media then pause it)

        @param keepratio Will auto estimate visual region and fill video in.
        @param options pass to omxplayer.
        @return object_path
        '''
        return self.load_media(mrl, x, y, width, height, False, keepratio, options)


    @dbus.service.signal(IList, signature='os')
    def Event(self, player_path, event):
        debug("Event: %s raise event [%s]" % (player_path, event))


    def terminate_all_players(self):
        debug("terminate all players and broadcast closed signal.")
        for pid in self.players.keys():
            path = self.players[pid].path
            try:
                self.players[pid].proc.terminate();
            except:
                pass
            self.Event(path, "closed")
        time.sleep(1.0)
        for pid in self.players.keys():
            kill_process(pid)


class OMXPlayer(dbus.service.Object):

    def __init__(self, mrl, x, y, width, height, keepratio, autoplay=True, options=None, bus = None, path = None):
        global Service
        dbus.service.Object.__init__(self)

        self.bus = bus
        self.mrl = mrl

        if mrl.startswith('.'):
            raise IOError('Unsafe path. Please use full path.')

        if mrl.startswith('/') and not os.access(mrl, os.R_OK):
            raise IOError('No permission to read %s' % mrl)

        video_info = detect_video_information(mrl)
        if video_info == False:
            raise IOError('Media "%s" not found' % mrl)
        
        self.video_size = (video_info[0], video_info[1])
        self.audio_stream_list = video_info[2]
        self.duration_milliseconds = video_info[3]

        if keepratio and self.video_size[0] > 0:
            video_width, video_height = self.video_size
            visual_area = estimate_visual_size(x, y, width, height, 
                                    video_width, video_height)
        else:
            visual_area = (x, y, width, height)

        self.x1, self.y1, self.x2, self.y2 = visual_area
        self.x2 += self.x1
        self.y2 += self.y1

        self._paused = False
        self._subtitle_toggle = False
        self._volumn = 0 # 0db
        self.audio_stream_index = 1

        self.options = options
        if self.options:
            m = re.search('(-n|--aidx) (\d+)', self.options)
            if m:
                self.audio_stream_index = int(m.group(2))
                if self.audio_stream_index > len(self.audio_stream_list):
                    self.audio_stream_index = len(self.audio_stream_list)
                elif self.audio_stream_index < 1:
                    self.audio_stream_index = 1
            
            # if you specify '--subtitles', subtitle state will be On.
            if '--subtitles ' in self.options:
                self._subtitle_toggle = True

        log("omxplayer play %s with options %s" % (self.mrl, self.options))
        #debug("window: %s %s %s %s" % (self.x1, self.y1, self.x2, self.y2))

        #Service.Event(mrl, "opening")
        cmd = [OMXPLAYER]
        if self.options:
            cmd.extend(self.options.split(' '))
        cmd.extend(['--win', '%d %d %d %d'%(self.x1,self.y1,self.x2,self.y2), self.mrl])
        debug(cmd)
        try:
            self.proc = subprocess.Popen(cmd, 
                stdin=subprocess.PIPE, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.STDOUT,
                env={"LD_LIBRARY_PATH": OMXPLAYER_LIB_PATH})
        #except OSError as e:
        except Exception as e:
            log("OSError: %s" % e)
            raise e

        self.path = "%s/%d" % (ListService.Path, self.proc.pid)
        self.add_to_connection(self.bus, self.path)
        log("omxplayer play %s success. object path is %s." % (self.mrl, self.path))
        
        if autoplay:
            Service.Event(self.path, "playing")
        else:
            time.sleep(1.0) # if you immediate Pause. It will not resume.
            self.Pause()
            Service.Event(self.path, "queueing")


    def close_player(self):
        self.remove_from_connection(self.bus, self.path)
        Service.Event(self.path, "closed")


    def control(self, ctrl):
        rc = True
        #debug('ctrl %s' % ctrl)
        try:
            self.proc.stdin.write(ctrl)
        except:
            rc = False

        if rc == False:
            try:
                self.proc.terminate()
            except:
                pass

        time.sleep(0.05) # waiting omxplayer read this command.


    @dbus.service.method(IPlayer)
    def Stop(self):
        pid = self.proc.pid
        debug("close %s" % self.mrl)
        self.control('q')
        result = self.proc.stdout.read()
        #debug(result)
        self.proc.poll()
        
        if 'have a nice day ;)' in result:
            debug('omxplayer end')
        else:
            debug('not normal terminate')
            kill_process(pid)

        #self.close_player() # will be done in joining_player().

        #debug("check xrefresh")
        #if Xrefresh_Display:
        #    xrefresh()
        return


    @dbus.service.method(IPlayer, out_signature='uu')
    def GetVideoSize(self):
        return self.video_size


    @dbus.service.method(IPlayer)
    def Pause(self):
        if self._paused:
            return
        self.control('p')
        self._paused = True
        Service.Event(self.path, "pause")
        return


    @dbus.service.method(IPlayer)
    def Resume(self):
        if not self._paused:
            return
        self.control('p')
        self._paused = False
        Service.Event(self.path, "resume")
        return


    @dbus.service.method(IPlayer, in_signature='s')
    def Volumn(self, tick):
        '''
        tick: + or -.
        one tick to +/- 3db.
        for example: '++' to increase 6 db, '-----' to decrease 15 db.
        '''
        for t in tick:
            if t == '+':
                self._volumn += 3 # +3db
            elif t == '-':
                self._volumn -= 3 # -3db
            else:
                continue
            self.control(t)
        return


    @dbus.service.method(IPlayer, in_signature='s')
    def SeekForward(self, distance):
        '''
        distance:
         small: 30 seconds (default)
         large: 600 seconds
        '''
        d = distance[0]
        if d == 'l':
            ctrl = '\x5b\x41'
        else:
            ctrl = '\x5b\x43'
        self.control(ctrl)
        return


    @dbus.service.method(IPlayer, in_signature='s')
    def SeekAhead(self, distance):
        '''
        Alias of SeekForward().
        '''
        self.SeekForward(distance)


    @dbus.service.method(IPlayer, in_signature='s')
    def SeekBack(self, distance):
        '''
        distance:
         small: 30 seconds (default)
         large: 600 seconds
        '''
        d = distance[0]
        if d == 'l':
            ctrl = '\x5b\x42'
        else:
            ctrl = '\x5b\x44'
        self.control(ctrl)
        return


    @dbus.service.method(IPlayer)
    def PreviousAudioStream(self):
        '''
        注意，切換音軌的動作，不會自動循環。
        '''
        if self.audio_stream_index <= 1:
            return
        self.audio_stream_index -= 1
        self.control('j')


    @dbus.service.method(IPlayer)
    def NextAudioStream(self):
        if self.audio_stream_index >= len(self.audio_stream_list):
            return
        self.audio_stream_index += 1
        self.control('k')


    @dbus.service.method(IPlayer)
    def ToggleSubtitle(self):
        '''
        Toggle subtitle On or Off.
        If you specify option '--subtitles', subtitle initial state is On.
        However, if you play mkv embed ssa/ass, subtitle initial state is Off.

        Subtitle: mkv embed ssa/ass or mp4 with external srt.
        Notice: subtitle will be rendered on fixed position (bottom of display),
        not on the bottom of video. (#7)
        '''
        self.control('s')
        self._subtitle_toggle = not self._subtitle_toggle
        return

    @dbus.service.method(IPlayer)
    def PreviousSubtitle(self):
        self.control('n')
        return

    @dbus.service.method(IPlayer)
    def NextSubtitle(self):
        self.control('m')
        return


    @dbus.service.method(IPlayer)
    def PreviousChapter(self):
        'Because omxplayer has bug, this action may not work. (#6)'
        self.control('i')
        return


    @dbus.service.method(IPlayer)
    def NextChapter(self):
        'Because omxplayer has bug, this action may not work. (#6)'
        self.control('o')
        return


    @property
    def MRL(self):
        'Media Resource Locator. Readonly.'
        return self.mrl

    @property
    def paused(self):
        'Is paused.'
        return self._paused

    @property
    def videoWidth(self):
        'Video original width.'
        return self.video_size[0]

    @property
    def videoHeight(self):
        'Video original height.'
        return self.video_size[1]

    @property
    def visualTop(self):
        'Top position of visual area'
        return self.x1

    @property
    def visualLeft(self):
        'Left position of visual area'
        return self.y1

    @property
    def visualBottom(self):
        'Bottom position of visual area'
        return self.x2

    @property
    def visualRight(self):
        'Right position of visual area'
        return self.y2

    @property
    def duration(self):
        'Duration of video (milliseconds)'
        return self.duration_milliseconds

    @property
    def volumn(self):
        'Volumn, unit: db.'
        return self._volumn

    @property
    def audioStreamList(self):
        'Audio stream List: (track : track details).'
        return self.audio_stream_list

    @property
    def currentAudioStream(self):
        'Index number of current audio stream. Base on 1.'
        return self.audio_stream_index

    @property
    def subtitle(self):
        'subtitle on or off.'
        return self._subtitle_toggle

    @dbus.service.method(IDBusProperties, in_signature='s', out_signature='a{sv}')
    def GetAll(self, interface):
        props = {}
        keys = ('MRL', 'paused', 'volumn', 'videoWidth', 'videoHeight', 
            'visualTop', 'visualLeft', 'visualBottom', 'visualRight',
            'duration',
            'audioStreamList', 'currentAudioStream', 'subtitle')
        for k in keys:
            props[k] = getattr(self, k, False)
        return props

    @dbus.service.method(IDBusProperties, in_signature='ss', out_signature='v')
    def Get(self, interface, propname):
        '''
        取得指定的屬性值。
        名稱首字元大寫(Dbus名稱慣例)或小寫(HTML5名稱慣例)都接受。
        例如 currentTime 或 CurrentTime 都可以取得同一個屬性的值。
        '''
        propname = propname[0].lower() + propname[1:]
        return getattr(self, propname, False)

    @dbus.service.method(IDBusProperties, in_signature='ssv')
    def Set(self, interface, propname, value):
        '''
        設定指定的屬性值。
        名稱首字元大寫(Dbus名稱慣例)或小寫(HTML5名稱慣例)都接受。
        例如 currentTime 或 CurrentTime 都可以設定同一個屬性的值。
        '''
        pass


def detect_video_information(mrl):
    '''
    return:
        (width, height, audio_stream_list, duration)
          unit of duration is milliseconds.
        (0, 0) - unknown size.
        False - file not found or command failed.
    '''
    try:
        output = subprocess.Popen([OMXPLAYER, "-i", mrl], 
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            env={"LD_LIBRARY_PATH": OMXPLAYER_LIB_PATH}).communicate()
    except OSError:
        return False

    result = output[0].strip()
    #debug(result)
    if result.endswith(' not found.'):
        return False

    video_width = 0
    video_height = 0
    m = re.search(r'Video: .+ (\d+)x(\d+)', result)
    # m will be None or re.Match
    if m:
        log("Video size detected: %s, %s" % (m.group(1), m.group(2)))
        video_width = int(m.group(1))
        video_height = int(m.group(2))
    else:
        debug("Size of video is unknown")

    duration = 0
    m = re.search(r'Duration: (\d+):(\d+):(\d+)\.(\d{2})', result)
    if m:
        log("Duration: %s hours, %s mins, %s seconds, %s ms" % (m.group(1), m.group(2), m.group(3), m.group(4)))
        # convert to milliseconds
        duration = (int(m.group(1)) * 3600 + int(m.group(2)) * 60 + int(m.group(3)))*1000 + int(m.group(4))*10
    else:
        debug("Duration of video is unknown")

    m = re.findall(r'Stream #(.+): Audio: (.+)', result)
    if m:
        audio_stream_list = m
    else:
        audio_stream_list = []

    return video_width, video_height, audio_stream_list, duration


def estimate_visual_size(x, y, width, height, video_width, video_height):
    '''
    w, y: 目標視窗預定放置座標
    width, height: 目標視窗最大長寬
    video_width, video_height: 來源影像長寬
    '''
    wg = float(width) / float(video_width)
    hg = float(height) / float(video_height)

    if (wg >= hg):
        visual_w = int(round(hg * video_width))
        visual_h = height
    else:
        visual_w = width
        visual_h = int(round(wg * video_height))
    # 目標視窗實際使用長寬 (固定比例縮放)
    debug("visual_w: %d, visual_h: %d" % (visual_w, visual_h))

    center_vertical_offset = 0
    center_horizon_offset = 0

    if (visual_w != width):
        center_horizon_offset = (width - visual_w) / 2
    if (visual_h != height):
        center_vertical_offset = (height - visual_h) / 2
    visual_x = int(round(x + center_horizon_offset))
    visual_y = int(round(y + center_vertical_offset))
    # 目標視窗實際放置座標 (置中於預期區域)
    debug("visual_x: %d; visual_y: %d" % (visual_x, visual_y))
    return visual_x, visual_y, visual_w, visual_h


def run_console_command(cmd):
    return subprocess.call(cmd.split())

# http://www.raspberrypi.org/phpBB3/viewtopic.php?f=35&t=9789
def turn_off_cursor():
    run_console_command('setterm -cursor off')

def turn_on_cursor():
    run_console_command('setterm -cursor on')

def prevent_screensaver():
    run_console_command('setterm -blank off -powerdown off')

# TODO 在 system bus 下，似乎不能使用 xrefresh 。
def xrefresh():
    debug("invoke xrefresh %s" % Xrefresh_Display)
    run_console_command('xset -display %s -q ' % Xrefresh_Display)
    run_console_command('xrefresh -display %s' % Xrefresh_Display)


def kill_process(pid):
    try:
        os.kill(pid, signal.SIGKILL)
    except:
        pass


def terminate_self(signum, func):
    global Service
    Service.terminate_all_players()
    remove_pid_file()
    log("Terminate service (signal: %d)" % signum)
    #sys.exit(0) # this will be hang.
    os._exit(0)


def get_pid_filepath():
    'this service is run by nobody. it could not save pid to /var/run.'
    return "/tmp/omxplayer-dbus-service-%s.pid" % Bus_Type


def remove_pid_file():
    try:
        os.remove(get_pid_filepath())
    except:
        pass


def log_pid_file():
    pid = os.getpid()
    try:
        pidfile = open(get_pid_filepath(), "w")
    except:
        pass
    else:
        pidfile.write("%d" % pid)
        pidfile.close()


def log(s):
    log_msg = "[%s] %s" % (time.asctime(), s)
    print(log_msg)

    if not Log_Mode:
        return

    # 安裝時，會設定此目錄為 nobody 可寫入。
    log_filepath = "/var/log/omxplayer-dbus/%s.log" % Bus_Type

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

    """)
    os._exit(status)

#def g_event_timer(*args):
#    print("tick")
#    return True

if __name__ == "__main__":
    os.chdir("/")
    try:
        opts, args = getopt.getopt(sys.argv[1:], "dhq", 
            ["debug", "help", "quiet", "session"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print(str(err)) # will print something like "option -a not recognized"
        usage(status=2)

    Debug_Mode = False
    Bus_Type = 'system'

    for o, a in opts:
        print("%s %s" % (o, a))
        if o in ("-d", "--debug"):
            Debug_Mode = True
        elif o in ("-q", "--quiet"):
            Log_Mode = False
        #elif o in ("--xrefresh"):
        #    debug('set xrefresh %s' % a)
        #    Xrefresh_Display = a
        elif o in ("--session"):
            Bus_Type = 'session'
        elif o in ("-h", "--help"):
            usage()

    # You must do this before connecting to the bus.
    DBusGMainLoop(set_as_default=True) # if you use DBus.
    loop = gobject.MainLoop()
     
    # enable g_event_timer tick.
    #gobject.timeout_add(10000, g_event_timer)

    if Bus_Type == 'session':
        bus = dbus.SessionBus()
    else:
        bus = dbus.SystemBus()
    Service = ListService(bus, loop)

    if Debug_Mode:
        _debug_text = "ON"
    else:
        _debug_text = "OFF"

    log('omxplayer-dbus-service startup. Debug mode [%s]' % _debug_text)
    log_pid_file()

    try:
        loop.run()
    except:
        pass
    terminate_self(signal.SIGTERM, None)
    os._exit(0)    

