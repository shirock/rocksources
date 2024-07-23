<?php
/**
 * @author: shirock.tw@gmail.com
 * @site: https://rocksaying.tw/
 * @license: GNU LGPL
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
    query_formatted() 呼叫的字串格式化方法。
    為了方便除錯，才設為公開方法。
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
                $v = $this->quote($v);

            return $v;
        };

        $query_string = preg_replace_callback('/\?{1,2}(\d*)/', $callback, $format);
        // echo $query_string, "\n";
        return $query_string;
    }

    /**
    送出可以代入參數的查詢句。
    
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

    @return PDOStatement|false
    同 query().

    @see https://www.php.net/manual/en/pdo.query.php query().
    @see https://www.php.net/manual/en/function.sprintf.php sprintf().
     */
    public function query_formatted(string $format, mixed ...$args): PDOStatement|false
    {
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
                    $v = $this->quote($v[1]);
                }
                else if ($v[0] == '!=') {
                    $op = '<>';
                    $v = $this->quote($v[1]);
                }
                else {
                    return false;
                }
            }
            else {
                $op = '=';
                $v = $this->quote($v);
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

    public function select_by_id(string $table, $id): PDOStatement|false
    {
        return $this->select($table, null, ['id'=>$id]);
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
            $vs[] = is_null($v) ? 'NULL' : $this->quote($v);
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
                $rs[] = is_null($v) ? 'NULL' : $this->quote($v);
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
            $v = is_null($v) ? 'NULL' : $this->quote($v);
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