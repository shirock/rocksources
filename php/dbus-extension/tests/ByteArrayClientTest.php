<?php
## phpunit
# run 'python bytearray-service.py' first.
class ByteArrayClientTest extends PHPUnit_Framework_TestCase
{
    function setUp() {
        $sample_service = 'tw.idv.rock.sample.Hello';
        $sample_object_path = '/tw/idv/rock/sample/Hello';
        $sample_interface = 'tw.idv.rock.sample.Hello';

        $this->bus = new Dbus(Dbus::BUS_SESSION);
         
        $this->proxy = $this->bus->createProxy(
            $sample_service, $sample_object_path, $sample_interface);
    }

    function testTransformBetweenService() {
        $loop = 10;
        $filepath = "image.png";
        $s = file_get_contents($filepath);
        $file_size = strlen($s);

        #echo "PID: ", posix_getpid(), "\n";
        #fgets(STDIN);

        for ($i = 0; $i < $loop; ++$i) 
        { 
            $a = new Bytearray("sf\0sfs f");
            $this->assertEquals("$a", "sf\0sfs f");
            $this->assertEquals(strlen($a), 8);
            $this->assertEquals(count($a), 8);

            $a = new ByteArray("9 fdsa\tsf\0sfs f");
            $this->assertEquals("$a", "9 fdsa\tsf\0sfs f");
            $this->assertEquals(strlen($a), 15);
            $this->assertEquals(count($a), 15);

            $res = $this->proxy->SayAy(array(1,2,3,4));
            $this->assertEquals($res, 'Size 4');

            $res = $this->proxy->SayAy($a);
            $this->assertEquals($res, 'Size 15');

            $res = $this->proxy->SayAy(new ByteArray($s));
            $this->assertEquals($res, "Size $file_size");

            $a = $this->proxy->RetAy($filepath);
            $this->assertEquals(get_class($a), 'ByteArray');
            $this->assertEquals((string)$a, $s);
            
        }

        file_put_contents('/tmp/dump.png', $a);
        echo "View /tmp/dump.png\n";
        #fgets(STDIN);
    }

}
?>
