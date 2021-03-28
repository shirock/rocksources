CommonGateway 資源注入
======================

CommonGateway 提供一種控制反轉(IoC)設計模式的資源注入方式。
它讓設計者可用 @resource 的特殊注釋 (annotation) ，從外部將特定資源指派給控制項的屬性，而不需要控制項自己找這些資源。

用法是在控制項類別屬性的注釋中，寫下特殊注釋 `@resource [資源名稱]` ，讓 CommonGateway 知道要把資源指派給這個控制項屬性。
若省略資源名稱，則從屬性名稱判斷資源項目。

目前有三項資源可以注入:

* config
* request
* request_document_type

用例:

~~~php
class Book {
    /**
      第一種寫法，省略資源名稱。

      @resource
      */
    var $config;

    /**
      第二種寫法，不省略資源名稱。

      @resource config
      */
    var $settings;

    /**
      CommonGateway 沒有名叫 xyz 的資源，忽略。

      @resource
      */
    var $xyz;
}
~~~

@resource config
----------------

將 CommonGateway 讀入的 app_config 內容，注入此控制項屬性。

用例: CommonGateway 讀到 etc/app_config.json ，且控制項屬性有此注釋。

~~~json
// etc/app_config.json:
{
    "siteName": "Book Store",
    "siteAddress": "rocksaying.tw"
}
~~~

~~~php
// book.php:
class Book {
    /**
      @resource
      */
    var $config;

    function index() 
    {
        echo $this->config->siteName;
    }
}
~~~

@resource request
-----------------

CommonGateway 會分析 HTTP 方法，自 Form 或 JSON 中取出客戶上傳資料，取名為 request 資源，並合併到全域變數 $_REQUEST 。
這就是 CommonGateway 的 request 資源。
換句話說，控制項或視圖內想要取得客戶上傳資料的話，一是透過 $_REQUUEST，二就是透過有這個注釋的屬性。

因為 $_REQUEST 的資料型態是 key/value 陣列，所以 request 資源也是 key/value 陣列。

註: PHP 並不會將 Form 或 JSON 上傳的資料內容放到 $_REQUEST 變數， CommonGateway 擴充了它。

用例: 用戶透過 POST 方法上傳 JSON 資料，且控制項屬性有此注釋。

~~~json
// POST book data.
{
    "title": "My Book",
    "ISBN": "123456"
}
~~~

~~~php
// book.php:
class Book {
    /**
      @resource request
      */
    var $book_form;

    function post() 
    {
        echo $this->book_form['title'], $this->book_form['ISBN'];
        // 或
        echo $_REQUEST['title'];
    }
}
~~~

@resource request_document_type
-------------------------------

CommonGateway 會分析客戶送來的 Accept 標頭，以第一個項目做為 view 的文件型態。
將分析出的文件型態，取名為  request_document_type。

request_document_type 之值，會是 'html', 'json', 'xml' 等文件型態的延伸名稱。
CommonGateway 會自己依據 request_document_type 載入對應的 view 。
若控制項需要自行處理回傳資料，便可依 request_document_type 判別應回傳的文件型態。
