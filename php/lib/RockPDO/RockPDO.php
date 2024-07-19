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
    PDOStatement 不能處理欄位名稱為中文或任何多位元文字的情況。
    碰到這種情況，需要自己準備 SQL 語句。
    我一向採用 ANSI/ISO 原則。
    Using Single quotes for String values. Using Double quotes for Delimited identifiers.

    PostgreSQL: https://www.postgresql.org/docs/current/sql-syntax-lexical.html
    SQL Server: https://learn.microsoft.com/en-us/sql/t-sql/statements/set-quoted-identifier-transact-sql
    SQLite: https://sqlite.org/lang_keywords.html
    MySQL: set ANSI_QUOTES. See https://dev.mysql.com/doc/refman/8.0/en/sql-mode.html#sqlmode_ansi_quotes
     */
    public function quote_identifier($k) {
        return sprintf('"%s"', str_replace('"', '""', $k));
    }

    /**
    WHERE ... AND ...
     */
    protected function prepare_where($conditions): string|false
    {
        if (empty($conditions))
            return '';

        $cs = [];
        foreach ($conditions as $k => $v) {
            if (is_string($v) and ($v[0] == '<' or $v[0] == '>')) {
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
            else if (is_null($v)) {
                $op = 'IS';
                $v = 'NULL';
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
     */
    public function select(
        string $table,
        $fields=null,
        array $conditions=null,
        $order_by=null,
        $order_method=self::ORDER_ASC
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