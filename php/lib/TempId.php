<?php
/**
 * TempId::make(): string
 * 
 * 產生一個具唯一性的 id ，固定 8 個字元。
 * 此 id 是按時序產生，故可排序。
 * 但理論上，任何方法產生的 id 都不保證順序性。
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

    static function make()
    {
        $tmp_path = getenv('TMP');
        if (!$tmp_path)
            $tmp_path = getenv('TEMP');
        $tmp_file = ($tmp_path ? $tmp_path . '/' : '') . '~tempid.lock';

        $fp = fopen($tmp_file, 'w');
        flock($fp, LOCK_EX);
        $now = new DateTime();
        usleep(1000);
        fclose($fp);

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

        $id1 = self::CHARS[$y];

        $id2 = [];
        self::division($ss, $id2);
        $id2 = str_pad(implode('', array_reverse($id2)), 5, '0', STR_PAD_LEFT);

        $id3 = [];
        self::division($v, $id3);
        $id3 = str_pad(implode('', array_reverse($id3)), 2, '0', STR_PAD_LEFT);

        return $id1.$id2.$id3;
    }
}
// echo TempId::make(), "\n";
// echo TempId::make(), "\n";
// echo TempId::make(), "\n";
?>