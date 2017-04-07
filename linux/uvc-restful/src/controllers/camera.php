<?php
class Camera
{
    private $cid;
    private $socket;
    private $module;
    private $addr_prefix;

    /*
    建立符號連結指向 index.php 。以符號連結做為模組識別名稱。
    例如建立符號連結 uvc.php 指向 index.php ，
    則 RESTful URL 就是 uvc.php/camera 。
    */
    function __construct()
    {
        $name = $_SERVER['SCRIPT_NAME'];
        $this->module = substr($name, strrpos($name, '/')+1, -4);
        // 'path/xxx.php' -> 'xxx'

        switch ($this->module) {
        case 'uvc':
            $this->addr_prefix = 'unix:///tmp/uvc-service-socket-';
            break;
        default:
            HttpResponse::not_implemented('Unknown code name.');
        }
    }

    /**
    $camera_idx 可為任何以數字結尾的字串，例如 "video0", "camera21", "1" 等。
    程式只看結尾數字決定要操作哪一隻鏡頭。
     */
    function _open($camera_idx)
    {
        // 先看最後2字元是不是數字
        $idx = substr($camera_idx, -2);
        if (ctype_digit($idx)) {
            $this->cid = (int) $idx;
        }
        else { // 只取最後一字元
            $this->cid = (int) substr($camera_idx, -1);
        }

        $this->socket = fsockopen($this->addr_prefix . $this->cid, -1, $errno, $errstr, 5);
        if (!$this->socket) {
            HttpResponse::not_found('Could not open camera. ' . $errstr);
        }

        $s = fgets($this->socket);
        //echo $s;  // +OK
        return true;
    }

    function _close()
    {
        fclose($this->socket);
    }

    function _command($cmd)
    {
        fputs($this->socket, $cmd);
        $s = fgets($this->socket);
        if ($s != "+OK\n") {
            HttpResponse::bad_request('Command fail. ' . $s);
        }
        return HttpResponse::OK;
    }

    public function index()
    {
        HttpResponse::not_implemented();
    }

    public function hide($camera_idx)
    {
        $this->_open($camera_idx);
        return $this->_command("hide");
    }

    public function show($camera_idx, $x = 0, $y = 0)
    {
        $this->_open($camera_idx);
        return $this->_command("show $x $y");
    }

    public function resize($camera_idx, $width, $height)
    {
        $this->_open($camera_idx);
        return $this->_command("resize $width $height");
    }

    public function save($camera_idx, $img_filepath)
    {
        // 第二節以後的內容合併起來視為檔案路徑。
        $paths = func_get_args();
        array_shift($paths);
        $img_filepath = implode('/', $paths);

        if (strpos($img_filepath, './') !== false) {
            HttpResponse::bad_request('Invalid file path.');
        }

        $this->_open($camera_idx);
        // 輸入的 $img_filepath 缺了開頭的 '/' 字元，故補上。
        return $this->_command('save /' . $img_filepath);
    }

    public function capture($camera_idx, $img_fmt, $data_fmt = 'binary')
    {
        $is_base64 = false;
        $fmts = array('jpg', 'bmp', 'png', 'tiff');

        $img_fmt = strtolower($img_fmt);
        if ( !in_array($img_fmt, $fmts) ) {
            HttpResponse::bad_request('Invalid image format.');
        }

        $this->_open($camera_idx);

        $cmd = "save $img_fmt";
        if ($data_fmt == 'base64') {
            $cmd .= ' base64';
            $is_base64 = true;
        }

        fputs($this->socket, $cmd);

        $s = fgets($this->socket); // bytes of data
        //echo $s;
        if (strpos($s, '-ERR') !== false) {
            $this->_close();
            HttpResponse::bad_request('Failed to save image.');
        }

        $sz = (int)$s;

        // gather data
        $data = array();
        $i = 0;
        while ($sz > 0) {
            #echo $sz, "\n";
            $read_len = ($sz > 8192 ? 8192 : $sz);
            $data[$i] = fread($this->socket, $read_len);
            $sz -= strlen($data[$i]);
            ++$i;
        }
        $data = implode('', $data);

        //file_put_contents("test.jpg", $data);

        $s = fgets($this->socket); // newline char.
        $s = fgets($this->socket); // +OK

        if ($is_base64) {
            header("Content-type: application/octstream");
            header('Content-Disposition: filename=' . $img_fmt . '.base64');
        }
        else {
            if ($img_fmt == 'jpg')
                $mime_type = 'image/jpeg';
            else
                $mime_type = 'image/' . $img_fmt;

            header('Content-type: ' . $mime_type);
            header('Content-Disposition: filename=capture.' . $img_fmt);
        }

        echo $data;

        return false; // I render, don't call viewer.
    }
}
?>