<?php
class Phpinfo {
    function _info() {
        phpinfo();
        return false;
    }
    function index() {
        return $this->_info();
    }
    function get() {
        return $this->_info();
    }
    function put() {
        return $this->_info();
    }
    function post() {
        return $this->_info();
    }
    function delete() {
        return $this->_info();
    }
}
?>
