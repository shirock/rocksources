#!/usr/bin/php
<?php
$words = array();

$tab = fopen('ar30_hw.txt', 'r');

fgets($tab); // skip first comment.

#echo "一、二級簡碼----\n";
while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s))
        break;

    $k = strtolower(rtrim(substr($s, 0, 2)));
    $t = trim(preg_replace('/□\s?/', '', substr($s, 2)));
    $words[$k] = preg_split('/\s+/', $t);
    #echo '[', $k, ']: ', $t, "\n";
}


$tab = fopen("ar30.cin", "r");

while (TRUE)
{
    $s = rtrim(fgets($tab));
    if ($s == '%quick end')
        break;
}

while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s) or $s[0] == '#')
        break;
    if ($s[0] == '%')
        continue;

    $tok = preg_split('/\s+/', $s);
    if (count($tok) < 2)
        continue;
    list($k, $w) = $tok;
    $k = strtolower($k);
    if (strlen($k) > 4) {
        $k = substr($k, 0, 4);
    } 
    if (!isset($words[$k])) 
        $words[$k] = array();
    if (!in_array($w, $words[$k]))
        $words[$k][] = $w;
    #echo $k, ': ', $tok[1], "\n";
}
fclose($tab);

//print_r($words);
echo "count of key: ", count($words), "\n";

$words_joined = array();
foreach ($words as $k => $ws) {
    $words_joined[$k] = implode(',', $ws);
}
//print_r($words_joined);

$json_tab = fopen('array30-tbl.js', 'w');
fwrite($json_tab, json_encode($words_joined));
fclose($json_tab);

exit(0);
?>
