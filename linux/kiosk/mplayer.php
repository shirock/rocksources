<?php
/*
mplayer.php Copyright (C) 2014 遊手好閒的石頭成 <shirock.tw@gmail.com>

mplayer.php is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or 
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see 
<http://www.gnu.org/licenses/lgpl-3.0-standalone.html>.

You should see https://rocksources.googlecode.com/ to get more 
information about mplayer.php.
*/

//const VIDEO_ROOT = '/home/kiosk/Videos/';
require_once 'config.php';

if (empty($_GET)) {
    $mrl = '/home/kiosk/Videos/720p/demo.mp4';
    $v_width = 640;
    $v_height = 360;
    $v_left = 100;
    $v_top = 300;
    $keep_aspect = true;
}
else {
    if (!isset($_GET['src'])) {
        header('HTTP/1.0 400 No media special');
        exit(1);
    }

    $mrl = VIDEO_ROOT . $_GET['src'];
    $v_width = (int)$_GET['width'];
    $v_height = (int)$_GET['height'];
    $v_left = (int)$_GET['left'];
    $v_top = (int)$_GET['top'];
    $keep_aspect = ($_GET['keep_aspect'] == 'true' ? true : false);

    if (strpos($mrl, './') or !file_exists($mrl)) {
        header('HTTP/1.0 404 Not play');
        exit(1);
    }
}

const MPLAYER_SERVICE_NAME = 'raspberry.pi.OMXPlayer';
const MPLAYER_OBJECT_PATH = '/raspberry/pi/OMXPlayer';
const MPLAYER_LIST_INTERFACE = 'raspberry.pi.OMXPlayerList';
const MPLAYER_PLAYER_INTERFACE = 'raspberry.pi.OMXPlayer';
const PROPERTY_INTERFACE = 'org.freedesktop.DBus.Properties';

$dbus = new DBus(DBus::BUS_SESSION);

$mplayer_list = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
    MPLAYER_OBJECT_PATH, MPLAYER_LIST_INTERFACE);

try {
    $player_path = $mplayer_list->Play($mrl, 
        $v_left, $v_top, $v_width, $v_height, 
        $keep_aspect, '');
}
catch (DbusException $e) {
    header('HTTP/1.0 404 Not play');
    print_r($e);
    exit(1);
}

$player_path = $player_path->getData();

$player = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
    $player_path, MPLAYER_PLAYER_INTERFACE);

$player_prop = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
    $player_path, PROPERTY_INTERFACE);

$duration = $player_prop->Get(MPLAYER_PLAYER_INTERFACE, 'duration')->getData();

if (PHP_SAPI != 'cli') {
    header('Content-type: text/plain');
    header('Cache-Control: no-cache');
    header('Pragma: no-cache');

    header('X-Media-Object-Id: ' . $player_path);
    header('X-Media-Object-Duration: ' . $duration);
    flush();
}

$dbus->addWatch(MPLAYER_LIST_INTERFACE);

$start_time = $end_time = $timestamp = 0;

while (true) {
    #echo "wait...\n";
    if ( !($s = $dbus->waitLoop(500)) ) {
        if ($timestamp < 1) {
            $start_time = $timestamp = time();
            $end_time = $start_time + ceil($duration / 1000) + 3; //delay 3 seconds.
        }
        else {
            $current_time = time();
            if ($current_time >= 5 + $timestamp) {
                echo 'time: ', $current_time - $start_time, "\n";
                $timestamp = $current_time;
            }
            if ($current_time > $end_time) {
                echo "Time up\n";
                $player->Stop();
                break;
            }
        }
        continue;
    }

    if ($s->matches(MPLAYER_LIST_INTERFACE, 'Event')) {
        $event_args = $s->getData()->getData();
        $event_path = $event_args[0]->getData();
        if ($event_path != $player_path)
            continue;
        $event_msg = $event_args[1];
        echo $event_path, ':', $event_msg, "\n";
        if ($event_msg == 'closed') {
            break;
        }
    }
}
?>
