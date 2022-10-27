<?php
// 注意，自 .NET6 起，使用 unix domain socket 實作 NamedPipe ，而不是用 FIFO 。
// $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$socket = socket_create(AF_UNIX, SOCK_STREAM, 0);

$ip = $argv[1];
$port = null;
echo "connect to $ip\n";

$msg = $argv[2];

if (socket_connect($socket, $ip, $port) === false) {
    echo socket_strerror(socket_last_error()), "\n";
    exit(1);
}

for ($i = 2; $i < $argc; $i++) {
    $rc = socket_write($socket, $argv[$i]);
    if ($rc === false) {
        echo socket_strerror(socket_last_error()), "\n";
        exit(1);
    }

    $result = socket_read($socket, 8192);
    if ($result === false) {
        echo socket_strerror(socket_last_error()), "\n";
        exit(1);
    }

    echo "Response: '$result'\n";
}

socket_close($socket);
?>