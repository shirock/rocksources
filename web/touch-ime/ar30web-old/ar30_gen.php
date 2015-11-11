<?php
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

$tab = fopen("ar30.cin", "r");

while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s)) {
        echo 'unexpected format';
        exit(1);
    }
    if ($s == '%quick begin')
        break;
}

// 收集一級碼
$code1 = array();
$code1_map = array(
    'q' => 0,
    'a' => 0,
    'z' => 0,
    'w' => 1,
    's' => 1,
    'x' => 1,
    'e' => 2,
    'd' => 2,
    'c' => 2,
    'r' => 3,
    'f' => 3,
    'v' => 3,
    't' => 4,
    'g' => 4,
    'b' => 4,
    'y' => 5,
    'h' => 5,
    'n' => 5,
    'u' => 6,
    'j' => 6,
    'm' => 6,
    'i' => 7,
    'k' => 7,
    ',' => 7,
    'o' => 8,
    'l' => 8,
    '.' => 8,
    'p' => 9,
    ';' => 9,
    '/' => 9
);

while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s)) {
        echo 'unexpected format';
        exit(1);
    }
    if ($s == '%quick end')
        break;
    list($k, $t) = explode(' ', $s, 2);
    if (!isset($code1[$code1_map[$k]]))
        $code1[$code1_map[$k]] = $t;
    else
        $code1[$code1_map[$k]] .= ' ' . $t;
}

#var_dump($code1);
#$c = explode(' ', $code1[0]);
#print_r($c);
#exit;

$len = count($basic_keys);

$toks = array(
    array(),
    array(),
    array(),
    array(),
    array(),
    array(),
    array(),
    array(),
    array(),
    array()
);
while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s))
        break;

    $tok = preg_split('/\s+/', $s);
    if (count($tok) < 2)
        continue;
    for ($i = 0; $i < $len; ++$i) {
        if (strpos($basic_keys[$i], $tok[0][0]) !== FALSE)
            break;
    }
    $toks[$i][] = $tok;
    //list($k, $t) = $tok;
}
fclose($tab);

$len = count($basic_keys);
for ($i = 0; $i < $len; ++$i) {
    for ($i2 = 0; $i2 < $len; ++$i2) {
        for ($i3 = 0; $i3 < $len; ++$i3) {
            $count = 0;
            $texts = array();
            foreach ($toks[$i] as $tok) {
                list($k, $t) = $tok;
                if (strlen($k) != 3)
                #if (!isset($k[2]))
                    continue;
                $c = $basic_keys[$i] . $basic_keys[$i2] . $basic_keys[$i3];
                if (strpos($basic_keys[$i], $k[0]) === FALSE or 
                    strpos($basic_keys[$i2], $k[1]) === FALSE or
                    strpos($basic_keys[$i3], $k[2]) === FALSE) 
                {
                    continue;
                }
                #echo "$c($count) : $t\n";
                $texts[] = $t;

                ++$count;
                #if ($count >= TEXTS_PER_PAGE) {
                #    break;
                #}
            }
            $ik = ($i < 9 ? $i + 1 : 0);
            $i2k = ($i2 < 9 ? $i2 + 1 : 0);
            $i3k = ($i3 < 9 ? $i3 + 1 : 0);
            #if ($count > TEXTS_PER_PAGE) {
            #    echo "$ik, $i2k, $i3k > TEXTS_PER_PAGE, $count\n";
            #}
            if (!empty($texts))
                file_put_contents("tbl/{$ik}-{$i2k}-${i3k}.tab", implode(',', $texts));
        }
    }
} 

#exit(0);

$len = count($basic_keys);
for ($i = 0; $i < $len; ++$i) 
{
    for ($i2 = 0; $i2 < $len; ++$i2)
    {
        for ($i3 = 0; $i3 < $len; ++$i3)
        {
            $count = 0;
            $texts = array();
            for ($i4 = 0; $i4 < $len; ++$i4)
            {
                foreach ($toks[$i] as $tok) {
                    list($k, $t) = $tok;
                    if (!isset($k[3]))
                        continue;
                    $c = $basic_keys[$i] . $basic_keys[$i2] . $basic_keys[$i3] . $basic_keys[$i4];
                    if (strpos($basic_keys[$i], $k[0]) === FALSE or 
                        strpos($basic_keys[$i2], $k[1]) === FALSE or
                        strpos($basic_keys[$i3], $k[2]) === FALSE or
                        strpos($basic_keys[$i4], $k[3]) === FALSE) 
                    {
                        continue;
                    }
                    $texts[] = $t;

                    ++$count;
                }
            }
            $ik = ($i < 9 ? $i + 1 : 0);
            $i2k = ($i2 < 9 ? $i2 + 1 : 0);
            $i3k = ($i3 < 9 ? $i3 + 1 : 0);
            #echo "$ik, $i2k, $i3k, x: $count\n";
            if (!empty($texts))
                file_put_contents("tbl/{$ik}-{$i2k}-${i3k}-x.tab", implode(',', $texts));
        }
    }
} 

exit(0);
?>

