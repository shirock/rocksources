<?php
/**
resource 註記:

 用法: @resource [資源名稱]
 若省略資源名稱，則視其同屬性名稱。

@resource request_document_type:

CommonGateway 會分析客戶送來的 Accept 標頭，以第一個項目做為 view 的文件型態。
並將分析出的文件型態，取名為  request_document_type。
此資源可用註記 @resource request_document_type 注入控制項指定的屬性。

request_document_type 之值，是 'html', 'json', 'xml' 等文件型態的延伸名稱。
CommonGateway 會自己依據 request_document_type 載入對應的 view 。
若控制項需要自行處理回傳資料，可參考屬性 request_document_type 判別回傳的文件
型態。

@resource request:

CommonGateway 會分析 HTTP 方法，自適當的資料來源中取出表單資料，取名為 request 資源。
此資源可用註記 @resource request 注入控制項指定的屬性。

傳統上，我們可以從 $_GET, $_POST 中取得表單資料。
但是 PUT 方法送來的表單，並沒有對應的全域變數。此時就需要透過 request 取得。

 */

// Mock data, TEST ONLY
if (PHP_SAPI == 'cli') {
    $_SERVER['PATH_INFO'] = "/book/info/123";
    $_SERVER['REQUEST_METHOD'] = 'POST';
    #$_SERVER['REQUEST_METHOD'] = 'GET';
    $_SERVER['HTTP_ACCEPT'] = 'application/json';
    $_SERVER['CONTENT_TYPE'] = 'application/x-www-form-urlencoded';
    $_POST = array('name' => 'rock');
}

class Controller 
{
    public function index() 
    {
        echo 'index...';
    }

    /**
     Laod uploaded files form $_FILES or $_POST (JSON only).
     1. 此方法不會保留上傳檔案的原本名稱。忽略 $_FILES 的 'name' 欄位。
     2. 使用 JSON 文件上傳檔案時，CommonGateway 會自動將 JSON 文件解碼再指派
        給 $_POST 變數。故此函數也會從 $_POST 變數中載入內容。
     3. 使用 JSON 文件上傳時，限定以 BASE64 處理要上傳的檔案內容。這是因為JSON
        規定採用 UTF-8 編碼。但是二進位文件內容若使用 UTF-8 編碼，則編碼後的內容
        長度會比 BASE64 長上許多。故此處限定預先以 BASE64 二進位文件內容。
     */    
    public static function load_uploaded_files($fields) 
    {
        $files = array();
        foreach ($fields as $field) {
            if (isset($_FILES[$field]) and !empty($_FILES[$field])) {
                if (is_array($_FILES[$field]['tmp_name'])) {
                    foreach ($_FILES[$field]['tmp_name'] as $filepath) {
                        if (!empty($filepath))
                            $files[$field][] = file_get_contents($filepath);
                    }
                } else {
                    if (!empty($_FILES[$field]['tmp_name']))
                        $files[$field] = file_get_contents($_FILES[$field]['tmp_name']);
                }
            } elseif (isset($_POST[$field]) and !empty($_POST[$field])) {
                if (is_array($_POST[$field])) {
                    foreach ($_POST[$field] as $encoded_text) {
                        $files[$field][] = base64_decode($encoded_text);
                    }
                } else {
                    $files[$field] = base64_decode($_POST[$field]);
                }
            }
        }
        return $files;
    }
}

class HttpResponse 
{
    // See http://en.wikipedia.org/wiki/List_of_HTTP_status_codes
    static $status = array(
        100 =>  'Continue',
        101 =>  'Switching Protocols',
        102 =>  'Processing',
        103 =>  'Checkpoint',
        200 =>  'Ok',
        201 =>  'Created',
        202 =>  'Accepted',
        203 =>  'Non-Authoritative Information',
        204 =>  'No Content',
        205 =>  'Reset Content',
        206 =>  'Partial Content',
        300 =>  'Multiple Choices',
        301 =>  'Moved Permanently',
        302 =>  'Found',
        303 =>  'See Other',
        304 =>  'Not Modified',
        305 =>  'Use Proxy',
        306 =>  'Switch Proxy',
        307 =>  'Temporary Redirect',
        308 =>  'Resume Incomplete',
        400 =>  'Bad Request',
        401 =>  'Unauthorized',
        402 =>  'Payment Required',
        403 =>  'Forbidden',
        404 =>  'Not Found',
        405 =>  'Method not Allowed',
        406 =>  'Not Acceptable',
        407 =>  'Proxy Authentication Required',
        408 =>  'Request Timeout',
        409 =>  'Conflict',
        410 =>  'Gone',
        411 =>  'Length Required',
        412 =>  'Precondition Failed',
        413 =>  'Request Entity Too Large',
        414 =>  'Request-URI Too Long',
        415 =>  'Unsupported Media Type',
        416 =>  'Requested Range Not Satisfiable',
        417 =>  'Expectation Failed',
        422 =>  'Unprocessable Entity',
        423 =>  'Locked',
        424 =>  'Failed Dependency',
        425 =>  'Unordered Collection',
        426 =>  'Upgrade Required',
        500 =>  'Internal Server Error',
        501 =>  'Not Implemented',
        502 =>  'Bad Gateway',
        503 =>  'Service Unavailable',
        504 =>  'Gateway Timeout',
        505 =>  'HTTP Version Not Supported',
        506 =>  'Variant Also Negotiates',
        507 =>  'Insufficient Storage',
        509 =>  'Bandwidth Limit Exceeded'
    );

    /*
    //由此段程式產生下列常數定義的程式碼。
    foreach (HttpResponse::$status as $c => $w) {
        $ws = preg_split('/[\s\-]/', $w);
        $w = strtoupper(implode('_', $ws));
        echo "    const $w = $c;\n";
        //define($w, $c);
    }
    */
    //const CONTINUE = 100; // 'continue' is a keyword, could not used in constant.
    const SWITCHING_PROTOCOLS = 101;
    const PROCESSING = 102;
    const CHECKPOINT = 103;
    const OK = 200;
    const CREATED = 201;
    const ACCEPTED = 202;
    const NON_AUTHORITATIVE_INFORMATION = 203;
    const NO_CONTENT = 204;
    const RESET_CONTENT = 205;
    const PARTIAL_CONTENT = 206;
    const MULTIPLE_CHOICES = 300;
    const MOVED_PERMANENTLY = 301;
    const FOUND = 302;
    const SEE_OTHER = 303;
    const NOT_MODIFIED = 304;
    const USE_PROXY = 305;
    const SWITCH_PROXY = 306;
    const TEMPORARY_REDIRECT = 307;
    const RESUME_INCOMPLETE = 308;
    const BAD_REQUEST = 400;
    const UNAUTHORIZED = 401;
    const PAYMENT_REQUIRED = 402;
    const FORBIDDEN = 403;
    const NOT_FOUND = 404;
    const METHOD_NOT_ALLOWED = 405;
    const NOT_ACCEPTABLE = 406;
    const PROXY_AUTHENTICATION_REQUIRED = 407;
    const REQUEST_TIMEOUT = 408;
    const CONFLICT = 409;
    const GONE = 410;
    const LENGTH_REQUIRED = 411;
    const PRECONDITION_FAILED = 412;
    const REQUEST_ENTITY_TOO_LARGE = 413;
    const REQUEST_URI_TOO_LONG = 414;
    const UNSUPPORTED_MEDIA_TYPE = 415;
    const REQUESTED_RANGE_NOT_SATISFIABLE = 416;
    const EXPECTATION_FAILED = 417;
    const UNPROCESSABLE_ENTITY = 422;
    const LOCKED = 423;
    const FAILED_DEPENDENCY = 424;
    const UNORDERED_COLLECTION = 425;
    const UPGRADE_REQUIRED = 426;
    const INTERNAL_SERVER_ERROR = 500;
    const NOT_IMPLEMENTED = 501;
    const BAD_GATEWAY = 502;
    const SERVICE_UNAVAILABLE = 503;
    const GATEWAY_TIMEOUT = 504;
    const HTTP_VERSION_NOT_SUPPORTED = 505;
    const VARIANT_ALSO_NEGOTIATES = 506;
    const INSUFFICIENT_STORAGE = 507;
    const BANDWIDTH_LIMIT_EXCEEDED = 509;

    static function status($statusCode, $message = false, $exit_program = true) 
    {
        if ($message == false) {
            if (isset(self::$status[$statusCode]))
                $message = self::$status[$statusCode];
            else
                $message = $statusCode;
        }
        header("HTTP/1.0 {$statusCode} {$message}");

        if ($exit_program) {
            echo "{$statusCode} {$message}";
            exit;
        }
    }

    static function exception($statusCode, $message = false) 
    {
        self::status($statusCode, $message, true);
    }

    /**
    以下回應方法都是回報錯誤狀態。呼叫後就會結束程式。
     */
    static function bad_request($msg=false) 
    {
        self::exception(HttpResponse::BAD_REQUEST, $msg);
    }

    static function unauthorized($msg=false) 
    {
        self::exception(HttpResponse::UNAUTHORIZED, $msg);
    }

    static function payment_required($msg=false) 
    {
        self::exception(HttpResponse::PAYMENT_REQUIRED, $msg);
    }

    static function forbidden($msg=false) 
    {
        self::exception(HttpResponse::FORBIDDEN, $msg);
    }

    static function not_found($msg=false) 
    {
        self::exception(HttpResponse::NOT_FOUND, $msg);
    }

    static function method_not_allowed($msg=false) 
    {
        self::exception(HttpResponse::METHOD_NOT_ALLOWED, $msg);
    }

    static function not_acceptable($msg=false) 
    {
        self::exception(HttpResponse::NOT_ACCEPTABLE, $msg);
    }

    static function request_timeout($msg=false) 
    {
        self::exception(HttpResponse::REQUEST_TIMEOUT, $msg);
    }

    static function conflict($msg=false) 
    {
        self::exception(HttpResponse::CONFLICT, $msg);
    }

    static function gone($msg=false) 
    {
        self::exception(HttpResponse::GONE, $msg);
    }

    static function internal_server_error($msg=false) 
    {
        self::exception(HttpResponse::INTERNAL_SERVER_ERROR, $msg);
    }

    static function not_implemented($msg=false) 
    {
        self::exception(HttpResponse::NOT_IMPLEMENTED, $msg);
    }

    static function bad_gateway($msg=false) {
        self::exception(HttpResponse::BAD_GATEWAY , $msg);
    }

    static function service_unavailable($msg=false) 
    {
        self::exception(HttpResponse::SERVICE_UNAVAILABLE, $msg);
    }
}

function url_root_path() 
{
    return dirname($_SERVER['SCRIPT_NAME']);
}

class CommonGateway 
{
    const DEFAULT_HOMEPAGE = 'views/index.phtml';

    protected $app_name = null;
    protected $control = null;
    protected $segments = null;
    protected $action = null;
    protected $request_document_type = 'html';
    protected $raw_request_data = null;
    
    function __get($k) 
    {
        return isset($this->$k) ? $this->$k : null;
    }

    function __construct() 
    {
        if ( !isset($_SERVER['PATH_INFO']) or $_SERVER['PATH_INFO'] == '/') {
            // $this->control = $this->segments = false;
            return;
        }

        $this->segments = explode('/', $_SERVER['PATH_INFO']);
        array_shift($this->segments); // first element always is an empty string.
        
        if ($this->segments[count($this->segments)-1] == '')
            array_pop($this->segments);

        $control_seg = array_shift($this->segments);

        $this->load_control($control_seg); // and set $this->app_name

        switch ($_SERVER['REQUEST_METHOD']) {
        case 'GET':
            $request_vars = $_GET;
            break;

        case 'POST':
        case 'PUT':
            if (isset($_SERVER['CONTENT_TYPE']))
                list($content_type) = explode(';', $_SERVER['CONTENT_TYPE']);
            else
                $content_type = '';

            # See issue #7
            # and http://php.net/manual/en/wrappers.php.php
            $this->raw_request_data = file_get_contents('php://input');

            # See issue #7
            # and http://php.net/manual/en/ini.core.php#ini.always-populate-raw-post-data
            $GLOBALS['HTTP_RAW_POST_DATA'] = $this->raw_request_data;

            # Form content types
            # See http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4
            if ($content_type == 'application/x-www-form-urlencoded') {
                if ($_SERVER['REQUEST_METHOD'] == 'POST') {
                    $request_vars = &$_POST;
                } else { // PUT
                    $request_vars = json_decode($this->raw_request_data, true);
                    $_POST= &$request_vars;
                    $_REQUEST = array_merge($_GET, $request_vars); 
                    # only contains $_GET and $_POST, due to security concerns.
                }
            } elseif ($content_type == 'application/json') {
                $request_vars = json_decode($this->raw_request_data, true);
                //if ($_SERVER['REQUEST_METHOD'] == 'POST')
                    $_POST = &$request_vars; // 兼容傳統格式。
                $_REQUEST = array_merge($_GET, $request_vars); 
                # only contains $_GET and $_POST, due to security concerns.
            } else {
                $request_vars = &$_POST;
            }
            break;

        case 'DELETE':
        default:
            $request_vars = array(); // empty
            break;
        }

        $app_config = $this->_load_app_config();
        $this->inject_resource($this->control, 'config', $app_config);

        $this->inject_resource($this->control, 'request', $request_vars);

        // I just look the first option.
        if (isset($_SERVER['HTTP_ACCEPT'])) {
            // the content of http header of Accept looks like:
            // Accept: text/plain; q=0.9, application/xml+html; q=0.1
            $http_accept = explode(',', $_SERVER['HTTP_ACCEPT']);
            
            if ($http_accept[0] != '*/*') {
                #$this->response_content_type = $http_accept[0];
                header('Content-Type: ' . $http_accept[0] .'; charset=utf-8');
            }

            list($tmp, $http_accept_ext) = explode('/', $http_accept[0]);
            // $http_accept_ext would be the extension name of document type. 
            // ex: '*', 'json', 'xml', etc.
            if ($http_accept_ext != '*')
                $this->request_document_type = $http_accept_ext;
        }

        $this->inject_resource($this->control, 'request_document_type', $this->request_document_type);
    }

    /**
    為指定個體注入指定的資源。
    
    使用註記 @resource [name] 。
    name 表示資源名稱，可省略。若註記省略 name 參數時，則以屬性名稱為配對的資源名稱。
    
    ps. PHP 不支援註記語法，此處的註記內容實際上應寫在屬性的 Doc 區。
    
    case 1:
      @resource request
      var $form;
      // 將名稱為 request 的資源注入 $form 屬性。
      
    case 2:
      @resource
      var $request
      // 將名稱為 request 的資源注入 $request 屬性。
     */
    public function inject_resource($target, $name, $resource)
    {
        $ro = new ReflectionObject($target);
        foreach ($ro->getProperties() as $prop) {
            $prop_name = $prop->name;
            $doc = $prop->getDocComment();
            if (preg_match("/@resource\s([\w\d_]+)?/", $doc, $m) > 0) {
                #print_r($m);
                $res_name = isset($m[1]) ? $m[1] : $prop_name;
                if ($res_name == $name) {
                    $prop->setAccessible(true); // only effect $prop, ignore the access modifier.
                    $prop->setValue($target, $resource); 
                }
            }
        }
        return;
    }

    protected function _load_app_config()
    {
        $config = false;
        $config_path = dirname(__FILE__) . '/etc/app_config.php';
        if (file_exists($config_path)) {
            @include_once $config_path;
        } else {
            $config_path = dirname(__FILE__) . '/etc/app_config.json';
            if (file_exists($config_path)) {
                $config = json_decode(file_get_contents($config_path));
            }
        }
        return $config;
    }

    protected function _detect_app_name($path, $name) 
    {
        if (!ctype_alnum(str_replace('_', '', $name))) { // invalid name.
            return false;
        }

        for ($case_f = 0; $case_f < 5; ++$case_f) {        
            switch ($case_f) {
            case 0: 
                // case: search directly by name.
                $app_name = $name;
                break;
            case 1:
                // case: name is 'abc', search for 'Abc.php';
                $app_name = ucfirst($name);
                break;
            case 2:
                // case: name is 'Abc', search for 'abc.php';
                $app_name = strtolower($name[0]) . substr($name, 1);
                break;
            case 3:
                // case: name is 'abc_def'
                $ws = explode('_', $name);
                for ($i = 0; $i < count($ws); ++$i) {
                    $ws[$i] = ucfirst($ws[$i]);
                }

                // case: name is 'abc_def', search 'Abc_Def.php'.
                $name = implode('_', $ws);
                break;
            case 4:
                // case: name is 'abc_def', search 'AbcDef.php'.
                $app_name = implode('', $ws);
                break;
            default:
                break;
            }

            $component_filepath = $this->_make_control_filepath($app_name); 
            if ( file_exists($component_filepath) ) { 
                return $app_name;
            }
        }        
        return false;
    }

    protected function _make_control_filepath($name)
    {
        return "controllers/{$name}.php";
    }

    protected function load_control($name) 
    {
        #$this->_load_component('controllers/', $name);
        $this->app_name = $this->_detect_app_name('controllers/', $name);
        if ($this->app_name == false) {
            HttpResponse::not_found();
        }

        $control_filepath = $this->_make_control_filepath($this->app_name); 
        
        require_once $control_filepath;

        $control_class_name = $this->app_name;
        $this->control = new $control_class_name;
    }

    function index() 
    {
        if (file_exists(self::DEFAULT_HOMEPAGE)) {
            include_once self::DEFAULT_HOMEPAGE;
        } else {
            echo '<p>index.php/{control_name}/{object_id}.</p>';
            echo '<p>You may put your controller class in controllers/{class_name}.php.</p>';
        }
        return false;
    }

    function run() 
    {
        if ( $this->control === null) {
            return $this->index();
        }

        if ( empty($this->segments) and $_SERVER['REQUEST_METHOD'] == 'GET') { // Without parameter
            $method = 'index';
        } else {
            $method = strtolower($_SERVER['REQUEST_METHOD']);
        }

        $method_exists = false;

        // first, try RESTful way. according to $_SERVER[REQUEST_METHOD]
        if ( !method_exists($this->control, $method)) {
            // second, try traditional (RPC) style. according to segment.
            if ( !empty($this->segments) ) {
                $method = array_shift($this->segments);
                if (method_exists($this->control, $method)) {
                    $method_exists = true;
                }
            }
        } else {
            $method_exists = true;
        }

        if ( !$method_exists ) {
            $tmp = strtolower($_SERVER['REQUEST_METHOD']) . ' or ' . $method;
            HttpResponse::not_implemented("This service does not implement $tmp method.");
        }

        if (preg_match('/^[a-zA-Z][a-zA-Z0-9_]+$/', $method) < 1) {
            HttpResponse::bad_request();
        }

        $this->action = $method;

        $arguments = $this->segments;

        $ref_method = new ReflectionMethod(get_class($this->control), $method);
        $method_parameters = $ref_method->getParameters();
        // 若定義了第一個參數為array，則PATH參數陣列將會直接傳入。
        // 其他情形則一律展開參數後傳入，此時函數內部可透過參數列名稱或 func_get_arg() 取得參數內容。
        if (isset($method_parameters[0]) and $method_parameters[0]->isArray()) {
            $model = $this->control->$method($arguments);
        } else {
            if (count($arguments) < $ref_method->getNumberOfRequiredParameters())
                HttpResponse::bad_request();
            $model = call_user_func_array(array($this->control, $method), $arguments);
        }
        return $model;
    }

    function render($model = null)
    {
        // 以控制項的公開屬性為資料來源
        if ($model === null or $model === true)
            $model = get_object_vars($this->control);
        // 若型態為陣列，則展開內容為區域變數.
        if (is_array($model))
            extract($model, EXTR_PREFIX_INVALID, 'data');
        #elseif (is_object($model) and !isset(${$this->app_name}))
        if ((is_object($model) or is_array($model)) and !isset(${$this->app_name}))
            ${$this->app_name} = &$model; // 指派此資料模型給控制項同名的變數
        if ($this->request_document_type == 'json' or 
            $this->request_document_type == 'javascript'
        ) {
            $_ext_name = 'js';
        } else {
            $_ext_name = $this->request_document_type;
        }

        // RoR style's view name.
        $_view_filepath = "views/{$this->app_name}/{$this->action}.p{$_ext_name}";

        if (file_exists($_view_filepath)) {
            include_once $_view_filepath;
        } elseif ($_ext_name == 'js') {
            // 如果未指定配對的view，但要求傳回的文件型態是json，就自動傳回整份 $model 內容。
            // 在我的使用經驗上，九成傳回json的view，內容都只有一行 json_encode($model) 。
            // 故將此規則寫入 Common Gateway ，減少空泛的 json view 。
            echo json_encode($model);
        } else {
            echo "Template is missing. Missing $_view_filepath.";
        }
    }

    function load_view_helper() 
    {
        @include_once "helpers/_global.php"; // for all application.
        
        $helper_filepath = "helpers/{$this->app_name}.php";
        @include_once $helper_filepath;
    }

} //end class CommonGateway

$gw = new CommonGateway();

/*
執行控制項(controller)的回傳值(即 $gw->run() 的回傳值)，決定視圖(view)的處理工作。
回傳值規則如後述。

1. false : 視同控制項自行處理回應工作， CommonGateway 不會繼續載入視圖。

遇到 false 以外的回傳值，CommonGateway 都會載入視圖。
CommonGateway 會自動根據服務名稱與 HTTP 標頭的 Accept 內容，載入對應的視圖。
視圖的副檔名按 Ruby on Rails 型式，開頭為 p ，後接文件型態名稱。
例如 HTML 文件的視圖，副檔名為 phtml 。
比較特別的是 JSON 文件的視圖，其副檔名為 pjs ，不是 pjson 。

CommonGateway 會根據控制項回傳資料的型態，決定傳給視圖的資料來源(model)內容為何。

null (或無回傳值) : 大部份控制項的處理函數不會回傳內容，故這是預設行為。
此時會將控制項的公開屬性當作資料來源(model)，將控制項的公開屬性內容展開成視圖活
動範圍內的區域變數。
例如控制項有公開屬性 title ，CommonGateway 會將此屬性指派為視圖的區域變數 $title 。

true : 同回傳 null 的情形。

整數 : 介於100 ~ 599間的整數，視為控制項直接回傳 HTTP 狀態碼。
CommonGateway 會將該狀態碼回傳給瀏覽器，而不載入任何視圖。

array : CommonGateway 會將回傳的陣列視為資料集合，將陣列內容展開成為視圖內的區域
變數。
注意，若陣列會數字索引陣列，則會展開後的區域變數名稱之字首為 data_ 。例如第0個陣列
元素展開後的區域變數名稱將會是 data_0 ，餘類推。

object : CommonGateway 會將回傳的個體視為資料來源，直接指派為視圖內的區域變數 $model。
此時在視圖內將可以調用該個體的方法。這可以取代 helper 。
*/
$model = $gw->run();

if ($model === false) {
    exit;
}

if (is_int($model) and $model >= 100 and $model <= 599) {
    HttpResponse::status($model);
}

$gw->load_view_helper();

$gw->render($model);

?>
