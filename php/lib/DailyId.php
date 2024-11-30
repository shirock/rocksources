<?php
/**
 * 按日期編流水號。每日重置為 1。
 * 
 * 若資料表格的 id 需要按日期編流水號，例如 1 月 1 日第2筆記錄 id 是 P0101002,
 * 1 月 2 日第1筆記錄是 P0102001 這類型式的話，就可以用 DailyId 取得編號。
 * 
 * 使用者用 key 指定編號用途或分類。例如 'Order' 表示訂單編號，'Ship' 表示出貨單編號。
 * 則 $order_id = $id_factor->update('Order')， $ship_id = $id_factor->update('Ship')
 * 就會得到不同流水號。
 * 
 * 要求資料庫引擎的 UPDATE 陳述可用 RETURNING 條目。
 * 已確認可用者: PostgreSQL; SQLite 3.35.0 (2021-03-12).
 * https://www.rocksaying.tw/archives/2022/sql-update-returning.html
 * @author: shirock.tw@gmail.com
 * @site: https://www.rocksaying.tw/
 * @license: GNU LGPL
 */
class DailyId
{
    private PDO $pdo;
    private string $table;
    private string $reset_datetime;

    /**
    @param string $reset_time
    id 重置時刻。預設零時重置(midnight)。
    'noon', '6 am', '08:00' 等格式皆為有效值。
    例如有些作業單位將今天12:00到隔天11:59的單據視為同一工作天，
    所以它們需要午時('noon' 或 '12:00')才重置指定id。
     */
    public function __construct(PDO $pdo, string $table, $reset_time='00:00')
    {
        // $drv = $pdo->getAttribute(PDO::ATTR_DRIVER_NAME);
        // if ( !in_array($drv, ['pgsql', 'sqlite']) ) {
        //     throw new PDOException('Unsupport');
        // }

        $this->pdo = $pdo;
        $this->table = sprintf('"%s"', str_replace('"', '""', $table));
        $this->reset_datetime = 'today ' . $reset_time;
    }

    /**
    取得工作天為準的"今天"日期。依 $this->reset_datetime 而定。
     */
    private function op_today(): DateTimeInterface
    {
        $base = new DateTimeImmutable($this->reset_datetime);
        $now = new DateTimeImmutable();
        if ($now < $base) {
            // echo "-1 day\n";
            $today = $now->sub(new DateInterval('P1D')); // 使用前一天的日期
        }
        else {
            $today = $now;
        }
        return $today;
    }

    /**
    建立 DailyId 使用的表格
     */
    public function init_table()
    {
        $sqlcmd = <<<SQLCMD
        CREATE TABLE IF NOT EXISTS {$this->table}
        (
            key CHARACTER VARYING(20) NOT NULL,
            day DATE,
            value INTEGER DEFAULT 0,
            PRIMARY KEY (key)
        );
        SQLCMD;

        return $this->pdo->exec($sqlcmd);
    }

    /**
    增加每日計數的 key
     */
    public function append($key)
    {
        $today = $this->pdo->quote((new DateTimeImmutable())->format('Y-m-d'));
        $k = $this->pdo->quote($key);

        $qs = <<<CHECK_CMD
        SELECT key FROM {$this->table} WHERE key = $k;
        CHECK_CMD;

        $stat = $this->pdo->query($qs);
        $v = $stat->fetchColumn(0);
        if ($v !== false) // existed.
            return false;

        $qs = <<<INSERT_CMD
        INSERT INTO {$this->table} (key, day, value) VALUES ($k, $today, 0);
        INSERT_CMD;
        return $this->pdo->exec($qs);
    }

    public function update($key, $length=false, $prefix=''): string
    {
        $k = $this->pdo->quote($key);
        $today = $this->pdo->quote($this->op_today()->format('Y-m-d'));
        
        $reset_cmd = <<<RESETSTR
        UPDATE {$this->table} SET value = 1, day = $today WHERE key = $k AND day <> $today;
        RESETSTR;
        // echo $reset_cmd, "\n";

        $rc = $this->pdo->exec($reset_cmd);
        if ($rc < 1) {
            $qs = <<<SQLSTR
            UPDATE {$this->table} SET value = value + 1 WHERE key = $k
            RETURNING value;
            SQLSTR;
            // echo $qs, "\n";
    
            $stat = $this->pdo->query($qs);
            if ($stat === false)
                return false;
    
            $v = $stat->fetchColumn(0);
            if ($v === false)
                return false;
        }
        else {
            $v = 1;
        }

        // 我不確定哪個方式比較快。
        // 上面分成兩行UPDATE，但幾乎只寫入一個欄位。
        // 下面只要一行UPDATE，但每次寫入兩個欄位。
/*      
        $qs = <<<SQLSTR
        UPDATE {$this->table} SET 
        value = (
            CASE WHEN day <> $today THEN 1
            ELSE value+1 END),
        day = (
            CASE WHEN day <> $today THEN $today
            ELSE day END)
        WHERE key = $k
        RETURNING value;
        SQLSTR;
        echo $qs, "\n";

        $stat = $this->pdo->query($qs);
        if ($stat === false)
            return false;

        $v = $stat->fetchColumn(0);
        if ($v === false)
            return false;
*/

        if ($length > 0) {
            $pad_len = $length - strlen($prefix);
            $id = $prefix . str_pad($v, $pad_len, '0', STR_PAD_LEFT);
        }
        else {
            $id = $v;
        }
        return $id;
    }

    /**
    時間戳記的年份用字元表示，從2024起算。
    如 2024 = A, 2025 = B
     */
    public function make($key, $length=false, $prefix='')
    {
        $ctime = $this->op_today();
        $cyear = chr(intval($ctime->format('y')) + 41);
        $stamp = sprintf('%s%s%s', $prefix, $cyear, $ctime->format('md'));
        return $this->update($key, $length, $stamp);
    }

    /**
    時間戳記為西元紀年後2位數，如 240102
     */
    public function make_ce_stamp2($key, $length=false, $prefix='')
    {
        $ctime = $this->op_today();
        $stamp = $prefix . $ctime->format('ymd');
        return $this->update($key, $length, $stamp);
    }

    /**
    時間戳記為西元紀年4位數，如 20240102
     */
    public function make_ce_stamp4($key, $length=false, $prefix='')
    {
        $ctime = $this->op_today();
        $stamp = $prefix . $ctime->format('Ymd');
        return $this->update($key, $length, $stamp);
    }

    /**
    時間戳記為民國紀年，如 1130102
     */
    public function make_roc_stamp($key, $length=false, $prefix='')
    {
        $ctime = $this->op_today();
        $cyear = intval($ctime->format('Y')) - 1911;
        $stamp = sprintf('%s%s%s', $prefix, $cyear, $ctime->format('md'));
        return $this->update($key, $length, $stamp);
    }

}

// date_default_timezone_set('Asia/Taipei');
// $pdo = new PDO('sqlite:did.db');
// $pdo = new PDO('pgsql:host=localhost;port=5432;dbname=ktvpos;user=user');
// $did = new DailyId($pdo, 'DailyId', 'noon');
// $did->init_table();
// echo $did->append('test') ? 'add' : 'existed', "\n";

// $new_id = $did->update('test');
// echo "$new_id \n";

// $new_id = $did->make('test', 12, 'P');
// echo "$new_id \n";

// $new_id = $did->make('test', 10, 'P');
// echo "$new_id \n";

// $new_id = $did->make_ce_stamp2('test', 10, 'P');
// echo "$new_id \n";

// $new_id = $did->make_ce_stamp4('test', 12, 'P');
// echo "$new_id \n";

// $new_id = $did->make_roc_stamp('test', 12, 'P');
// echo "$new_id \n";
?>