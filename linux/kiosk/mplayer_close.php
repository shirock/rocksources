<?php
const MPLAYER_SERVICE_NAME = 'raspberry.pi.OMXPlayer';
const MPLAYER_OBJECT_PATH = '/raspberry/pi/OMXPlayer';
const MPLAYER_LIST_INTERFACE = 'raspberry.pi.OMXPlayerList';
const MPLAYER_PLAYER_INTERFACE = 'raspberry.pi.OMXPlayer';

if (empty($_GET)) {
    $id = $argv[1];
}
else {
    $id = $_GET['id'];
}

$dbus = new DBus(DBus::BUS_SESSION);

$list = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
    MPLAYER_OBJECT_PATH, MPLAYER_LIST_INTERFACE);

$path_found = false;

$playlist = $list->List()->getData();
foreach ($playlist as $dbus_listitem) {
    $dbus_item = $dbus_listitem->getData();
    $playpath = $dbus_item[0]->getData();
    if ($id == 'all') {
        $player = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
            $playpath, MPLAYER_PLAYER_INTERFACE);
        $player->Stop();
    }
    elseif ($playpath == $id) {
        $path_found = true;
    }
}

if (!$path_found) {
    //echo "no matched path\n";
    exit;
}

$player = $dbus->createProxy(MPLAYER_SERVICE_NAME, 
    $id, MPLAYER_PLAYER_INTERFACE);

if (!$player)
    exit;

try {
    $player->Stop();
}
catch (DbusException $e) {
    echo "-ERR failed\n";
    exit;
}

?>
