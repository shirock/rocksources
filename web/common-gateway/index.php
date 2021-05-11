<?php
namespace {
// Mock data, TEST ONLY
if (PHP_SAPI == 'cli') {
    $_SERVER['PATH_INFO'] = "/book/info/123";
    $_SERVER['REQUEST_METHOD'] = 'POST';
    #$_SERVER['REQUEST_METHOD'] = 'GET';
    $_SERVER['HTTP_ACCEPT'] = 'application/json';
    $_SERVER['CONTENT_TYPE'] = 'application/x-www-form-urlencoded';
    $_POST = array('name' => 'rock');
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
        451 =>  'Unavailable For Legal Reasons',
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
    const UNAVAILABLE_FOR_LEGAL_REASONS = 451;
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

    // PSR-1 name style
    static function badRequest($msg=false)
    {
        self::bad_request($msg);
    }

    static function unauthorized($msg=false)
    {
        self::exception(HttpResponse::UNAUTHORIZED, $msg);
    }

    static function payment_required($msg=false)
    {
        self::exception(HttpResponse::PAYMENT_REQUIRED, $msg);
    }

    static function paymentRequired($msg=false)
    {
        self::payment_required($msg);
    }

    static function forbidden($msg=false)
    {
        self::exception(HttpResponse::FORBIDDEN, $msg);
    }

    static function not_found($msg=false)
    {
        self::exception(HttpResponse::NOT_FOUND, $msg);
    }

    static function notFound($msg=false)
    {
        self::not_found($msg);
    }

    static function method_not_allowed($msg=false)
    {
        self::exception(HttpResponse::METHOD_NOT_ALLOWED, $msg);
    }

    static function methodNotAllowed($msg=false)
    {
        self::method_not_allowed($msg);
    }

    static function not_acceptable($msg=false)
    {
        self::exception(HttpResponse::NOT_ACCEPTABLE, $msg);
    }

    static function notAcceptable($msg=false)
    {
        self::not_acceptable($msg);
    }

    static function request_timeout($msg=false)
    {
        self::exception(HttpResponse::REQUEST_TIMEOUT, $msg);
    }

    static function requestTimeout($msg=false)
    {
        self::request_timeout($msg);
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

    static function internalServerError($msg=false)
    {
        self::internal_server_error($msg);
    }

    static function not_implemented($msg=false)
    {
        self::exception(HttpResponse::NOT_IMPLEMENTED, $msg);
    }

    static function notImplemented($msg=false)
    {
        self::not_implemented($msg);
    }

    static function bad_gateway($msg=false) 
    {
        self::exception(HttpResponse::BAD_GATEWAY , $msg);
    }

    static function badGateway($msg=false) 
    {
        self::bad_gateway($msg);
    }

    static function service_unavailable($msg=false)
    {
        self::exception(HttpResponse::SERVICE_UNAVAILABLE, $msg);
    }

    static function serviceUnavailable($msg=false)
    {
        self::service_unavailable($msg);
    }
}

class CommonGateway
{
    const DEFAULT_HOMEPAGE = 'views/index.phtml';

    protected $app_name = null;
    protected $control = null;
    protected $segments = null;
    protected $action = null;
    protected $request_document_type = 'js'; //'html';
    protected $raw_request_data = null;

    function __get($k)
    {
        return isset($this->$k) ? $this->$k : null;
    }

    function __construct()
    {
        if ( !isset($_SERVER['PATH_INFO']) or $_SERVER['PATH_INFO'] == '/') {
            // 未指定 controller 時，嘗試找 Home controller 負責首頁。
            // 找不到 Home controller ，將會呼叫 index() 顯示預設首頁。
            if ($this->detectAppName('Home')) {
                $this->segments = array('Home');
            }
            else {
                return;
            }
        }
        else {
            $this->segments = explode('/', $_SERVER['PATH_INFO']);
            array_shift($this->segments); // first element always is an empty string.

            if ($this->segments[count($this->segments)-1] == '')
                array_pop($this->segments);
        }

        $app_config = $this->loadAppConfig();

        $control_seg = array_shift($this->segments);

        $this->loadControl($control_seg); // and set $this->app_name

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
                } 
                else { // PUT
                    $request_vars = json_decode($this->raw_request_data, true);
                    $_POST= &$request_vars;
                    $_REQUEST = array_merge($_GET, $request_vars);
                    # only contains $_GET and $_POST, due to security concerns.
                }
            } 
            elseif ($content_type == 'application/json') {
                $request_vars = json_decode($this->raw_request_data, true);
                //if ($_SERVER['REQUEST_METHOD'] == 'POST')
                    $_POST = &$request_vars; // 兼容傳統格式。
                $_REQUEST = array_merge($_GET, $request_vars);
                # only contains $_GET and $_POST, due to security concerns.
            } 
            else {
                $request_vars = &$_POST;
            }
            break;

        case 'DELETE':
        default:
            $request_vars = array(); // empty
            break;
        }

        $this->injectResource($this->control, 'config', $app_config);

        $this->injectResource($this->control, 'request', $request_vars);

        // I just look the first option.
        if (isset($_SERVER['HTTP_ACCEPT'])) {
            // the content of http header of Accept looks like:
            // Accept: text/plain; q=0.9, application/xml+html; q=0.1
            $http_accept = explode(',', $_SERVER['HTTP_ACCEPT']);

            // some clients will always insert '*/*' in the first option. skip it.
            if ($http_accept[0] == '*/*') { 
                array_shift($http_accept);
            }

            header('Content-Type: ' . $http_accept[0] .'; charset=utf-8');

            list($tmp, $http_accept_ext) = explode('/', $http_accept[0]);
            // $http_accept_ext would be the extension name of document type.
            // ex: '*', 'json', 'xml', etc.
            if ($http_accept_ext != '*')
                $this->request_document_type = $http_accept_ext;
        }

        $this->injectResource($this->control, 'request_document_type', $this->request_document_type);
    }

    /**
     * 取得基於 index.php 的 URL 路徑。
     * 不指定 $path 時，回傳 index.php 的 URL 。
     */
    public static function makeURL($path = false)
    {
        $root = $_SERVER['SCRIPT_NAME'];
        if (!$path) {
            return $root;
        }
        else if ($path[0] != '/') {
            $sep = '/';
        }
        else {
            $sep = '';
        }
        return $root . $sep . $path;
    }

    public static function makeFilepath($name)
    {
        return dirname(__FILE__) . '/' . $name;
    }

    protected function makeControlFilepath($name)
    {
        return "controllers/{$name}.php";
    }

    protected function loadAppConfig()
    {
        $config = false;
        $config_path = self::makeFilepath('etc/app_config.php');
        if (file_exists($config_path)) {
            @include_once $config_path;
        } 
        else {
            $config_path = self::makeFilepath('etc/app_config.json');
            if (file_exists($config_path)) {
                $config = json_decode(file_get_contents($config_path));
            }
        }
        return $config;
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
    protected function injectResource($target, $name, $resource)
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

    /**
     session 設置參數放在 etc/session_cookie.(json|php) 。
     參數內容看 session_set_cookie_params() 說明。
     若用 session_cookie.php ，則陣列名稱是 $options ，例如:
     $options = [
        'lifetime' => '3600',
        'samesite' => 'Strict'
      ]
     */
    protected function startSession()
    {
        $options = false;
        $params_path = self::makeFilepath('etc/session_cookie.json');
        if (file_exists($params_path)) {
            $options = json_decode(file_get_contents($params_path), true);
        } 
        else {
            $params_path = self::makeFilepath('etc/session_cookie.php');
            if (file_exists($params_path)) {
                @include_once $params_path;
            }
        }

        if ($options)
            session_set_cookie_params($options);

        session_start();
    }

    private $is_authorized = false;

    /**
     * 若控制項註記 @authorize ，則需經過認證。
     * 若控制項方法註記 @authorize ，則需經過認證。
     * 
     * 認證用控制項為 Authorize 或 Login 。
     * 授權時，必須設定 $_SESSION['Authorization'] 為任意值，有效值甚至包括 false 。
     * index.php 僅依 isset($_SESSION['Authorization']) 判定是否授權。
     */
    protected function authorize($target, $is_reflect = false)
    {
        // 若 class 已註記 @authorize ，就表示全部方法都要求授權。
        // 就不再判斷 method 是否註記 @authorize 。
        if ($this->is_authorized) {
            return;
        }

        $ro = ($is_reflect ? $target : new ReflectionObject($target));
        $doc = $ro->getDocComment();
        if (preg_match("/@authorize\s/", $doc, $m) > 0) {
            $this->is_authorized = true;
            $this->startSession();
            if (isset($_SESSION['Authorization'])) {
                return;
            }

            // 認證用控制項為 Authorize 或 Login
            if (file_exists($this->makeControlFilepath('Login')))
                $authorize_control = 'Login';
            else 
                $authorize_control = 'Authorize';

            $authorize_path = self::makeURL($authorize_control);
            // echo 'redirect to ', $authorize_path;
            header("Location: $authorize_path");
            exit;
        }
        return;
    }

    protected function detectAppName($name)
    {
        if (!ctype_alnum(str_replace('_', '', $name))) { // invalid name.
            return false;
        }

        $name = strtolower($name); // 消除大小寫差異。

        // 若這是包含 _ 的名稱，先拆字。
        $ws = explode('_', $name);
        for ($i = 0; $i < count($ws); ++$i) {
            $ws[$i] = ucfirst($ws[$i]);
        }

        for ($case_f = 0; $case_f < 5; ++$case_f) {
            switch ($case_f) {
            case 0:
                // case: search directly by name.
                $file_name = $name;
                break;
            case 1:
                // case: name is 'abc', search for 'Abc.php';
                $file_name = ucfirst($name);
                break;
            case 2:
                // case: name is 'abc_def', search 'Abc_Def.php'.
                $file_name = implode('_', $ws);
                break;
            case 3:
                // case: name is 'abc_def', search 'AbcDef.php'.
                $file_name = implode('', $ws);
                break;
            default:
                break;
            }

            $component_filepath = $this->makeControlFilepath($file_name);
            if ( file_exists($component_filepath) ) {
                return ucfirst($file_name);
            }
        }
        return false;
    }

    protected function loadControl($name)
    {
        $this->app_name = $this->detectAppName($name);
        if ($this->app_name == false) {
            HttpResponse::not_found();
        }

        $control_filepath = $this->makeControlFilepath($this->app_name);

        require_once $control_filepath;

        $control_class_name = $this->app_name;
        $this->control = new $control_class_name;
    }

    function index()
    {
        if (file_exists(self::DEFAULT_HOMEPAGE)) {
            include_once self::DEFAULT_HOMEPAGE;
        } 
        else {
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

        if ($this->app_name == 'Login' or $this->app_name == 'Authorize') {
            // 目標是負責認證工作的控制項，需要開始 session 
            $this->startSession();
        }
        else {
            $this->authorize($this->control);
        }

        if ( empty($this->segments) and $_SERVER['REQUEST_METHOD'] == 'GET') { // Without parameter
            $method = 'index';
        } 
        else {
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
        } 
        else {
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
        if (!$ref_method->isPublic()) {
            // I only invoke public method of control.
            // ps. In PHP, method will be defined as public if there is no declaration.
            HttpResponse::bad_request();
        }

        $this->authorize($ref_method, true);

        $method_parameters = $ref_method->getParameters();
        // 若定義了第一個參數為array，則PATH參數陣列將會直接傳入。
        // 其他情形則一律展開參數後傳入，此時函數內部可透過參數列名稱或 func_get_arg() 取得參數內容。
        $first_parameter_is_array = false;
        if (isset($method_parameters[0])) {
            $p0 = $method_parameters[0]->getType(); // 未明定型態，則為 null
            if ($p0 and $p0->getName() == 'array')
                $first_parameter_is_array = true;
        }

        // if (isset($method_parameters[0]) and $method_parameters[0]->isArray()) {
        if ($first_parameter_is_array) {
            $model = $this->control->$method($arguments);
        } 
        else {
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
        // 分配 $model 一個和控制項名稱相同但首字母小寫的別名(reference)
        $model_alias = lcfirst($this->app_name);
        if ((is_object($model) or is_array($model)) and 
            !isset($$model_alias))
        {
            $$model_alias = &$model;
        }

        // 根據服務名稱與 HTTP 標頭的 Accept 內容，載入對應的視圖。
        // 視圖的副檔名按 Ruby on Rails 型式，開頭為 p ，後接文件型態名稱。
        // 例如 HTML 文件的視圖，副檔名為 phtml 。
        // 比較特別的是 JSON 文件的視圖，其副檔名為 pjs ，不是 pjson 。
        if ($this->request_document_type == 'json' or
            $this->request_document_type == 'javascript')
        {
            $_ext_name = 'js';
        } 
        else 
        {
            $_ext_name = $this->request_document_type;
        }

        // RoR style's view name.
        $_view_filepath = "views/{$this->app_name}/{$this->action}.p{$_ext_name}";

        if (file_exists($_view_filepath)) {
            include_once $_view_filepath;
        } 
        elseif ($_ext_name == 'js') {
            // 如果未指定配對的view，但要求傳回的文件型態是json，就自動傳回整份 $model 內容。
            // 在我的使用經驗上，九成傳回json的view，內容都只有一行 json_encode($model) 。
            // 故將此規則寫入 Common Gateway ，減少空泛的 json view 。
            echo json_encode($model);
        } 
        else {
            echo "Template is missing. Missing $_view_filepath.";
        }
    }

    function loadViewHelper()
    {
        @include_once "helpers/_global.php"; // for all application.

        $helper_filepath = "helpers/{$this->app_name}.php";
        @include_once $helper_filepath;
    }

} //end class CommonGateway

$gw = new CommonGateway();

/*
執行控制項(controller)的回傳值(即 $gw->run() 的回傳值)，決定視圖(view)的處理工作。
CommonGateway 會根據控制項回傳資料的型態，決定傳給視圖的資料來源(model)內容為何。
回傳值規則看下方 render() 說明。
但是以下種兩回傳值不會載入視圖:
1. false 。
1. 回傳值為介於100 ~ 599間的整數，視為控制項直接回傳 HTTP 狀態碼。
CommonGateway 會將該狀態碼回傳給瀏覽器。
*/
$model = $gw->run();

if ($model === false) {
    exit;
}

if (is_int($model) and $model >= 100 and $model <= 599) {
    HttpResponse::status($model);
}

$gw->loadViewHelper();

/*
將 $model (控制項資料內容) 傳給 render() 作為「視圖活動範圍內可用的資料內容」。

1. 若 $model 為 null (或無回傳值): 大部份控制項的處理函數不回傳內容，故這是預設行為。
此時會將控制項的公開屬性當作資料來源(model)，將控制項的公開屬性內容展開成
視圖活動範圍內的區域變數。
例如控制項有公開屬性 title ，CommonGateway 會將此屬性指派為視圖的區域變數 $title 。

2. 若 $model 為 true : 同回傳 null 的情形。

3. 若 $model 為 false : 視同控制項自行處理回應工作， CommonGateway 不會繼續載入視圖。

4. 若 $model 為介於100 ~ 599間的整數，視為控制項直接回傳 HTTP 狀態碼。
CommonGateway 會將該狀態碼回傳給瀏覽器，而不載入任何視圖。

5. 若 $model 為 array : CommonGateway 會將回傳的陣列視為資料來源，
指派為視圖內的區域變數 $model，並將陣列內容展開成為視圖內的區域變數。
注意，若陣列為數字索引陣列，則展開後的區域變數名稱之字首為 data_ 。
例如 $model = array('a', 'b') ，則視圖內展開的區域變數內容將是
$data_0 == 'a', $data_1 == 'b' ，餘類推。

6. 若 $model 為 object : CommonGateway 會將回傳的個體視為資料來源，
指派為視圖內的區域變數 $model。
此時在視圖內將可以調用該個體的方法。這可以取代 helper 。

7. 若 $model 型態為 array 或 object ，則視圖內將同時分配一個和控制項名稱相同的別名。

例如控制項 MyBook 回傳的資料為 object ，包含一個資料欄位 Title 。
則在視圖內，可以用下列途徑取得 Title 內容:
1. $model->Title;
2. $MyBook->Title;
*/
$gw->render($model);

} // end global namespace

namespace cg {
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
    public static function loadUploadedFiles($fields)
    {
        $files = array();
        foreach ($fields as $field) {
            if (isset($_FILES[$field]) and !empty($_FILES[$field])) {
                if (is_array($_FILES[$field]['tmp_name'])) {
                    foreach ($_FILES[$field]['tmp_name'] as $filepath) {
                        if (!empty($filepath))
                            $files[$field][] = file_get_contents($filepath);
                    }
                } 
                else {
                    if (!empty($_FILES[$field]['tmp_name']))
                        $files[$field] = file_get_contents($_FILES[$field]['tmp_name']);
                }
            } 
            elseif (isset($_POST[$field]) and !empty($_POST[$field])) {
                if (is_array($_POST[$field])) {
                    foreach ($_POST[$field] as $encoded_text) {
                        $files[$field][] = base64_decode($encoded_text);
                    }
                } 
                else {
                    $files[$field] = base64_decode($_POST[$field]);
                }
            }
        }
        return $files;
    }
}
} // end namespace cg

namespace cg\html {
    // base on index.php/$controller_path
    function request_url($controller_path = false)
    {
        $root = '//' . $_SERVER['HTTP_HOST'] . $_SERVER['SCRIPT_NAME'];
        if ($controller_path) {
            $root .= '/' . $controller_path;
        }
        return $root;
    }

    // redirect to $fullpath/index.php or $fullpath/index.php/controller_path
    function redirect($controller_path = false)
    {
        header('Location: ' . request_url($controller_path));
    }

    function resource_url($path = false)
    {
        $root = dirname($_SERVER['SCRIPT_NAME']);
        if (!$path) {
            return $root;
        }
        return $root . '/' . $path;
    }

    function stylesheet($srcs)
    {
        if (is_array($srcs)) {
            foreach ($srcs as $src) {
                echo '<link rel="stylesheet" href="', resource_url($src), '">', "\n";
            }
        }
        else {
            echo '<link rel="stylesheet" href="', resource_url($srcs), '">', "\n";
        }
    }

    function script($srcs)
    {
        if (is_array($srcs)) {
            foreach ($srcs as $src) {
                echo '<script src="', resource_url($src), '"></script>', "\n";
            }
        }
        else {
            echo '<script src="', resource_url($srcs), '"></script>', "\n";
        }
    }

    function refresh($seconds)
    {
        echo '<meta http-equiv="refresh" content="', $seconds, '">', "\n";
    }
} // end namespace cg\html
?>