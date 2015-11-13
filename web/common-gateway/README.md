# CommonGateway

## 介紹

CommonGateway (index.php) 的主要設計目的是用於設計 RESTful API 或是 Single page web app ，而不是完整的 Web Site 。它是一個簡化的 MVC 容器。按照 MVC 的設計模式，它將一個 Web 應用服務分成三個部份，即資料模型(Model)、流程控制項(Controller)與視圖(View)。 CommonGateway 主要替設計人員處理控制項與視圖工作。至於資料模型則不是 CommonGateway 的責任。資料模型由設計人員透過 schema-database 或其他偏好的資料庫函數處理。

CommonGateway 為程序人員完成下列工作:

* 根據 URL 路徑(PATH_INFO) 選擇 Web 應用服務的控制項。正是這「依路徑選擇目標」的行為特徵，而且又是 Common Gateway Interface (CGI) 的實作項目，故我將此項目命名為 CommonGateway 。
* 它會將客戶端送出的文件資料，預先處理成關聯式陣列結構。除了傳統的 Query string 與 Form data ，它也能處理 HTTP PUT Method 會送出的資料。它也支援 JSON 型態的資料。
* 它會根據 RESTful 的原則，調用對應的控制項方法。
* 它會根據控制項方法的回傳結果與客戶端期望的文件回應型態，處理對應的視圖樣板。
* 它會儘量透過外部注入的方式，將其他資源放入控制項 (即 IoC 模式)，減少對原有程式碼的侵入性。

更多內容請參閱: [CommonGateway 介紹](http://rocksaying.tw/archives/2013/CommonGateway%20%E4%BB%8B%E7%B4%B9.html) 。

## 設計意見

### PSR-0 autoload:

I think that autoload should not be my job. CommonGateway is the front before your framework. So your framework should define autoload function.

### PSR-1 coding standard:

Considering namespace.
