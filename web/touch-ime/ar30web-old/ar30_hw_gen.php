<?php
define('TEXTS_PER_PAGE', 30);

#iconv_set_encoding('internal_encoding', 'UTF-8');
#iconv_set_encoding('input_encoding', 'UTF-8');
#iconv_set_encoding('output_encoding', 'UTF-8');

$basic_keys = array(
    'AQZ',
    'SWX',
    'CDE',
    'FRV',
    'BGT',
    'HNY',
    'JMU',
    ',IK',
    '.LO',
    'P/;'
);

$tab = fopen('ar30_hw.txt', 'r');

fgets($tab); // skip first comment.

$codes = array();
while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s))
        break;

    $k = rtrim(substr($s, 0, 2));
    $t = trim(preg_replace('/□\s?/', '', substr($s, 2)));
    $codes[$k] = preg_split('/\s+/', $t);
    #echo '[', $k, ']: ', $t, "\n";
}

#echo "一級簡碼----\n";
$texts = array();
for ($i = 0; $i < 10 ; ++$i) {
    $texts[$i] = array();
    for ($j = 0; $j < 3; ++$j) {
        $k = $basic_keys[$i][$j];
        $texts[$i] = array_merge($texts[$i], $codes[$k]);
    }
}

for ($i = 0; $i < 10; ++$i) {
    $t = array();
    foreach ($texts[$i] as $v) {
        if (empty($v))
            continue;
        $t[] = $v;
    }
    $ik = ($i < 9 ? $i + 1 : 0);
    file_put_contents("tbl/$ik.tab", implode(',', $t));
}

#print_r($texts);

#echo "二級簡碼----\n";
$texts = array();
for ($i = 0; $i < 10 ; ++$i) {
    $texts[$i] = array();
    for ($i2 = 0; $i2 < 10; ++$i2) {
        $texts[$i][$i2] = array();
        for ($j = 0; $j < 3; ++$j) {
            for ($j2 = 0; $j2 < 3; ++$j2) {
                $k = $basic_keys[$i][$j] . $basic_keys[$i2][$j2];
                $texts[$i][$i2] = array_merge($texts[$i][$i2], $codes[$k]);
            }
        }
    }
}

for ($i = 0; $i < 10; ++$i) {
    for ($i2 = 0; $i2 < 10; ++$i2) {
        $t = array();
        foreach ($texts[$i][$i2] as $v) {
            if (empty($v))
                continue;
            $t[] = $v;
        }
        $ik = ($i < 9 ? $i + 1 : 0);
        $i2k = ($i2 < 9 ? $i2 + 1 : 0);
        file_put_contents("tbl/$ik-$i2k.tab", implode(',', $t));
    }
}

#print_r($texts);


#var_dump($code1);
#$c = explode(' ', $code1[0]);
#print_r($c);
exit(0);

?>

