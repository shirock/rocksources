<?php
/**
 * Time-Sorted ID functions.
 * 
 * ID 是按時序產生 (Time-Sorted ID)，故可排序 。
 *
 * @author: shirock.tw@gmail.com
 * @site: https://www.rocksaying.tw/archives/2021/sql-temp-id.html
 * @license: GNU LGPL
 */
class TempId
{
    const CHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxy';

    static function division(int $n, &$s)
    {
        $r = $n % 60;
        $q = (int)($n / 60);

        $s[] = self::CHARS[$r];
        if ($q >= 60)
            self::division($q, $s);
        else if ($q > 0)
            $s[] = self::CHARS[$q];
        return;
    }
    
    private static function temp_file($fname)
    {
        $tmp_path = getenv('TMP');
        if (!$tmp_path)
            $tmp_path = getenv('TEMP');
        return ($tmp_path ? $tmp_path . '/' : '/tmp/') . $fname;
    }

    /**
     * 產生一個具唯一性的 id ，長度為 8 個字元 (32 bits)。
     * 可排序 。
     */
    static function make()
    {
        $tmp_file = self::temp_file('~tempid.lock');

        $now = new DateTimeImmutable();

        $ts = $now->format('y,m,d,h,i,s,v');
        // echo $ts, "\n";
        $ts = explode(',', $ts);
        $y = ((int)$ts[0]) - 21;
        $m = ((int)$ts[1]) * 2678400; // * 31;
        $d = ((int)$ts[2]) * 86400;
        $h = ((int)$ts[3]) * 3600;
        $i = ((int)$ts[4]) * 60;
        $s = (int)$ts[5];
        $v = (int)$ts[6];
        $ss = $m + $d + $h + $i + $s;
        // echo "y = $y, ss = $ss, v = $v\n";

        if (file_exists($tmp_file)) {
            $fp = fopen($tmp_file, 'r+');
            flock($fp, LOCK_EX);
            $vid = fgets($fp);
            fseek($fp, 0);
        }
        else {
            $fp = fopen($tmp_file, 'w');
            flock($fp, LOCK_EX);
            $vid = 0;
        }
        // echo "vid = $vid\n";
        if ($vid == $v) {
            usleep(1000);
            $now = new DateTimeImmutable();
            $v = (int)$now->format('v');
            // echo "new v = $v\n";
        }
        fwrite($fp, sprintf('%03d', $v));
        fflush($fp);
        fclose($fp);

        $id1 = self::CHARS[$y];

        $id2 = [];
        self::division($ss, $id2);
        $id2 = str_pad(implode('', array_reverse($id2)), 5, '0', STR_PAD_LEFT);

        $id3 = [];
        self::division($v, $id3);
        $id3 = str_pad(implode('', array_reverse($id3)), 2, '0', STR_PAD_LEFT);

        return $id1.$id2.$id3;
    }

    /**
     * 產生一個具唯一性的 id ，長度為 16 個字元。
     * 第一個字元是指定內容，其他15個字元全部是數字字元。
     * 可排序 。
     */
    static function make16($first='0')
    {
        $tmp_file = self::temp_file('~tempid16.lock');

        if (file_exists($tmp_file)) {
            $fp = fopen($tmp_file, 'r+');
            flock($fp, LOCK_EX);
            $vid = fgets($fp);
            fseek($fp, 0);
        }
        else {
            $fp = fopen($tmp_file, 'w');
            flock($fp, LOCK_EX);
            $vid = '';
        }

        $now = new DateTimeImmutable();
        $ts = substr($now->format('YmdHisv'), 2, 15);
        // echo "\t$ts\n";

        if ($vid == $ts) {
            usleep(1000);
            $now = new DateTimeImmutable();
            $ts = substr($now->format('YmdHisv'), 2, 15);
            // echo "\tnew ts = $ts\n";
        }

        fwrite($fp, $ts);
        fflush($fp);
        fclose($fp);

        return $first.$ts;
    }
}

/**
 * Alias, short name.
 */
function tsid()
{
    return TempId::make();
}

function tsid16($first='0')
{
    return TempId::make16($first);
}

// echo TempId::make(), "\n";
// echo tsid(), "\n";
// echo tsid(), "\n";
// echo tsid(), "\n";
// echo tsid(), "\n";
// date_default_timezone_set('Asia/Taipei');
// echo TempId::make16('p'), "\n";
// echo TempId::make16('P'), "\n";
// echo tsid16('X'), "\n";
// echo tsid16(), "\n";
// echo tsid16(), "\n";
?>