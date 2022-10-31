<?php
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
// $socket = socket_create(AF_UNIX, SOCK_STREAM, 0);

// usage: tcp-client.php [address:port] [messages ...]
// ex: tcp-client.php localhost:4321 abc hello world!
list($ip, $port) = explode(':', $argv[1]);
echo "connect to $ip:$port\n";

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