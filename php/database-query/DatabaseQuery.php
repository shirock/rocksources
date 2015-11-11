<?php
require_once 'Schema.php';
require_once 'DatabaseHelper.php';
require_once 'DatabaseRow.php';

/**
 * Database Query class
 *
 * Active Record/LINQ class
 *
 * @package library
 * @subpackage database
 * @version: $Revision: 56 $, $Date: 2012-11-06 17:13:31 +0800 (二, 06 11月 2012) $
 * @author: $Author: shirock.tw@gmail.com $
 * @license: GNU LGPL
 */
class DatabaseQuery {
    /**
     * Handle of relationship database.
     * @var object
     */
    protected $db = false;

    /**
     * Schema object.
     * @var Schema
     */
    protected $schema = false;

    /**
     * Get SchemaTable object of this table.
     * @return SchemaTable
     */
    public function schema() {
        return $this->schema;
    }

    protected $statments = array();

    protected $lastQueryString = '';

    const Op_Equal = 0;
    const Op_Like = 1;

    /**
     * Disable decode when select().
     * @var bool
     */    
    const DISABLE_DECODE = false;

    /**
     * Enable decode when select().
     * @var bool
     */    
    const ENABLE_DECODE = true;

    /**
     * Constructor
     *
     * If you do not give $parameters, this will just load class but does not
     * to construct a workable instance.
     *
     * @param object|array $parameters A set of parameters.<br/>
     *  db: The handler of relationship database.<br/>
     *  schema: Schema object.<br/>
     * @access public
     */
    public function __construct($parameters = false) {
        if ( empty($parameters) )
            return;

        if ( is_array($parameters) )
            $parameters = (object) $parameters;

        $this->db = $parameters->db;
        if ( isset($parameters->schema) )
            $this->schema = $parameters->schema;
        else
            $this->schema = new NullObject;
    }

    /**
     * 調整字串為 SQL 查詢句可用的安全格式.
     *
     * NOTICE: 這裡的格式意義跟 PDO::prepare 不一樣.
     *
     * 1.欄位用冒號, prepare行為: 套上雙引號(this->quoteName())
     * <br/>
     * 2. 替換值一律用 ?, prepare行為: 自動根據算符左值取欄位值，並套上單引號
     * (調用 quote).
     * 只有跟在比較算符後的 ? 才會被替換.
     * 當無法替換時，會擲出例外.
     * <br/>
     * $args 格式: array(
     *     '欄位1' => 替換值 或 '',
     *     '欄位2' => 替換值 或 ''
     *      .
     * );
     * <br/>
     * 如果欄位只需要調整名稱，沒有替換值，可設為任何值；通常是''，很安全。
     * <br/>
     * 3. SQL 的 not equal 算符是 <> ，不是 != 。遇到 != 的算符一併修正之。
     *
     * $s = "where :a = ? and :b =?";
     * $args = array('a' => 1, 'b' => "I'm rock")
     * prepared: where "a" = '1' and "b" ='I''m rock'
     *
     * $s = ':a > ? or :a = :b and :b !=? and :c <> abs(:d) or :dd or :d';
     * $args = array('a' => 1, 'b' => "I'm rock", 'c' => '', 'd' => '');
     * $prepared = $this->query->prepare($s, $args);
     * $this->assertEquals(
     *     "\"a\" > '1' or \"a\" = \"b\" and \"b\" <>'I''m rock' and \"c\" <> abs(\"d\") or :dd or \"d\"",
     *     $prepared
     * );
     *
     * @param string $str
     * @param array $parameters Default is false.
     * @return string prepared string.
     */
    public function prepare($s, $parameters = false) {
        //SQL 的 not equal 算符是 <> ，不是 != ，修正。
        if (strpos($s, '!=') !== false) {
            $s = strtr($s, array('!=' => '<>'));
        }

        $ss = explode('?', $s);
        //print_r($ss);

        $ssIndex = 0;
        $ssCount = count($ss) - 1;

        $encode = false;
        if (isset($this->statments['from'])) {
            $table = $this->statments['from'];
            if (isset($this->schema->$table)) {
                $tableSchema = $this->schema->table($table);
                $encode = true;
            }
        }

        for ($ssIndex = 0; $ssIndex < $ssCount; ++$ssIndex) {
            if ( preg_match('/(\s|^):?(?P<field>\S+)\s*(?P<op>[!<>=]+)\s*$/', $ss[$ssIndex], $m) <= 0)
                throw new Exception("The element in \"{$ss[$ssIndex]}?\" can not be replaced.");
            //print_r($m);
            if ( !isset($parameters[$m['field']]) )
                throw new Exception("The element in \"{$ss[$ssIndex]}?\" can not be replaced.");
            $value = $parameters[$m['field']];

            if ($encode) {
                $columnSchema = $tableSchema->column($m['field']);
                if ( !is_a($columnSchema, 'NullObject') and isset($columnSchema->encoder) ) {
                    //echo "{$m['field']} encode\n";
                    $value = $columnSchema->encode($value);
                }
            }
            $ss[$ssIndex] .= $this->db->quote($value);
        }

        $ss = implode('', $ss);
        //echo $ss, "\n";

        // 為列於 $parameters 清單中的名稱參數加上括號。可以接受中文或非英文字元的名稱。
        if ($parameters) {
            foreach ($parameters as $field => $value) {
                $ss = preg_replace('/(\s|\(|-|^):?('. preg_quote($field) .')([\s\),;=><]|$)/',
                    '\1'. DatabaseHelper::quoteName($field) .'\3',
                    $ss);
            }
        }

        // 為其他未列於 $parameters 清單中的名稱參數加上括號。僅限使用英文字元、數字與底線的名稱。
        $ss = preg_replace_callback('/(\s|\(|-|^):([\w_]+)([\s\),;=><]|$)/',
            create_function('$matches', 
                'return $matches[1] . DatabaseHelper::quoteName($matches[2]) . $matches[3];'),
            $ss);
        return $ss;
    }

    public function getLastQueryString() {
        return $this->lastQueryString;
    }

    protected function _generateSelectPart() {
        if ( isset($this->statments['select']) and $this->statments['select'] ) {
            $statments = $this->statments['select'];
            if ( is_string($statments) ) {
                $select = DatabaseHelper::quoteName($statments);
            }
            else {
                $select = array();
                foreach ($statments as $field => $expr) {
                    if (is_int($field)) // index array, element is the field's name.
                        $select[] = DatabaseHelper::quoteName($expr);
                    else // association array, field => expr
                        $select[] = $expr . ' AS ' . DatabaseHelper::quoteName($field);
                }
                $select = implode(', ', $select);
            }
        }
        else {
            $select = '*';
        }
        return $select;
    }


    protected function _prepareField($field, $value, $op) {
        if ($op == self::Op_Like) {
            $s = ":$field = ?";
            $name = $field;
            $value = "%$value%";
            $result = $this->prepare($s, array($name => $value));
            //$result will like "name" = '%rock%'
            //                  get ^" = '^ replace to LIKE
            //$result = preg_replace('/\s+=\s+/', 'LIKE', $result);
            $p = strlen( DatabaseHelper::quoteName($field) );
            $result = substr($result, 0, $p) . ' LIKE ' . substr($result, $p + 3);
        }
        else {
            preg_match('/(?P<fieldName>\S+)(\s|[!=><])*(?P<op>[=><])$/', $field, $m);
            //print_r($m);
            if (!isset($m['op']) ) { // $m['op'] == ''
                $s = ":$field = ?";
                $name = $field;
            }
            else {
                $s = ":$field ?";
                $name = $m['fieldName'];
            }
            $result = $this->prepare($s, array($name => $value));
        }

        return $result;
    }

    protected function _generateWherePart() {
        if ( isset($this->statments['where']) ) {
            $aggregate = ' AND ';
            $op = self::Op_Equal;
            $statments = $this->statments['where'];
        }
        else if ( isset($this->statments['or_where']) ) {
            $aggregate = ' OR ';
            $op = self::Op_Equal;
            $statments = $this->statments['or_where'];
        }
        else if ( isset($this->statments['like']) ) {
            $aggregate = ' AND ';
            $op = self::Op_Like;
            $statments = $this->statments['like'];
        }
        else if ( isset($this->statments['or_like']) ) {
            $aggregate = ' OR ';
            $op = self::Op_Like;
            $statments = $this->statments['or_like'];
        }
        else
            return '';
        
        $parts = array();
        foreach ($statments as $field => $value) {
            if (is_array($value)) {
                $parts[] = $this->prepare($field, $value);
            }
            else {
                $parts[] = $this->_prepareField($field, $value, $op);
            }
        }
        return 'WHERE ' . implode($aggregate, $parts);
    }

    protected function _generateWhatByPart($act, $key) {
        if (isset($this->statments[$key])) {
            $statments = $this->statments[$key];
            
            if (is_string($statments)) {
                $groupBy = DatabaseHelper::quoteName($statments);
            }
            else {
                $groupBy = array();
                foreach ($statments as $field) {
                    $groupBy[] = DatabaseHelper::quoteName($field);
                }
                $groupBy = implode(', ', $groupBy);
            }
            return $act . ' ' . $groupBy;
        }
        return '';
    }

    protected function _generateGroupByPart() {
        return $this->_generateWhatByPart('GROUP BY', 'group_by');
    }

    protected function _generateOrderByPart() {
        return $this->_generateWhatByPart('ORDER BY', 'order_by');
    }

    /**
     * Make SQL string for getAll().
     *
     * @return string|false $sqlString
     * @access protected
     */
    protected function generateSqlString() {
        $queryString = array('SELECT');
        $queryString[] = $this->_generateSelectPart();

        $queryString[] = 'FROM';
        $queryString[] = DatabaseHelper::quoteName($this->statments['from']);

        $s = $this->_generateWherePart();
        if ( !empty($s) )
            $queryString[] = $s;

        $s = $this->_generateGroupByPart();
        if ( !empty($s) )
            $queryString[] = $s;

        $s = $this->_generateOrderByPart();
        if ( !empty($s) )
            $queryString[] = $s;

        $this->lastQueryString = implode(' ', $queryString) . ';';
        return $this->lastQueryString;
    }
    
    public function from($table) {
        $this->statments['from'] = $table;
        return $this;
    }
    
    public function where($conditions) {
        $this->statments['where'] = $conditions;
        return $this;
    }
    
    public function or_where($conditions) {
        $this->statments['or_where'] = $conditions;
        return $this;
    }
    
    public function like($conditions) {
        $this->statments['like'] = $conditions;
        return $this;
    }
    
    public function or_like($conditions) {
        $this->statments['or_like'] = $conditions;
        return $this;
    }
    
    public function group_by($fields) {
        $this->statments['group_by'] = $fields;
        return $this;
    }
    
    public function order_by($fields) {
        $this->statments['order_by'] = $fields;
        return $this;
    }

    /**
     parse arguments of select().
     */
    protected function _select_args($argc, $args) {
        // I tested. In this case, declaration _select_args($args) is faster than _select_args(&$args).

        // usage 1, 2: if first argument is array.
        // usage 3: if first argument is string.
        // fetch_style: if last or last 2nd argument is int.
        // decode: if last or last 2nd argument is bool.
        
        //$argc = func_num_args();
        #echo "argc: $argc \n";
        #print_r($args);
        
        $fields = false; // '*'
        if ($argc >= 1) {
            $arg_current_index = 0;
            if (is_array($args[0])) {
                $fields = $args[0];
                ++$arg_current_index;
            }
            else if (is_string($args[0])) {
                $fields = array();
                while ($arg_current_index < $argc) {
                    $arg_current = $args[$arg_current_index];
                    if ( !is_string($arg_current))
                        break;
                    ++$arg_current_index;
                    $fields[] = $arg_current;
                }
            }
            
            while ($arg_current_index < $argc) {
                $arg_current = $args[$arg_current_index];
                ++$arg_current_index;
                if (is_bool($arg_current)) {
                    $decode = $arg_current;
                }
                else if (is_int($arg_current)) {
                    $fetch_style = $arg_current;
                }
            }
        }

        if (!isset($decode))
            $decode = DatabaseQuery::ENABLE_DECODE;
        if (!isset($fetch_style))
            $fetch_style = PDO::FETCH_OBJ;

        #echo "fields\n";
        #print_r($fields);
        #echo "\n";
        #echo "decode: ", $decode ? 'true' : 'false', "\n";
        #echo "fetch_style: ", $fetch_style, "\n";
        #echo "----\n";
        return array($fields, $decode, $fetch_style);
    }

    /**
     * For unit test.
     */
    public function test_select() {
        return $this->_select_args(func_num_args(), func_get_args());
    }

    /**
     * Select data.
     *
     * Variable arguments.
     *
     * usage 1: select(array(field1, field2, ...), $decode, $fetch_style)
     * usage 2: select(array(field1=>expr1, ...), $decode, $fetch_style)
     *   set alias name to field or expr.
     * usage 3: select(field1, field2, fieldn..., $decode, $fetch_style)
     *
     * if you need set alias for expr or field, you should choice usage 2.
     *
     * $decode 與 $fetch_style 要放在參數列的最後處，兩者位置可對調。
     *
     * @return mixed  false or an array of object of records.
     * @access public
     */
    public function select() {
        // usage 1, 2: if first argument is array.
        // usage 3: if first argument is string.
        // fetch_style: if last or last 2nd argument is int.
        // decode: if last or last 2nd argument is bool.

        //$this->statments['select'] = $fields;
        list($this->statments['select'], $decode, $fetch_style) =
            $this->_select_args(func_num_args(), func_get_args());

        $sqlString = $this->generateSqlString();

        $table = $this->statments['from'];
        $this->statments = array(); //clean
        
        if (empty($sqlString))
            return false;

        //echo $sqlString, "\n";

        if ($query = $this->db->query($sqlString)) {
            $results = array();

            if ($fetch_style == PDO::FETCH_CLASS) {
                //if (!class_exists('DatabaseRow'))
                //    require 'database_row.php';
                /* 
                PDO::FETCH_CLASS will set properties before constructor.
                However DatabaseRow's constructor will reset all (clean) properties,
                so all data will be clean. 
                That does not match our required.
                There is an issue to talk about how to set properties later.
                See https://bugs.php.net/bug.php?id=49521
                According to this issue, I refactor here.
                */
                $fetch_style |= PDO::FETCH_PROPS_LATE;
                $query->setFetchMode(PDO::FETCH_CLASS|PDO::FETCH_PROPS_LATE, 
                    'DatabaseRow', 
                    array(array(
                        'db' => $this->db,
                        'schema' => $this->schema,
                        'table' => $table
                    ))); 
            }

            /*
            The following ways are not working as our expected.
            
            1.
            $query->fetchObject(PDO::FETCH_CLASS|PDO::FETCH_PROPS_LATE,
                    'DatabaseRow', 
                    array(array(
                        'db' => $this->db,
                        'schema' => $this->schema,
                        'table' => $table
                    )));
            Fatel: argument 2 should be long/int.
            
            2.
            $query = $this->db->query($sqlString, PDO::FETCH_CLASS | PDO::FETCH_PROPS_LATE,
                    'DatabaseRow', 
                    array(array(
                        'db' => $this->db,
                        'schema' => $this->schema,
                        'table' => $table
                    )));
            Assert error: PDO::FETCH_PROPS_LATE does not effect.
            */
            
            while ( $result = $query->fetch($fetch_style) ) {
                $results[] = ($decode == true
                    ? $this->_decode($table, $result)
                    : $result
                );
            }
            $query->closeCursor(); //release data
            //print_r($results);
            return $results;
        }
        else {
            return false;
        }
    }


    /**
     * After get data from database, decode those.
     *
     * @param string mixed $data
     * @return string|false Return an escaped and quoted string of $value
     *  if $value is valided. False means an invalid value.
     * @access protected
     */
    protected function _decode($table, &$data) {
        if ( !isset($this->schema->$table) )
            return $data;
        $tableSchema = $this->schema->$table;
        $is_obj_style = is_array($data) ? false : true;
        foreach ($data as $key => $value) {
            $columnSchema = $tableSchema->column($key);
            //$data->$key = $columnSchema->decode($value);
            $v = $columnSchema->decode($value);
            if ($is_obj_style)
                $data->$key = $v;
            else
                $data[$key] = $v;
        }
        return $data;
    }

}

?>
