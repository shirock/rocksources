<?php
/*
MQTT publish flow.

Author: rock <shirock.tw@gmail.com>
Web site: https://rocksaying.tw/
License: LGPL or CC BY-SA.

這是針對 NB-IoT 裝置寫的 MQTT 訊息發佈範例。
這些裝置可能沒有完整的 MQTT 函數庫；
或者不具有乙太網路介面，只能透過通訊模組的 AT 指令送出資料封包。

通訊模組使用者，請將本範例的 stream_socket_client(), fwrite(), fread() 換成你的 AT 指令。

http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/errata01/os/mqtt-v3.1.1-errata01-os-complete.html

以中華電信 IoT 平台為例: https://iot.cht.com.tw/iot/developer/mqtt
*/

$mqtt_broker = 'iot.cht.com.tw:1883';
$username = 'PROJECT_KEY or DEVICE_KEY'; // 填入你的專案鑰匙或設備金鑰匙
$password = 'PROJECT_KEY or DEVICE_KEY';
$username = 'rock'; // 填入你的專案鑰匙或設備金鑰匙
$password = 'password';
$client_id = gethostname() . '-' . getmypid();

// 填入你的設備編號
$topic = '/v1/device/${device_id}/rawdata ';

$msg = json_encode([[
    'id' => '123321', // 填入感測器編號
    'value' => ["123", "456", "789"],
    'time' => strftime('%Y-%m-%dT%H:%M:%S')
]]);

$qos = 0;

// 開始 MQTT 訊息發佈流程

$data_count = 0;

$socket = stream_socket_client('tcp://' . $mqtt_broker, 
    $errno, $errstr, 60, STREAM_CLIENT_CONNECT);
stream_set_timeout($socket, 5);
// stream_set_blocking($socket, 0);
function str_content($str)
{
    $len = strlen($str);
    $msb = $len >> 8;
    $lsb = $len & 0xff;
    return chr($msb) . chr($lsb) . $str;
}

//==== begin CONNECT

$variable_header = [];
$variable_header[] = "\x00\x04MQTT\x04";

// 註: MQTT協定文件， byte 以 1 為基底， bit 以 0 為基底。
// byte 8: Flags
// bit 7: username
// bit 6: password
// bit 1: clean session
$variable_header[] = chr(128+64+2);

// byte 9 ~ 10: keep alive
// 10 seconds.
$variable_header[] = "\x00\x0A";

$variable_header[] = str_content($client_id);
$variable_header[] = str_content($username);
$variable_header[] = str_content($password);

$variable_header = implode('', $variable_header);

# CONNECT cmd: 0x10
$fixed_header = chr(0x10) . chr(strlen($variable_header));

fwrite($socket, $fixed_header);
fwrite($socket,  $variable_header);

$rb = fread($socket, 4);

// $rb[0] is CONNACK cmd 0x20.
// $rb[3] is error code.
if ($rb[0] == "\x20" && $rb[3] == "\x0") {
    echo "connected\n";
}
else {
    echo "failed to connect.\n";
    exit(1);
}

$data_count = strlen($fixed_header) + strlen($variable_header) + 4;
//==== end CONNECT

//==== begin PUBLISH
$payload = [];
$payload[] = str_content($topic);

if ($qos > 0) {
    $packet_id = rand();
    $packet_id = chr($packet_id >> 8) . chr($packet_id & 0xFF);
    // echo "packet_id ${packet_id}\n";
    echo "packet_id ", ord($packet_id[0]), ord($packet_id[1]), "\n";
    $payload[] = $packet_id;
}

$payload[] = $msg;
$payload = implode('', $payload);

// publish 的 Remaining Length 是取摘要值。
$digest = '';
$payload_len = strlen($payload);
echo "len $payload_len\n";
do {
    $digit = $payload_len % 128;
    $payload_len >>= 7;
    if ($payload_len > 0)
        $digit |= 0x80;
    $digest .= chr($digit);
} while ($payload_len > 0);

// PUBLISH cmd: 0x30
// bit1~bit2 = qos
// 0x30 publish qos 0
// 0x32 publish qos 1
// 0x34 publish qos 2
$cmd = 0x30 + ($qos << 1);
$fixed_header = chr($cmd) . $digest;

echo "fix \n";
for ($i = 0; $i < strlen($fixed_header); ++$i)
    echo dechex(ord($fixed_header[$i])), ',';
echo "\n";

fwrite($socket, $fixed_header);
fwrite($socket, $payload);
$data_count += strlen($fixed_header) + strlen($payload);
echo "message $msg \n";

if ($qos > 0) {
    $rb = fread($socket, 4);
    $data_count += 4;
    // PUBACK 0x40 for qos 1
    // PUBREC 0x50 for qos 2
    if ($rb[0] == chr((0x4 + ($qos >> 1)) << 4) && $packet_id == $rb[2].$rb[3]) {
        if ($qos == 1)
            echo "published (qos 1)\n";
        else
            echo "publish rec (qos 2)\n";
    }
    else {
        echo "failed to published.\n";
        exit(1);
    }
    if ($qos == 2) {
        $fixed_header = "\x62\x02"; // PUBREL
        fwrite($socket, $fixed_header);
        fwrite($socket, $packet_id);
        $rb = fread($socket, 4);
        $data_count += 8;
        // 0x70 PUBCOMP
        if ($rb[0] == chr(0x70) && $packet_id == $rb[2].$rb[3]) {
            echo "publish complete (qos 2)\n";
        }
    }
}
else {
    echo "publish (qos 0)\n";
}
//==== end PUBLISH

// DISCONNECT cmd: 0xE0
$fixed_header = "\xE0\x00";
fwrite($socket, $fixed_header);
$data_count += 2;
echo "disconnect\n";

echo "總共使用了 ${data_count} bytes\n";
?>