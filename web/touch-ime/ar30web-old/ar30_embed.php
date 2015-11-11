#!/usr/bin/php
<?php
//$tbls = new DirectoryIterator('glob://tbl/*.tab');  # php 5.3 or later
$tbls = glob('tbl/*.tab');

$size_count = 0;
$word_tbl = array();

foreach ($tbls as $tbl) {
    //echo $tbl->getFilename(), "\n";
    //$size_count += $tbl->getSize();
    //echo $tbl, "\n";
    $size_count += filesize($tbl);

    $tbl_key = substr($tbl, 4, -4); // strip 'tbl/' and '.tab.'
    //echo $tbl_key, "\n";
    $contents = trim(file_get_contents($tbl));
    $word_tbl[$tbl_key] = $contents;
}

//file_put_contents('word_table.js', json_encode($word_tbl));
//echo $size_count / 1024, " KB";

$src = file_get_contents('index.html');

$src = str_replace(
    'var embedded_word_tab = false;', 
    'var embedded_word_tab = ' . json_encode($word_tbl) . ';', 
    $src);

file_put_contents('index2.html', $src);
/*
In firefox's javascript engineer, embedded_word_tab will take above 900KB heap space.
(1.3MB - 404KB)
*/
exit(0);
?>
