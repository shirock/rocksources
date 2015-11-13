database-query
--------------

原先叫 Schema-Database 。但後來覺得 schema 這塊的使用率並不高，而且不好記，所以現在改稱 *database-query* 。

database-query 是一個針對小型應用系統或嵌入式系統所設計的抽象資料庫存取層函數庫。 它提供一組 Schema class 用於組態資料庫表格與欄位，強化資料內容的型態檢查、格式檢查、解碼與編碼動作。 同時設計了兩種 class, Database_Query 和 Database_Row ，提供基礎的 Active Record 與 ORM 能力。這兩種 class 運用 PDO class 與 Schema class ，簡化資料查詢、存取和資料內容檢查動作。

這套 library 功能並不多。我覺得 PDO 已經夠抽象了，沒有必要再加一層，所以這套 library 只用 PDO 作為資料存取層，而不另行設計。但是它特別加上了 Schema 類別，強化資料內容的型態檢查、格式檢查、解碼與編碼動作。一但你將格式(pattern)、編碼(encoder)、解碼(decoder)動作組織到 Schema 後，它就會在必要之處運用它們，你不必再自己處理。

我將資料庫的操作動作劃分成兩個類別，一個主要負責查詢，另一個負責新增、更新與刪除。

在應用系統中，大約有八成以上的資料庫操作動作是查詢。Database_Query 針對這種情形 提供了簡便的查詢語法，產生查詢句並執行之，取代我們自行組織 SQL 查詢句。 Database_Row 的查詢結果集只是一組含有記錄個體的普通陣列。我們讀取這些結果集的方法， 與使用其他 Database library 時的方式並無差異。

當我們需要更新或刪除查詢結果集的其中一筆記錄時，這時就輪到 Database_Row 發揮功用。 我們取出這筆記錄包入 Database_Row，利用它的方法異動此筆記錄。 在小型應用系統中，我們幾乎不會批次異動整個資料結果集，Database_Row 只提供恰到好處的處理功能。

Database_Query 與 Database_Row 是互補的類別，同時也不干涉你原本的程式框架。

主要介紹文章請閱讀 [石頭閒語 - PHP - database-query](http://rocksaying.tw/archives/2009/PHP%20-%20Schema-Database.html) 。

### Query

它的查詢動作如下:

```php
$results = $query->from('Test')->select();

$results = $query->from('Test')->select('id');

$query->from('Test')
      ->where(array('id' => 10))
      ->select(array('id', 'name'));

$query->from('Test')
      ->where(array('password' => '123')) //it will encode '123' by your schema.
      ->select();

$total_qty_expr = $query->prepare('SUM(:qty)', array('qty' => ''));

$query->from('Order')
      ->where(array('qty >' => 30))
      ->group_by('id')
      ->order_by('total_qty')
      ->select(array('id', 'total_qty' => $total_qty_expr));
```

如果你覺得這種語法很像 LINQ ，不用懷疑，我就是抄 LINQ 的。

### Update and ORM

它也提供了基本的 ORM 功能，例如 :

```php
$data = array(
    'id'  => 1,
    'name'    => 'admin',
    'password' => '123',
    'permission'    => 0,
    'email'     => 'admin@abc.com'
);

$row->assign($data);

$row->insert();

//You can use array index to access field.
echo $row['name'];

//Also you can use object property to access field.
echo $row->permission;

$row->email = 'rock@abc.com';

$row->update();

$id = $row->id;

$result = $row->get($id);

$this->assertEquals('rock@abc.com', $result->name);

$row->delete();

$result = $row->get($id);

$this->assertTrue( empty($result) );
```
