<?php
/**
 * Null Object design pattern.
 * @see http://blog.roodo.com/rocksaying/archives/6028145.html
 * @author: rock
 * @license: GNU LGPL
 */
class NullObject {
    public function __get($k) {
        return $this;
    }
    public function __set($k, $v) {
        //donothing
    }
    /**
     To invoke isset with a property of NullObject will be false always.
     But to invoke isset with a NullObject itself will be true.
     If you want to detect some thing is NullObject, using is_a($x, 'NullObject') instead.
     */
    public function __isset($k) {
        return false;
    }
    public function __call($m, $args) {
        //donothing
        return $this;
    }
}
?>
