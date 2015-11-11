<?php
require_once 'Schema.php';
require_once 'DatabaseHelper.php';

/**
 * Database RowIterator class
 *
 * @see http://blog.roodo.com/rocksaying/archives/3532653.html
 * @package library
 * @subpackage database
 * @version: $Revision: 53 $, $Date: 2012-11-06 15:31:51 +0800 (二, 06 11月 2012) $
 * @author: $Author: shirock.tw@gmail.com $
 * @license: GNU LGPL
 */
class DatabaseRowIterator extends ArrayIterator {
    protected $row;
    protected $columnList;
    public function __construct(&$row) {
        $this->row = $row;
    }

    public function rewind() {
        $this->columnList = $this->row->columnList();
        reset($this->columnList);
        return $this;
    }

    public function valid() {
        return key($this->columnList) === null ? false : true;
    }

    public function current() {
        $index = current($this->columnList);
        return $this->row[$index];
    }

    public function key() {
        $index = current($this->columnList);
        return $index;
    }

    public function next() {
        $index = next($this->columnList);
        return $this->row[$index];
    }
}


/**
 * Database Row class
 * 
 * This is an ORM generic class.
 * 
 * @package library
 * @subpackage database 
 * @version: $Revision: 53 $, $Date: 2012-11-06 15:31:51 +0800 (二, 06 11月 2012) $
 * @author: $Author: shirock.tw@gmail.com $  
 * @license: GNU LGPL
 */
class DatabaseRow extends ArrayObject {
    /**
     * Current row's data.
     * @var object
     */
    protected $data;
    
    /**
     * mark changed fields.
     * assign() and setter will set changed.
     * unset() will unset changed.
     */
    protected $changed;

    /**
     * Mark fields to be 'Unchanged'.
     * When update(), the 'Unchanged' fields will be ignored.
     *
     * @param $fields
     *  * If you don't given, set all 'Unchagned'.
     *  * Give a string, split to an array by ','.
     *  * Give an array, set those fields to be 'Unchanged'.
     */
    public function unchange($fields = true) {
        if ($fields === true) {
            $this->changed = array(); //set all to 'Unchanged'
        }
        else {
            if (is_string($fields))
                $fields = preg_split('/\s*,\s*/', $fields);

            foreach ($fields as $field) {
                unset( $this->changed[$field] );
            }
        }
    }

    protected $iterator;

    /**
     * It means: $array[] = $v
     */
    public function append($v) {
        //do nothing.
        //It does not supported.
        return $this;
    }

    /**
     * It means: $array[$index] = $v
     * Ruby's []= method.
     */
    public function offsetSet($index, $v) {
        $this->data->$index = $v;
        $this->changed[$index] = true;
        return $this;
    }

    /**
     * It means: $object->$key = $v
     */
    public function __set($key, $value) {
        $this->offsetSet($key, $value);
    }

    /**
     * It means:  $v = $array[$index]
     * Ruby's [] method.
     */
    public function offsetGet($index) {
        if ( isset($this->data->$index) )
            return $this->data->$index;
        else
            return false;
    }

    /**
     * It means: $v = $object->$key
     */
    public function __get($key) {
        return $this->offsetGet($key);
    }

    /**
     * It means: isset($array[$index])
     */
    public function offsetExists($index) {
        return isset($this->data->$index);
    }

    /**
     * It means: isset($object->$key)
     */
    public function __isset($key) {
        return isset($this->data->$key);
    }

    /**
     * It means: unset($array[$index])
     */
    public function offsetUnset($index) {
        unset($this->data->$index);
        unset($this->changed[$index]);
        return $this;
    }

    /**
     * It means: unset($object->$key)
     */
    public function __unset($key) {
        $this->offsetUnset($key);
    }
    
    /**
     * It means: count($array)
     */
    public function count() {
        return count($this->columnList);
    }

    public function getIterator() {
        return $this->iterator;
    }

    /**
     * Handle of relationship database.
     * @var PDO object
     */
    protected $db;

    /**
     * Table name.
     * @var string
     * @access protected
     */
    protected $table;

    /**
     * Schema.
     * @var SchemaTable
     */
    protected $schema;

    protected $_full_schema;

    /**
     * Constructor
     *
     * If you do not give $parameters, this will just load class but does not
     * to construct a workable instance.
     *
     * @param object|array $parameters A set of parameters.<br/>
     *  db: The handle of relationship database.<br/>
     *  schema: Schema object.<br/>
     *  table: Table name which this object maps to. Table name will be capitalized.<br/>
     *  $key: If you set a property that was named as primary key (like $parameters->id),
     *   it will invoke get() to load data from database.
     */
    public function __construct($parameters = false) {
        if ( empty($parameters) )
            return;

        $this->iterator = new DatabaseRowIterator($this);

        if ( is_array($parameters) )
            $parameters = (object) $parameters;

        $this->db = $parameters->db;

        $this->table = (isset($parameters->table)
            ? $parameters->table
            : get_class($this)
        );

        $this->_full_schema = $parameters->schema;
        $this->schema = $parameters->schema->table($this->table);

        if (!is_a($this->schema, 'NullObject')) {
            $keyName = $this->schema->_primaryKey;
            if ( isset($parameters->$keyName) and !empty($parameters->$keyName) ) {
                // load from database
                $this->get($parameters->$keyName);
            }
            else {
                $this->clean();
            }
        }
        else {
            $this->clean();
        }
    }

    /**
     * Get SchemaTable object of this table.
     * @return SchemaTable
     */
    public function schema() {
        return $this->schema;
    }

    /**
     * Get all fields' name
     * @return array All fields' name.
     */
    public function columnList() {
        return $this->schema->columnList();
    }

    /**
     * Get all fields' name
     * Alias of columnList()
     * @return array All fields' name.
     */
    public function fieldList() {
        return $this->columnList();
    }

    /**
     * Get the name of primary key
     *
     * @return string Name of primary key
     */
    public function primaryKey() {
        $key = $this->schema->_primaryKey;
        if ($key)
            return $key;
        $list = $this->columnList();
        return $list[0]; //若無 _primaryKey, 則將第一個欄位作為主鍵。
    }

    /**
     * Get table name
     * @return string Table name
     */
    public function table() {
        return $this->table;
    }

    /**
     * 當 defalt 為 false 時，此欄位不可省略。
     * 此時根據 type 給定預設值。
     * 數值型態: 0; 非數值型態: ''
     */
    protected function defaultValueByType(&$schema) {
        if ($schema->default !== false)
            throw new Exception("Only when default is false could invoke this method.");

        switch ($schema->type) {
            case 'bool':
            case 'int':
            case 'integer':
            case 'float':
            case 'numeric':
            case 'decimal':
                return 0;
                break;

            default:
                return '';
                break;
        }
    }

    /**
     * Set data as empty.
     *
     * 根據 schema 的預設值清除資料中的欄位內容。
     * 有 default 給 default；沒有預設值的，根據資料型態給定。
     *  * 數值型態: 0
     *  * string: ''
     * Notice: All fields will be set 'Unchanged'.
     */         
    public function clean() {
        $this->data = null;
        foreach ($this->columnList() as $name) {
            $columnSchema = $this->schema->column($name);
            //echo "set this->data->{$name} = {$columnSchema->default}\n";
            //if ($columnSchema->default === true)
            //    continue;
            //else
            if ($columnSchema->default === false)
                $this->data->$name = $this->defaultValueByType($columnSchema);
            else
                $this->data->$name = $columnSchema->default;
        }
        $this->unchange();
    }

    /**
     * Use query result to new an instance of DatabaseRow.
     *
     * factory() 預期的資料來源是自資料庫中讀取的，不檢查資料欄位的格式。
     * update() 時也不會再次執行編碼動作。insert() 通常會失敗(因為已存在)。
     *
     * @param object|array A data of result from database query.
     * @return DatabaseRow
     */
    public function factory($data) {
        $p->db = $this->db;
        $p->schema = $this->_full_schema;
        $p->table = $this->table;
        $obj = new DatabaseRow($p);
        $obj->assign($data);
        $obj->unchange();
        return $obj;
    }

    /**
     * Assign data which input by user.
     *
     * When you assign values to fields, DatabaseRow will apply the pattern
     * role to check them.
     *
     * assign() 預期的資料來源是由使用者輸入的(未經編碼)，所以會對資料欄位進行格
     * 式檢查。同時影嚮 insert() 與 update() 於儲存具有編碼動作的欄位時，
     * 會對其執行編碼動作。
     *
     * 當表格中具有非對稱編碼動作的欄位時，一般的儲存動作應該略過那些欄位。
     * 作法有二:
     * 1. query 時，不取出那些欄位。
     * 2. unchange() 那些欄位。
     *
     * @param object|array $data Assign $data to $this->data
     * @param array [$returnInvalidFields] 若提供的資料內容之欄位值是無效的(invalid)，
     *  則那些無效值之欄位名稱將被放入 $returnInvalidFields 陣列中。
     *  可供指派者檢查哪些資料內容有誤。<br/>
     *  若 assign() 回傳 true ，則此陣列將會是空陣列。<br/>
     *  此為選擇性參數。
     * @return bool 若回傳 false，則可檢查 $returnInvalidFields 之內容。
     */
    public function assign($data, &$returnInvalidFields = false) {
        $invalidFields = array();

        if (is_array($data))
            $data = (object)$data;

        $this->data = null;
        $this->changed = array();
        foreach ($this->columnList() as $field) {
            $fieldSchema = $this->schema->column($field);

            if ( !isset($data->$field) ) {
                if ( $fieldSchema->default === false ) {
                    // 若 default 為 false ，表示不允許空值。
                    $invalidFields[] = $field;
                    continue;
                }

                $value = $fieldSchema->default;
                $this->changed[$field] = true;
            }
            else {
                $value = $data->$field;

                if ( !$fieldSchema->isValid($value) ) {
                    $invalidFields[] = $field;
                    continue;
                }

                switch ($fieldSchema->type) {
                    case 'int':
                    case 'integer':
                        $this->data->$field = intval($value);
                        break;

                    case 'float':
                    case 'numeric':
                    case 'decimal':
                        $this->data->$field = floatval($value);
                        break;

                    default:
                        $this->data->$field = $value;
                        break;
                }

                $this->changed[$field] = true;
            }
        } //end foreach ($this->_columnList as $field)

        if ($returnInvalidFields !== false)
            $returnInvalidFields = $invalidFields;
        return (empty($invalidFields) ? true : false);
    } //end function assign

    /**
     * Is all field's content valid?
     *
     * @param object|array $data Default is current data ($this->data)
     * @param array $ignores Ignores these fields. Default is false.
     * @return bool
     */
    public function isValid($data = false, $ignores = false) {
        if ($data === false)
            $data = $this->data;
        return $this->schema->isAllValid($data, $ignores);
    }

    /**
     * Update or Insert data.
     *
     * @param string $method 'Insert' or 'Update'
     * @param object|array $data  If id is not set or empty, it will insert an new record.
     * @return mixed
     */
    private function _insert_or_update($method) {
        $data = $this->data;

        $generateSqlString = "_generate{$method}SqlString";
        if ($sqlString = $this->$generateSqlString($data)) {
            if ($query = $this->db->exec($sqlString)) {
                $this->unchange();
                return $query;
            }
        }
        //print_r($this->db->errorInfo());
        return false;
    }

    protected $lastQueryString;

    /**
     * Get last query string
     */
    public function getLastQueryString() {
        return $this->lastQueryString;
    }

    /**
     * Generate SQL string for insert().
     *
     * @param mixed $data
     * @return string|false $sqlString
     */
    protected function _generateInsertSqlString($data) {
        $values = array();
        $cols = array();

        foreach ($this->columnList() as $key) {
            $columnSchema = $this->schema->column($key);

            // Note: If not give data and schema->default is true, it means to use database's default value.
            if (!isset($data->$key)) {
                if ($columnSchema->default === true) {
                    //echo "skip\n";
                    continue; //skip, use database's default.
                }
                else {
                    //echo "set to default\n";
                    $data->$key = $columnSchema->default;
                }
            }
            else if ($data->$key === true and $columnSchema->default === true) {
                //echo "skip\n";
                continue; //skip, use database's default.
            }

            $value = $data->$key;
            if (false === ($value = DatabaseHelper::prepareValue($this->db, $columnSchema, $value)))
                return false;

            $cols[] = DatabaseHelper::quoteName($key);
            $values[] = $value;
        }
        $cols = implode(',', $cols);
        $values = implode(',', $values);
        $tableName = DatabaseHelper::quoteName($this->table);

        $sqlString = "INSERT INTO {$tableName} ({$cols}) VALUES ({$values});";

        $this->lastQueryString = $sqlString;
        return $sqlString;
    }

    /**
     * Insert data.
     *
     * //@param object|array $data  Struct of data (Object or Associative array)
     * @param $callback(&$this, &$data) What you want to do after insert.
     *        If $callback return a result and $data is false, then
     *        it will assign the result to this.
     *        Usally to load inserted data to instance of DatabaseRow.
     * @return mixed  false means failed.
     */
    public function insert($callback = false) {
        $result = $this->_insert_or_update('Insert');

        if ($result and $callback) {
            $rc = call_user_func_array($callback, array(&$this));
            if ($rc) {
                //echo "assign back\n";
                $this->assign($rc);
            }
        }
        return $result;
    }

    /**
     * Insert data.
     *
     * Alias of insert method.
     *
     * @param object|array $data  Struct of data (Object or Associative array)
     * @return mixed  false or id (id > 0).
     */
    public function create() {
        return $this->insert();
    }

    /**
     * Generate SQL string for update().
     *
     * @param mixed $data
     * @return string|false $sqlString
     */
    protected function _generateUpdateSqlString($data) {
        $primaryKey = $this->primaryKey();
        
        $columnSchema = $this->schema->column($primaryKey);
        $whereValue = DatabaseHelper::prepareValue($this->db,
            $columnSchema, $data->$primaryKey);

        $values = array();
        foreach ($this->columnList() as $key) {
            if ( !isset($this->changed[$key]) )
                continue; // 'Unchanged', skip.

            if ($key == $primaryKey
                or method_exists($this, 'update' . ucfirst($key))
                    // if this field has a special method to update, skip it.
                or !isset($data->$key)
                    // No data to update this field.
            )
            {
                continue;
            }

            $columnSchema = $this->schema->column($key);
            $value = $data->$key;
            if (false === ($value = DatabaseHelper::prepareValue($this->db,
                    $columnSchema, $value))
                )
            {
                return false;
            }
            //$values[] = sprintf('"%s" = %s', $key, $this->_prepare($key, $value));
            $values[] = '"' . $key . '" = ' . $value;
        }
        $values = implode(',', $values);
        $tableName = DatabaseHelper::quoteName($this->table);
        $primaryKey = DatabaseHelper::quoteName($primaryKey);

        $sqlString = "UPDATE {$tableName} SET {$values} WHERE {$primaryKey} = {$whereValue};";

        $this->lastQueryString = $sqlString;
        return $sqlString;
    }

    /**
     * Update data.
     *
     * It will not update the primary key or the fields that have special udpate methods.
     *
     * @param object|array $data  If id is not set or empty, it will insert an new record.
     * @return mixed  false or true. if create, return id.
     */
    public function update() {
        return $this->_insert_or_update('Update');
    }

    /**
     * Update primary key
     *
     * @param mixed $oldKeyValue
     * @param mixed $newKeyValue
     * @return bool  true or false
     */
    public function updatePrimaryKey($newKeyValue) {
        $keyName = $this->primaryKey();
        $oldKeyValue = $this->$keyName;

        $columnSchema = $this->schema->column($keyName);

        $oldValue = $oldKeyValue;
        if (false === ($oldValue = DatabaseHelper::prepareValue($this->db,
                $columnSchema, $oldValue)))
        {
            return false;
        }

        $newValue = $newKeyValue;
        if (false === ($newValue = DatabaseHelper::prepareValue($this->db,
                $columnSchema, $newValue)))
        {
            return false;
        }
        $table = DatabaseHelper::quoteName($this->table);
        $keyName = DatabaseHelper::quoteName($keyName);
        $sqlString = "UPDATE {$table} SET {$keyName} = {$newValue}
            WHERE {$keyName} = {$oldValue};";

        $this->lastQueryString = $sqlString;

        $r = $this->db->query($sqlString) ? true : false;
        if ($r)
            $this->data->$keyName = $newKeyValue;
        return $r;
    }

    /**
     * Generate SQL string for get().
     *
     * @param mixed $keyValue
     * @return string|false $sqlString
     */
    protected function _generateGetSqlString($keyValue) {
        $primaryKey = $this->primaryKey();

        if ($keyValue === false)
            $keyValue = $this->data->$primaryKey;

        if (false === ($preparedValue = DatabaseHelper::prepareValue($this->db,
                $this->schema->column($primaryKey), $keyValue)))
            return false;

        $tableName = DatabaseHelper::quoteName($this->table);
        $primaryKey = DatabaseHelper::quoteName($primaryKey);

        $sqlString = "SELECT * FROM {$tableName} WHERE {$primaryKey} = {$preparedValue};";

        $this->lastQueryString = $sqlString;
        return $sqlString;
    }

    /**
     * Get raw data.
     *
     * @param mixed $keyValue
     * @return mixed  false or an object of record.
     */
    protected function _getData($keyValue, $decode = true) {
        if ($keyValue == false)
            $keyValue = $this->offsetGet($this->primaryKey());

        $sqlString = $this->_generateGetSqlString($keyValue);

        if (empty($sqlString))
            return false;

        if ($query = $this->db->query($sqlString)) {
            $result = $query->fetchObject();
            $query->closeCursor();
            if ( !empty($result) ) {
                //print_r($result);
                foreach ($this->columnList() as $field) {
                    if ($decode) {
                        $columnSchema = $this->schema->column($field);
                        $this->data->$field = $columnSchema->decode($result->$field);
                    }
                    else {
                        $this->data->$field = $result->$field;
                    }
                }
                $this->unchange();
                return $this->data;
            }
        }

        //echo "not found\n";
        $this->clean();
        return false;
    }

    /**
     * Get raw data.
     *
     * @param mixed $keyValue
     * @return mixed  false or an object of record.
     */
    public function getRawData($keyValue = false) {
        return $this->_getData($keyValue, false);
    }

    /**
     * Get data.
     *
     * It will call decoder after read data from database.
     *
     * @param mixed $keyValue
     * @return mixed  false or an object of record.
     */
    public function get($keyValue = false) {
        return $this->_getData($keyValue, true);
    }

    /**
     * Generate SQL string for delete().
     *
     * @param mixed $keyValue
     * @return string|false $sqlString
     */
    protected function _generateDeleteSqlString($keyValue) {
        $keyName = $this->primaryKey();

        if ($keyValue === false)
            $keyValue = $this->data->$keyName;

        if (false === ($preparedValue = DatabaseHelper::prepareValue($this->db,
                $this->schema->column($keyName), $keyValue)))
        {
            return false;
        }
        $tableName = DatabaseHelper::quoteName($this->table);
        $keyName = DatabaseHelper::quoteName($keyName);
        $sqlString = "DELETE FROM {$tableName} WHERE {$keyName} = {$preparedValue};";

        $this->lastQueryString = $sqlString;
        return $sqlString;
    }

    /**
     * Delete data
     * @param mixed $keyValue  The key of row.
     * @return bool  false or true.
     */
    public function delete($keyValue = false) {
        $sqlString = $this->_generateDeleteSqlString($keyValue);
        if (empty($sqlString))
            return false;

        $r = $this->db->query($sqlString) ? true : false;
        if ($r)
            $this->clean();
        return $r;
    }
}
?>
