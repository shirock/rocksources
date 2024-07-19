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
// $stat = $db->select('table1', ['id', 'name']);
// $stat = $db->select('table1', '*', ['A"B'=>null]);
// $stat = $db->select('table1', '*', ['id'=>'> 1']);
// $stat = $db->select('table1', '*', ['id'=>2, 'name'=>'def']);
// $stat = $db->select('table1', null, null, 'id');
print_r($stat->fetchAll(PDO::FETCH_OBJ));

echo $db->sprintf('select * from ?? where id = ? and name = ?', 'table1', 3, 'ghi'), "\n";

$stat = $db->query_formatted('select * from ?? where id = ? and name = ?', 'table1', 3, 'ghi');
$row = $stat->fetchObject();
print_r($row);

$stat = $db->query_formatted('select * from ??0 where id = ?2 and name = ?1', 'table1', 'ghi', 3);
$row = $stat->fetchObject();
print_r($row);
?>