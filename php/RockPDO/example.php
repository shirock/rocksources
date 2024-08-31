<?php
require 'RockPDO.php';

/*
CREATE TABLE table1
(
    id integer,
    name character varying(20),
    "A""B" integer
)
*/

$db = new RockPDO('pgsql:host=localhost;port=5432;dbname=localdb', 'postgres');
// echo $db->quote_identifier('A"B'), "\n";
// echo $db->quote_identifier('id'), "\n";

$rows = [
    ['id'=>1, 'name'=>'abc'],
    ['id'=>2, 'name'=>'def'],
];

// $db->insert('table1', ['id'=>3, 'name'=>'ghi']);
// $db->insert('table1', $rows);
// $db->insert_rows('table1', $rows);

// $db->update('table1', ['id'=>1], ['name'=>'Rock']);
// $db->update('table1', ['id'=>3], ['A"B'=>10]);
// $db->update('table1', ['id'=>'> 1'], ['name'=>'Rock']);
// $db->update('table1', ['id'=>'<1'], ['name'=>'Rock']);
// $db->update('table1', ['id'=>'>=1'], ['name'=>'Rock']);
// $db->update('table1', ['id'=>'<>1'], ['name'=>'Rock']);
// $db->update('table1', ['id'=>'<> 1', 'name'=>'abbc'], ['name'=>'Rock']);

$stat = $db->select('table1');

$stat = $db->select('table1', ['id', 'name']);

// id = 1
$stat = $db->select('table1', '*', ['id'=> 1]);
$stat = $db->select('table1', '*', ['id'=>['=', 1]]);

// 原先的條件式規則，是把比較符號放進代表value的字串。
// 若value第一個字元是比較符號，會當作使用者要特定比較行為。
// 但這種寫法不可靠。例如value真的是 > 開頭的文字，就會產生錯誤的查詢敘述。
//$stat = $db->select('table1', '*', ['id'=>'< 1']); // SQL Error

// id < 1 或 >, <>, >=, <=
// 用陣列包裹比較符號和值
$stat = $db->select('table1', '*', ['id'=>['<', 1]]);
$stat = $db->select('table1', '*', ['id'=>['>', 1]]);
$stat = $db->select('table1', '*', ['id'=>['<>', 1]]);
$stat = $db->select('table1', '*', ['id'=>['>=', 1]]);
$stat = $db->select('table1', '*', ['id'=>['<=', 1]]);

// '!=' 並非 SQL 標準比較符號，但此處視為 '<>'
$stat = $db->select('table1', '*', ['id'=>['!=', 1]]);

// field IS NULL
$stat = $db->select('table1', '*', ['A"B'=>null]);
$stat = $db->select('table1', '*', ['name'=>['=', null]]);

// field IS NOT NULL
$stat = $db->select('table1', '*', ['name'=>['<>', null]]);

$rows = $db->fetch_all_objects('table1', ['name' => [fn($v)=>['LIKE', '%'.$v.'%'], 'e']]);
// 最初覺得用 arrow function 解決 LIKE 語法就夠了，但語義不夠清楚
// 所以還是加了'contains','starts_with','ends_with'條件敘述
$rows = $db->fetch_all_objects('table1', ['name' => ['contains',   'e']]);
$rows = $db->fetch_all_objects('table1', ['name' => ['starts_with', 'd']]);
$rows = $db->fetch_all_objects('table1', ['name' => ['ends_with',   'f']]);

$stat = $db->select('table1', '*', ['id'=>['>=', 2], 'name'=>'def']);
$stat = $db->select('table1', null, null, 'id');

// field is bool
$stat = $db->select('table1', '*', ['A"B'=>true]);
$stat = $db->select('table1', '*', ['A"B'=>false]);

// print_r($stat->fetchAll(PDO::FETCH_OBJ));

echo $db->sprintf('select * from ?? where id = ? and name = ?', 'table1', 3, 'ghi'), "\n";

$stat = $db->query_formatted('select * from ?? where id = ? and name = ?', 'table1', 3, 'ghi');

$stat = $db->query_formatted('select * from ??0 where id = ?2 and name = ?1', 'table1', 'ghi', 3);
$row = $stat->fetchObject();
print_r($row);

$rows = $db->fetch_all_objects('table1');
print_r($rows);

$row = $db->fetch_object('table1', 2);
print_r($row);

$enum = $db->enumerate('table1');
print_r($enum);
?>