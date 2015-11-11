<?php
require_once 'PHPUnit/Framework.php';
require_once '../DatabaseQuery.php';

class DatabaseQuerySelectArgumentsTest extends PHPUnit_Framework_TestCase
{
    function setUp() {
        $this->q = new DatabaseQuery();
    }

    function testSelect() {
        list($f, $d, $s) = $this->q->test_select();
        $this->assertFalse($f);
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }
    
    function testSelect1Array() {
        list($f, $d, $s) = $this->q->test_select(array('a'));
        $this->assertContains('a', $f);
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testSelect1Decode() {
        list($f, $d, $s) = $this->q->test_select(DatabaseQuery::DISABLE_DECODE);
        $this->assertFalse($f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testSelect1FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select(PDO::FETCH_ASSOC);
        $this->assertFalse($f);
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testSelect1Field2Decode() {
        list($f, $d, $s) = $this->q->test_select('a', DatabaseQuery::DISABLE_DECODE);
        $this->assertContains('a', $f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testSelect1Field2FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select('a', PDO::FETCH_ASSOC);
        $this->assertContains('a', $f);
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testSelect1Field2Decode3FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select('a', DatabaseQuery::DISABLE_DECODE, PDO::FETCH_ASSOC);
        $this->assertContains('a', $f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testSelect1Field2FetchNum3Decode() {
        list($f, $d, $s) = $this->q->test_select('a', PDO::FETCH_NUM, DatabaseQuery::DISABLE_DECODE);
        $this->assertContains('a', $f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

    function testSelect1Array2FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select(array('a'), PDO::FETCH_ASSOC);
        $this->assertContains('a', $f);
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testSelect1Array2Decode3FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select(array('a'), DatabaseQuery::DISABLE_DECODE, PDO::FETCH_ASSOC);
        $this->assertContains('a', $f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testSelect1Array2FetchNum3Decode() {
        list($f, $d, $s) = $this->q->test_select(array('a'), PDO::FETCH_NUM, DatabaseQuery::DISABLE_DECODE);
        $this->assertContains('a', $f);
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

    function testSelect1Field2Field3FetchNum() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', PDO::FETCH_NUM);
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertEquals(2, count($f));
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

    function testSelect1Field2Field3Field4FetchNum() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', PDO::FETCH_NUM);
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertContains('c', $f);
        $this->assertEquals(3, count($f));
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

    function testSelect1Field2Field3Field4Decode() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', DatabaseQuery::DISABLE_DECODE);
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertContains('c', $f);
        $this->assertEquals(3, count($f));
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testSelect1Field2Field3Field4FetchNum5Decode() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', PDO::FETCH_NUM, DatabaseQuery::DISABLE_DECODE);
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertContains('c', $f);
        $this->assertEquals(3, count($f));
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

    function testBadSelect1Field2Field3Field4Decode5Decode() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', DatabaseQuery::ENABLE_DECODE, DatabaseQuery::DISABLE_DECODE);
        // it should take last decode setting.
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertContains('c', $f);
        $this->assertEquals(3, count($f));
        $this->assertFalse($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testBadSelect1Field2Field3Field4FetchNum5FetchAssoc() {
        list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', PDO::FETCH_NUM, PDO::FETCH_ASSOC);
        // it should take last fetch_style setting.
        $this->assertContains('a', $f);
        $this->assertContains('b', $f);
        $this->assertContains('c', $f);
        $this->assertEquals(3, count($f));
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_ASSOC);
    }

    function testBadSelect1Field2Array3Field() {
        list($f, $d, $s) = $this->q->test_select('a', array('b'), 'c');
        // ignore 2 and 3 arguments.
        $this->assertContains('a', $f);
        $this->assertEquals(1, count($f));
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_OBJ);
    }

    function testBadSelect1Field2Array3Field4FetchNum() {
        list($f, $d, $s) = $this->q->test_select('a', array('b'), 'c', PDO::FETCH_NUM);
        // ignore 2 and 3 arguments.
        $this->assertContains('a', $f);
        $this->assertEquals(1, count($f));
        $this->assertTrue($d);
        $this->assertEquals($s, PDO::FETCH_NUM);
    }

#    function testTime() {
#        $start_time = time();
#        for ($i = 0; $i < 900000; ++$i) {
#            list($f, $d, $s) = $this->q->test_select('a', 'b', 'c', PDO::FETCH_NUM, true);
#        }
#        $end_time = time();
#        echo $end_time - $start_time, "\n";
#        $this->assertTrue(true);
#    }
}

// Run this test if this source file is executed directly.
if (!defined('PHPUnit_MAIN_METHOD')) {
    require_once 'PHPUnit/TextUI/TestRunner.php';

    $suite  = new PHPUnit_Framework_TestSuite('DatabaseQuerySelectArgumentsTest');
    $result = PHPUnit_TextUI_TestRunner::run($suite);
}
?>
