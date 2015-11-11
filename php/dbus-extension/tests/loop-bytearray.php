<?php
$s = file_get_contents('image.png');
echo "PID: ", posix_getpid(), "\n";

$ba = new ByteArray($s);
//echo "Length of bytearray: ", strlen($ba), ", ", count($ba), "\n";
//echo $ba[1], $ba[2], "\n";

$ba = new ByteArray('123');

fgets(STDIN);

for ($i = 0; $i < 200; ++$i) {
    $ba = new ByteArray($s);
    //echo "Length of bytearray: ", strlen($ba), ", ", count($ba), "\n";
    //echo $ba[1], $ba[2], "\n";

    $ba = new ByteArray('123');
    //echo "Length of bytearray: ", strlen($ba), ", ", count($ba), "\n";
    //echo $ba[0], $ba[1], "\n";
}

echo "End\n";
fgets(STDIN);

?>
