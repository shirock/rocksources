<?php
# phpunit
class ByteArrayTest extends PHPUnit_Framework_TestCase
{
    function testByteArrayMethods()
    {
        $src = '123456';
        $src_len = strlen($src);

        $ba = new ByteArray($src);
        
        $this->assertEquals('ByteArray', get_class($ba));

        $this->assertEquals($src_len, count($ba));
        $this->assertEquals($src_len, strlen($ba));
        $this->assertEquals($src, (string)$ba);

        for ($i = 0; $i < $src_len; ++$i)
            $this->assertEquals($src[$i], $ba[$i]);

        $this->assertEquals('1', $ba[0]);
        $this->assertEquals('6', $ba[5]);
        $this->assertEquals('6', $ba[-1]);
        
        $this->assertTrue(isset($ba[0]));
        $this->assertTrue(isset($ba[1]));
        $this->assertTrue(isset($ba[5]));

        $this->assertFalse(isset($ba[-1]));
        $this->assertFalse(isset($ba[6]));

        $ba[0] = 'a';
        $this->assertEquals('a', $ba[0]);        
        $ba[3] = 'D';
        $this->assertEquals('D', $ba[3]);
        $ba[5] = 'y';
        $this->assertEquals('y', $ba[-1]);
        $this->assertEquals('y', $ba[5]);
        $ba[-1] = 'z';
        $this->assertEquals('z', $ba[-1]);
        $this->assertEquals('z', $ba[5]);
        $ba[6] = 'a';
        $this->assertFalse(isset($ba[6]));
        $this->assertEquals('a23D5z', (string)$ba);
    }

    /**
     * @expectedException PHPUnit_Framework_Error_Notice
     */
    public function testIndexOutOfRange()
    {
        $ba = new ByteArray('123');
        $res = $ba[3];
    }

    /**
     * @expectedException PHPUnit_Framework_Error_Notice
     */
    public function testIndexOutOfNegativeRange()
    {
        $ba = new ByteArray('123');
        $res = $ba[-4];
    }
}

?>
