# CommonGateway

已將 CommonGateway 從大庫的子目錄搬出，移到獨立源碼庫 https://github.com/shirock/common-gateway-framework 。

Move to https://github.com/shirock/common-gateway-framework .

## 介紹

CommonGateway (index.php) 主要設計目的是用於設計 RESTful API 或是 Single page web app 。它按照 MVC 的設計模式，將 Web 應用服務分成三個部份，即資料模型(Model)、流程控制項(Controller)與視圖(View)。 CommonGateway 替設計人員處理控制項與視圖工作。至於資料模型則不是 CommonGateway 的責任。資料模型由設計人員透過其偏好的資料庫框架處理。

CommonGateway 為設計人員完成下列工作:

* 根據 URL 路徑(PATH_INFO) 選擇 Web 應用服務的控制項。正是這「依路徑選擇目標」的行為特徵，而且又是 Common Gateway Interface (CGI) 的實作項目，故我將此項目命名為 CommonGateway 。
* 它會將客戶端送出的文件資料，預先處理成關聯式陣列結構。除了傳統的 Query string 與 Form data ，它也能處理 HTTP PUT Method 會送出的資料。它也支援 JSON 型態的資料。
* 它會根據 RESTful 的原則，調用對應的控制項方法。
* 它會根據控制項方法的回傳結果與客戶端期望的文件回應型態，處理對應的視圖樣板。
* 它會儘量透過外部注入的方式，將其他資源放入控制項 (即 IoC 模式)，減少對原有程式碼的侵入性。

更多內容請參閱: [CommonGateway 介紹](http://rocksaying.tw/archives/21318202.html) 。
