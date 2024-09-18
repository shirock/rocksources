<?php
/**
This file is part of RockPDO.

RockPDO is free software: you can redistribute it and/or modify it under the 
terms of the GNU Lesser General Public License as published by the Free Software 
Foundation, either version 3 of the License, or (at your option) any later version.

RockPDO is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>. 

Copyright 2024 shirock.tw@gmail.com

@author shirock.tw@gmail.com
@site https://rocksaying.github.io/
@license GNU LGPL
 */
class RockPDO extends PDO
{
    public const ORDER_ASC = 0;
    public const ORDER_DESC = 1;

    /**
     IS NULL 和 IS NOT NULL 另外處理。
    '!=' is not standard operator. 允許使用，視為 '<>'
     */
    public const Comparisons = ['<', '>', '=', '<>', '<=', '>='];
    public const Likes  = [
        'contains'    => '%V%',
        'starts_with' => 'V%',
        'ends_with'   => '%V'];

    /**
    double_quote()是靜態方法，一律用雙引號作為分隔符號。
    quote_identifier()是實例方法，按理會依PDO driver決定分隔符號。
     */
    public static function double_quote(string $k): string
    {
        return sprintf('"%s"', str_replace('"', '""', $k));
    }

    /**
    PDOStatement 不能處理欄位名稱為中文或任何多位元文字的情況。
    碰到這種情況，需要自己準備 SQL 語句。
    我一向採用 ANSI/ISO 原則。
    Using Single quotes for String values. Using Double quotes for Delimited identifiers.

    PostgreSQL: https://www.postgresql.org/docs/current/sql-syntax-lexical.html
    SQL Server: https://learn.microsoft.com/en-us/sql/t-sql/statements/set-quoted-identifier-transact-sql
    SQLite: https://sqlite.org/lang_keywords.html
    MySQL: set ANSI_QUOTES. See https://dev.mysql.com/doc/refman/8.0/en/sql-mode.html#sqlmode_ansi_quotes
     */
    public function quote_identifier(string $k): string
    {
        return sprintf('"%s"', str_replace('"', '""', $k));
    }

    /**
    PDO::quote() 的 $value 已明確宣告為 string 。
    當 $value 的型態不是字串時，PHP 會按隱式轉型方法轉為字串。
    碰到 null, false 時，將不會得到 SQL 語法預期的結果。
    ps. false will implicitly to ''.

    @param mixed $value
    null return NULL,
    true return '1', false return '0',
    others return PDO::quote($value).
    @return string
     */
    public function quote_value(mixed $value): string
    {
        if (is_bool($value))
            return $this->quote($value ? '1' : '0');
        return is_null($value) ? 'NULL' : $this->quote($value);
    }

    /**
    產生查詢句。是 query_formatted() 呼叫的方法之一。

    @param string $format
      1. :name 自 $options 取得鍵為 name 的值，替換為查詢句的欄位值 (單引號)。
      2. !name 直接將 name 視為資料庫識別符，改用雙引號括起 (不是用 $options 的值)。
         但若 $options 查得到鍵，且值的型態為 object，則視為變數，替換為值。
    @param array $options
      Example:
      $options = ['find_field' => (object)'id', 'id' => 123];
      echo $pdo->interpolate('SELECT * FROM !Member WHERE !find_field = :id', $options);
      Output:  SELECT * FROM "Member" WHERE "id" = '123'
     */
    public function interpolate(string $format, array $options): string
    {
        $callback = function ($match) use ($options) {
            // var_dump($match); // [0: all, 1: type, 2:key]
            list($_, $t, $k) = $match;
            if ($t == ':' and isset($options[$k])) {
                return $this->quote_value($options[$k]);
            }
            else if ($t == '!') {
                // scalar: https://www.php.net/manual/en/language.types.object.php
                $v = is_object($options[$k]??false) ? $options[$k]->scalar : $k;
                return $this->quote_identifier($v);
            }
            return $_; // do nothing
        };

        $query_string = preg_replace_callback('/(:|!)(\S+)/', $callback, $format);
        return $query_string;
    }

    /**
    產生查詢句。是 query_formatted() 呼叫的方法之一。

    @param string $format
    參數位置的格式是 ?? 或 ?。
    ?? 表示此位置的參數為 identifier； ? 表示此位置的參數為 value。
    預設是按順序代入參數。

    例如: query_formatted('SELECT * FROM ?? WHERE ?? = ?', 'TableName', '編號', 100);
    將向 Database 送出查詢句: SELECT * FROM "TableName" WHERE "編號" = '100'.

    若是參數位置後綴數字，則此數字為參數索引值，表示代入第幾個參數。
    索引值從 0 開始。

    例如: query_formatted('SELECT * FROM ??0 WHERE id1 = ?1 OR id2 = ?1', 'TableName', 100);
    將向 Database 送出查詢句: SELECT * FROM "TableName" WHERE id1 = '100' OR id2 = '100'.

    @param mixed $args
    參數清單。

    @see https://www.php.net/manual/en/pdo.query.php query().
    @see https://www.php.net/manual/en/function.sprintf.php sprintf().
     */
    public function sprintf(string $format, mixed ...$args): string
    {
        $i = 0;

        $callback = function ($match) use ($args, &$i) {
            // var_dump($match);
            if ($match[1] != '') // 指定位置
                $v = $args[$match[1]];
            else // 不指定位置
                $v = $args[$i++];

            if (str_starts_with($match[0], '??')) // identifier
                $v = $this->quote_identifier($v);
            else
                $v = $this->quote_value($v);

            return $v;
        };

        $query_string = preg_replace_callback('/\?{1,2}(\d*)/', $callback, $format);
        // echo $query_string, "\n";
        return $query_string;
    }

    /**
    送出可以代入參數的查詢句給資料庫，回傳資料庫查詢結果。
    
    @param string $format
    依參數方式決定。
    如果參數是一個陣列，那呼叫 $this->interpolate() 產生查詢句。
    否則會呼叫 $this->sprintf() 產生查詢句。

    @param mixed $args
    參數清單。

    @return PDOStatement|false
    同 query().
     */
    public function query_formatted(string $format, mixed ...$args): PDOStatement|false
    {
        if (isset($args[0]) and is_array($args[0]))
            $query_string = $this->interpolate($format, $args[0]);
        else
            $query_string = $this->sprintf($format, ...$args);
        return $this->query($query_string);
    }

    /**
    WHERE ... AND ...
     */
    protected function prepare_where(?array $conditions): string|false
    {
        if (empty($conditions))
            return '';

        $cs = [];
        foreach ($conditions as $k => $v) {
            // 取消不可靠的表達方式
            /*if (is_string($v) and ($v[0] == '<' or $v[0] == '>')) {
                if ($v[1] == '=' or $v[1] == '>') {
                    $op = substr($v, 0, 2);
                    $skip = 2;
                }
                else {
                    $op = $v[0];
                    $skip = 1;
                }
                $v = $this->quote(trim(substr($v, $skip)));
            }
            else*/
            if (is_null($v)) {
                $op = 'IS';
                $v = 'NULL';
            }
            else if (is_array($v)) {
                // [comparsion, value]
                if (is_null($v[1])) {
                    $op = ($v[0] == '=' ? 'IS' : 'IS NOT');
                    $v = 'NULL';
                }
                else if (in_array($v[0], self::Comparisons)) {
                    $op = $v[0];
                    $v = $this->quote_value($v[1]);
                }
                else if ($v[0] == '!=') {
                    $op = '<>';
                    $v = $this->quote_value($v[1]);
                }
                else if (is_callable($v[0])) {
                    // callback($value): [$op, $new_value]
                    list($op, $v) = $v[0]($v[1]);
                    $v = $this->quote_value($v);
                }
                else if (isset(self::Likes[$v[0]])) {
                    $op = 'LIKE';
                    $v = $this->quote_value(str_replace('V', $v[1], self::Likes[$v[0]]));
                }
                else {
                    return false;
                }
            }
            else {
                $op = '=';
                $v = $this->quote_value($v);
            }
            $cs[] = sprintf('%s %s %s', $this->quote_identifier($k), $op, $v);
        }
        $cs = sprintf('WHERE %s', implode(' AND ', $cs));
        return $cs;
    }

    /**
    SELECT fields FROM table WHERE ... AND ... ORDER BY field ASC
    @param string $table
    @param mixed $fields
    用陣列寫出要選取的欄位名稱。
    或 null, '', 或 '*' 表示所有欄位。
    @param ?array $conditions
    @param ?string $order_by
    排序基準的欄位名稱。
    @param int $order_method
    RockPDO::ORDER_ASC 或 RockPDO::ORDER_DESC
     */
    public function select(
        string $table,
        mixed $fields=null,
        ?array $conditions=null,
        ?string $order_by=null,
        int $order_method=self::ORDER_ASC
        ): PDOStatement|false
    {
        $sqlstr = '';
        if (empty($fields) or $fields== '*') {
            $fs = '*';
        }
        else {
            $fs = [];
            foreach ($fields as $k) {
                $fs[] = $this->quote_identifier($k);
            }
            $fs = implode(',', $fs);
        }

        $cs = $this->prepare_where($conditions);
        if ($cs === false) {
            return false;
        }

        if ($order_by) {
            $os = sprintf(
                'ORDER BY %s %s',
                $this->quote_identifier($order_by),
                $order_method==self::ORDER_DESC ? 'DESC' : 'ASC');
        }
        else {
            $os = '';
        }

        $sqlstr = sprintf('SELECT %s FROM %s %s %s;', $fs, $this->quote_identifier($table), $cs, $os);
        // echo $sqlstr,"\n";
        return $this->query($sqlstr);
    }

    /**
    簡化 RockPDO::select($table, '*', ...)->fetchAll(PDO::FETCH_OBJ);
     */
    public function fetch_all_objects(
        string $table,
        ?array $conditions=null,
        ?string $order_by=null,
        int $order_method=self::ORDER_ASC): array|false
    {
        $stat = $this->select($table, null, $conditions, $order_by, $order_method);
        return $stat ? $stat->fetchAll(PDO::FETCH_OBJ) : false;
    }

    /**
    從表格讀出枚舉清單。
    用於表格欄位具有 id 和 name 類似組合，而使用者需要 array(key=>value, ...) 的清單。
     */
    public function enumerate(string $table, string $key='id', string $value_field='name'): array
    {
        $stat = $this->select($table, [$key, $value_field], null, $key);
        return $stat ? $stat->fetchAll(PDO::FETCH_KEY_PAIR) : [];
    }

    public function select_by_id(string $table, $id): PDOStatement|false
    {
        return $this->select($table, null, ['id'=>$id]);
    }

    /**
    RockPDO::select_by_id()->fetchObject() 的連鎖操作使用率太高了。
    增加簡化方法。

    @param ?string $class See PDOStatement::fetchObject().
    @param array $constructorArgs See PDOStatement::fetchObject().
     */
    public function fetch_object(
        string $table, 
        $id, 
        ?string $class=null, 
        array $constructorArgs=[]): object|false
    {
        $stat = $this->select_by_id($table, $id);
        return $stat ? $stat->fetchObject($class, $constructorArgs) : false;
    }

    /**
    INSERT INTO table (key of values) VALUES (value of values)
     */
    public function insert(string $table, array $values): int|false
    {
        if (array_is_list($values) and is_array($values[0])) {
            return $this->insert_rows($table, $values);
        }

        $fs = [];
        $vs = [];
        foreach ($values as $k => $v) {
            $fs[] = $this->quote_identifier($k);
            $vs[] = $this->quote_value($v);
        }
        $fs = implode(',', $fs);
        $vs = implode(',', $vs);

        $sqlstr = sprintf('INSERT INTO %s (%s) VALUES (%s);', $this->quote_identifier($table), $fs, $vs);
        // echo $sqlstr, "\n";
        return $this->exec($sqlstr);
    }

    public function insert_rows(string $table, array $values): int|false
    {
        $fs = implode(
            ',',
            array_map(
                function($k){ return $this->quote_identifier($k); },
                array_keys($values[0])
            )
        );
        /*
        $vs = implode(',', array_map(function($row){
                $s = sprintf('(%s)', implode(',', array_map(function($v){
                                return $this->quote($v);
                            },
                            $row)));
                return $s;
            }, $values));
        */
        $vs = [];
        foreach ($values as $row) {
            $rs = [];
            foreach ($row as $v) {
                $rs[] = $this->quote_value($v);
            }
            $vs[] = sprintf('(%s)', implode(',', $rs));
        }
        $vs = implode(',', $vs);

        $sqlstr = sprintf('INSERT INTO %s (%s) VALUES %s;', $this->quote_identifier($table), $fs, $vs);
        // echo $sqlstr, "\n";
        return $this->exec($sqlstr);
    }

    /**
    UPDATE table SET field1=value1, fields2=value2, ... WHERE ... AND ...
     */
    public function update(string $table, array $conditions, array $values): int|false
    {
        $cs = $this->prepare_where($conditions);

        $vs = [];
        foreach ($values as $k => $v) {
            $v = $this->quote_value($v);
            $vs[] = sprintf('%s=%s', $this->quote_identifier($k), $v);
        }
        $vs = implode(',', $vs);

        $sqlstr = sprintf('UPDATE %s SET %s %s;', $this->quote_identifier($table), $vs, $cs);
        // echo $sqlstr, "\n";
        return $this->exec($sqlstr);
    }

    public function update_by_id(string $table, $id, array $values): int|false
    {
        return $this->update($table, ['id'=>$id], $values);
    }

    /**
    DELETE FROM table WHERE ... AND ...
     */
    public function delete(string $table, array $conditions): int|false
    {
        $cs = $this->prepare_where($conditions);

        $sqlstr = sprintf('DELETE FROM %s %s;', $this->quote_identifier($table), $cs);
        // echo $sqlstr, "\n";
        return $this->exec($sqlstr);
    }

    public function delete_by_id(string $table, $id): int|false
    {
        return $this->delete($table, ['id'=>$id]);
    }
}
?>