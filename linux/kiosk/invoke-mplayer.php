<?php
//system("mplayer ~/Videos/tvking.mp4");
#$geometry = $_GET['width'] . 'x' . $_GET['height'] . '+' . $_GET['left'] . '+' . $_GET['top'];
#    $file = $_GET['src'];
const VIDEO_ROOT = '/home/rock/Videos/';

if (!isset($_GET['src'])) {
    header('HTTP/1.0 404 Not play');
    exit(0);
}

$video_path = VIDEO_ROOT . $_GET['src'];
$v_width = (int)$_GET['width'];
$v_height = (int)$_GET['height'];
$v_left = (int)$_GET['left'];
$v_top = (int)$_GET['top'];

if (strpos($video_path, './') or !file_exists($video_path)) {
    header('HTTP/1.0 404 Not play');
    exit(0);
}

system("mplayer -quiet -noborder -vo vdpau -vc ffmpeg12vdpau,ffvc1vdpau,ffh264vdpau -ontop -geometry $geometry $video_path");

?>
