<?php
require_once '../database_query.php';

$db = new PDO('sqlite::memory:');
$query = new DatabaseQuery(array('db' => $db));

$parameters = array(
    'age'   => 10
    );

// My prepare syntax example:
$my_st = "SELECT :id, :name, :age, sum(:qty) FROM :user WHERE :age >= ?;";
$sql_statment = $query->prepare($my_st, $parameters);

echo $sql_statment, "\n"; // if will quote field name too.


// PDO prepare syntax example:
$pdo_st = "SELECT id, name, age FROM user WHERE age >= :age;";

$pdo_statment = $db->prepare($pdo_st, $parameters);

//$sub_expr = $query->prepare('SUBSTR(:name, 0, 5)', array());
//echo $sub_expr, "\n";

$total_qty_expr = $query->prepare('SUM(:qty)', array());

$query->from('Order')
    ->select(array('id', 'total' => $total_qty_expr));
echo $query->getLastQueryString(), "\n";
?>
