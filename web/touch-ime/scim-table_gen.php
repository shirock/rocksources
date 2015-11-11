#!/usr/bin/php
<?php
function table_convert($scim_table_filename, $output_filename) 
{
    $tab = fopen($scim_table_filename, "r");

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

    $max_key_len = 0;
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
        if (strlen($k) > $max_key_len)
            $max_key_len = strlen($k);
        echo $tok[0], ': ', $tok[1], "\n";
    }
    fclose($tab);

    //print_r($words);
    echo "count of key: ", count($words), "\n";
    echo "max key len: ", $max_key_len, "\n";

    $words_joined = array();
    foreach ($words as $k => $ws) {
        $words_joined[$k] = implode(',', $ws);
    }
    //print_r($words_joined);

    #$words_joined['~s'] = file_get_contents('s.tab');
    //$words_joined['~w'] = file_get_contents('w.tab');
    //$words_joined['~en'] = file_get_contents('en.tab');

    $json_tab = fopen($output_filename, 'w');
    //fwrite($json_tab, "/*zhuyin*/\n");
    //fwrite($json_tab, "var embedded_word_tab = ");
    fwrite($json_tab, json_encode($words_joined));
    //fwrite($json_tab, ";\n");
    fclose($json_tab);
}

table_convert("quick-classic.txt", "quick-tbl.js");

exit(0);
?>
