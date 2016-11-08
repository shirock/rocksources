php dbus extension with ByteArray support
-----------------------------------------

php-dbus 是來自 PECL DBus 的 PHP 擴展項目，其用途在於讓 PHP 支援 DBus 功能。

我為 php-dbus 添加了一個新類別: *ByteArray* ，再讓它的 DBus 方法支援 *ByteArray* 類別。

*我正準備將此項目升級支持 PHP7 。我任職公司中有許多專案使用了我維護的 php dbus 項目。配合近期專案預設系統往 Ubuntu 16 轉移的期程，這件事成為我正職工作的優先項目。我想這個月就會完成吧(2016-11)。*

### ByteArray

你需要取回此處經我改寫過的 php-dbus 源碼，再按「[Write a PHP DBus client](http://rocksaying.tw/archives/12193781.html)」所說明的方式，安裝 php-dbus 擴展項目，才能使用 ByteArray 類別。

ByteArray 可以視為 PHP 字串的 box 類別。在大多數場合可以直接當成字串運算。

建構時需要傳給它一個字串。其他時候和一般字串沒兩樣。用例如下:

```php
$src = '123456';

$ba = new ByteArray($src);

echo get_class($ba), "\n"; // ByteArray

echo count($ba), "\n";  // array behavior
echo strlen($ba), "\n"; // string behavior

echo "interpolate $ba\n";;

echo $ba[0], "\n";  // indexer get.
echo $ba[-1], "\n"; // I allow negative position.

echo isset($ba[1]) ? 'yes' : 'no', \n";

$ba[0] = 'a';       // indexer set.
$ba[-1] = 'z';
```

參考 tests 中的「ByteArrayTest.php」了解 ByteArray 現行可用的方法。

### 用於 DBus 的場合

示範用 DBus 服務 'com.transtep.sample.Hello'，提供兩個方法。

* int SayAy(array); 傳入一個陣列，回傳陣列大小。
* ByteArray RetAy(filepath); 傳入一個檔案路徑，回傳檔案內容 (binary string)。

```php
$sample_service = 'blog.rock.sample.Hello';
$sample_object_path = '/blog/rock/sample/Hello';
$sample_interface = 'blog.rock.sample.Hello';

$d = new Dbus(Dbus::BUS_SYSTEM);

$proxy = $d->createProxy(
    $sample_service, $sample_object_path, $sample_interface);

$a = new ByteArray("sf\0sfs f");
echo $a, "\n";
echo strlen($a), "\n";

$res = $proxy->SayAy(array(1,2,3,4)); // pass a DBusArray
echo $res, "\n";    // return length of array.

$res = $proxy->SayAy($a);   // pass a ByteArray
echo $res, "\n";    // return length of array.

$filepath = "/home/rock/Pictures/wallpaper/1.jpg";

$s = file_get_contents($filepath);

echo "size of file: ", strlen($s), "\n";

$res = $proxy->SayAy(new ByteArray($s));
echo $res, "\n";

$a = $proxy->RetAy($filepath); // return the content of file.
echo get_class($a), "\n";
echo strlen($a), "\n";
```

在沒有 ByteArray 之時，我們必須先將 binary 字串中的每個字元依序取出內碼(使用 chr 函數)，放入一個 PHP 陣列中， php-dbus 才能正確地傳遞給 DBus 服務。 這帶來額外的效能問題，還牽扯到一個 PHP 內部記憶體管理的問題。請看下列 PHP 程式，各位猜猜這個 PHP 程式要配置多少記憶體？

```php
<?php
$l = 500000; // 500KB

$a = array();

for ($i = 0; $i < $l; ++$i)
    $a[] = 1;

sleep(60*5);
?>
```

執行上述程式後，它會睡眠5分鐘不動。各位可以用記憶體觀察程式，例如 Linux 可用 top 指令或 gnome-system-monitor ，查看這個程式為長度 500000 的陣列耗用多少記憶體。我自己觀察到的結果是 100MB 起跳，遠遠超過 500KB 。

使用 ByteArray 就可以避免上述兩個問題。

###### 相關文章

更多相關文章請查閱石頭閒語。

* [php dbus extension with ByteArray support](http://rocksaying.tw/archives/21315764.html) 。
* [php dbus 0.1.0 撰寫 DBus service 的使用經驗](http://rocksaying.tw/archives/13799073.html)。
* [PHP D-Bus 與 Gearman 之比較](http://rocksaying.tw/archives/13478419.html)。
* [php dbus unboxing](http://rocksaying.tw/archives/16404035.html)。
