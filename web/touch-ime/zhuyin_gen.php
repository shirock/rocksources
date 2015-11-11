#!/usr/bin/php
<?php
$tab = fopen("zhuyin.txt", "r");

while (TRUE)
{
    $s = rtrim(fgets($tab));
    if ($s == 'BEGIN_TABLE')
        break;
}

#var_dump($code1);
#$c = explode(' ', $code1[0]);
#print_r($c);
#exit;

$words = array();
while (TRUE)
{
    $s = rtrim(fgets($tab));
    if (empty($s) || $s == 'END_TABlE')
        break;

    $tok = preg_split('/\s+/', $s);
    if (count($tok) < 2)
        continue;
    list($k, $w) = $tok;
    if (!isset($words[$k])) 
        $words[$k] = array();
    $words[$k][] = $w;
    echo $tok[0], ': ', $tok[1], "\n";
}
fclose($tab);

//print_r($words);
echo "count of key: ", count($words), "\n";

$words_joined = array();
foreach ($words as $k => $ws) {
    $words_joined[$k] = implode(',', $ws);
}
//print_r($words_joined);

//$words_joined['~s'] = file_get_contents('s.tab');
//$words_joined['~w'] = file_get_contents('w.tab');
//$words_joined['~en'] = file_get_contents('en.tab');

$json_tab = fopen('zhuyin-tbl.js', 'w');
//fwrite($json_tab, "/*zhuyin*/\n");
//fwrite($json_tab, "var embedded_word_tab = ");
fwrite($json_tab, json_encode($words_joined));
//fwrite($json_tab, ";\n");
fclose($json_tab);

exit(0);
?>
