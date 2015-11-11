<meta charset="utf-8"/>
<?php
require_once 'config.php';
$video_root_state = file_exists(VIDEO_ROOT) ? 'Ok' : 'Failed';

$dbus_state = class_exists('DBus') ? 'Ok' : 'Failed';
if (!$dbus_state) {
    echo "php-dbus: Failed\n";
    exit;
}

const MPLAYER_SERVICE_NAME = 'raspberry.pi.OMXPlayer';
const MPLAYER_OBJECT_PATH = '/raspberry/pi/OMXPlayer';
const MPLAYER_LIST_INTERFACE = 'raspberry.pi.OMXPlayerList';

$dbus = new DBus(DBus::BUS_SESSION);
$dbus_session_state = ($dbus ? 'Ok' : 'Failed');

if ($dbus) {
    $mplayer_list = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
        MPLAYER_OBJECT_PATH, MPLAYER_LIST_INTERFACE);
}
else {
    $mplayer_list = false;
}
$mplayer_state = ($mplayer_list ? 'Ok' : 'Failed');

$mplayer_response_state = 'Ok';
$headers = get_headers($_SERVER["HTTP_REFERER"] . 'mplayer.php?check=1');
//print_r($headers);
$mplayer_response_state = strpos($headers[0], '400 No media') ? 'Ok' : 'Failed';
?>
<p>
Check points:
</p>
<ul>
    <li>VIDEO_ROOT: <?=$video_root_state?>
    <li>php-dbus: <?=$dbus_state?>
    <li>apache-session: <?=$dbus_session_state?>
    <li>omxplayer-dbus: <?=$mplayer_state?>
    <li>request mplayer.php: <?=$mplayer_response_state?>
</ul>
