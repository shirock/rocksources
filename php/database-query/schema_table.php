<?php
//if ( !class_exists('NullObject') )
require 'NullObject.php';

/**
 * SchemaTableColumn class
 * 
 * @package library
 * @subpackage database 
 * @version: $Revision: 136 $, $Date: 2008-07-09 20:40:39 +0800 (星期三, 09 七月 2008) $
 * @author: $Author: rock $  
 * @license: GNU LGPL
 */
class SchemaTableColumn {
    /**
     * Data Type.
     * int, string (for SQL data type: varchar, char...), bit,
     * timestamp, numeric, float.
     *
     * @var string, read-only preperty
     */
    protected $type;

    /**
     * Data input pattern.
     * <p>
     * Regexp pattern or ctype function, like ctype_digit, ctype_alnum. <br/>
     *  1. REGEX pattern. You do not need to quote with '/'. <br/>
     *  2. ctype_? functions, like 'ctype_digit'. <br/>
     *  3. is_? functions, like 'is_int', 'is_string'. <br/>
     *  4. customer method like 'Account::validUserId'. <br/>
     *     If name of scope is 'Php', it means invoking the function of PHP
     *     instead of invoking the method of class.
     *  5. Predefined Patterns: 'pattern_email', 'pattern_ip',
     *     'pattern_domain_name', 'pattern_timestamp'.
     * </p><p>
     * This is difference from $type. This was used for data which
     * user input to. Therefore the variable's type of data is string often.
     * </p>
     *
     * @var string, read-only preperty
     */
    protected $pattern;

    protected static $PredefinedPatterns = array(
        'pattern_email' => '^[\\w\\.]+@[\\w\\.]+$',
        'pattern_ip'    => '^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$',
        'pattern_domain_name'   => '[\\w\\.]+',
        'pattern_timestamp'     => '^(\\d{2,4}-\\d{2}-\\d{2}(\\s\\d{2}:\\d{2}:\\d{2})?(\\s[\\w\\+]+)?)|CURRENT_TIMESTAMP$'
    );

    /**
     * Get predefined patterns
     * @return array predefined patterns.
     */
    public static function getPredefinedPatterns() {
        return self::$PredefinedPatterns;
    }

    /**
     * Default value.
     * <p>
     *  1. 'false': no default value. <br/>
     *  2. 'true': it will generate by DBMS automatically. <br/>
     *  3. others: the default value of this field.
     *  4. If type is 'bool', you should use 0 or 1 as default value.
     * </p>
     *
     * @var mixed, read-only preperty
     */
    protected $default;

    /**
     * Data notic.
     *
     * Usually it is a message to show user.
     *
     * @var string, read-only preperty
     */
    protected $notice;

    /**
     * Encode casading.
     * <p>
     * It tells how to encode data before store to database.<br/>
     * For example, 'Account::encryptPassword', will invoke
     * Account::encryptPassword($data).
     * </p><p>
     * If name of scope is 'Php', it means invoking the function of PHP
     * instead of invoking the method of class.<br/>
     * You may use '|' to pipe more encoders.
     * </p>
     *
     * @var array, read-only preperty
     */
    protected $encoder;

    /**
     * Decode casading.
     * <p>
     * It tells how to decode data after fetch from database.<br/>
     * For example, 'IpCast::toInt|Php::long2ip', will
     * invoke long2ip(IpCast::toInt($data)).
     * </p><p>
     * If name of scope is 'Php', it means invoking the function of PHP
     * instead of invoking the method of class.<br/>
     * You may use '|' to pipe more encoders.
     * </p>
     *
     * @var array, read-only preperty
     */
    protected $decoder;

    /**
     * Constructor
     *
     * @param object|array Object (or array) contains type, pattern, default and
     *  notice, etc.
     */
    public function __construct($parameters) {
        if (is_array($parameters))
            $parameters = (object)$parameters;

        foreach (get_class_vars(get_class($this)) as $key => $defaultValue) {
            if ( !isset($parameters->$key) )
                $this->$key = false;
            else
                $this->$key = $parameters->$key;
        }
        $this->encoder = empty($this->encoder) ? false : explode('|', $this->encoder);
        $this->decoder = empty($this->decoder) ? false : explode('|', $this->decoder);
    }

    /**
     * Getter for read-only preperty.
     */
    public function __get($key) {
        if (isset($this->$key))
            return $this->$key;
        return false;
    }
    
    /**
     * isset for read-only preperty.
     */
    public function __isset($key) {
        if ($key == 'default')
            return true;
        else if ($this->$key)
            return true;
        return false;
        //return isset($this->$key);
    }

    /**
     * Is valid?
     *     
     * @param mixed $data     
     * @return bool     
     * @access public     
     */         
    public function isValid($data) {
        $isType = 'is_' . $this->type;
        if (function_exists($isType)) {
            if ($isType($data) and $this->type != 'string') {
                //echo $isType, "\n";
                return true;
            }
        }

        if ($this->type == 'bit' and preg_match("/[xb]'[a-f0-9]*'/i", $data)) {
            //the $data is already a SQL bit string.
            return true;
        }

        // 空字串處理，視其有無預設值
        if ($data == '') {
            return $this->default !== false;
        }

        if (empty($this->pattern))
            return true;

        // Is using a function to check?
        if (function_exists($this->pattern)) {
            $f = $this->pattern;
            //echo $f, "\n";
            return $f($data);
        }

        // Is using a method of class to check?
        // like 'Mydata::checkPin'
        if (strpos($this->pattern, '::') > 0) { 
            $m = array($this->pattern);
            return $this->coder($m, $data);
        }

        $pattern = isset(self::$PredefinedPatterns[$this->pattern])
            ? self::$PredefinedPatterns[$this->pattern]
            : $this->pattern;

        $r = preg_match('/'. $pattern .'/m', $data); // enable multiline
        if ($r === false)
            throw new Exception("An error occurred when invoke preg_match('/$pattern/m')");
        else if ($r === 0)
            return false;
        else 
            return true;
    }
    
    const Decoder = 'decoder';
    const Encoder = 'encoder';

    protected function coder($coder, &$data) {
        //$user->password->encode = 'Account::encryptPassword';
        // call_user_func(array('Account', 'encryptPassword'), $data);
        if (empty($this->$coder))
            return $data;

        foreach ($this->$coder as $encoder) {
            $method = explode('::', $encoder);
            if ($method[0] == 'Php')
                $data = $method[1]($data);
            else
                $data = call_user_func($method, $data);
        }
        return $data;
    }


    /**
     * Cascade encode.
     * It will cascade encode $data for stored in database.
     * @return mixed
     */
    public function encode($data) {
        if ( !$this->isValid($data) )
            return false;
        return $this->coder(self::Encoder, $data);
    }

    /**
     * Cascade decode.
     * It will cascade decode $data which fetched from database.
     * @return mixed
     */
    public function decode($data) {
        return $this->coder(self::Decoder, $data);
    }

} //end class SchemaTableColumn


/**
 * SchemaTable class
 * 
 * @package library
 * @subpackage database 
 * @version: $Revision: 136 $, $Date: 2008-07-09 20:40:39 +0800 (星期三, 09 七月 2008) $
 * @author: $Author: rock $  
 * @license: GNU LGPL
 */
class SchemaTable {
    /**
     * Table Attributes
     * The first char is '_' or '@' means attribute.
     *
     * @var array
     */
    protected $attributes = array();

    /**
     * Schema of Columns
     *
     * @var array
     */
    protected $columns = array();


    /**
     * Constructor.
     *
     * @param object|array $schema A struct of table schema.
     */
    function __construct($schema) {
        foreach ($schema as $name => $content) {
            // is $name like '_primaryKey' or '@primaryKey' ?
            if ($name[0] == '_' || $name[0] == '@') {
                $this->attributes[substr($name,1)] = $content;
            }
            else {
                $this->columns[$name] = new SchemaTableColumn($content);
            }
        }
    }
    
    /**
     * Magic get for attribute and column.
     * First char is '_': attribute.
     */
    public function  __get($key) {
        if ($key[0] == '_' ) { //|| $name[0] == '@'
            return $this->attribute($key);
        }
        return $this->column($key);

    }

    /**
     * Magic isset for attribute and column.
     * First char is '_': attribute.
     */
    public function  __isset($key) {
        if ($key[0] == '_' ) { //|| $name[0] == '@'
            $attrName = substr($key, 1);
            return isset($this->attributes[$attrName]);
        }
        return isset($this->columns[$key]);

    }

    /**
     * Get column's schema of table
     *
     * @param string $name of column
     * @return SchemaTableColumn|NullObject
     */
    public function column($name) {
        return (isset($this->columns[$name])
            ? $this->columns[$name]
            : new NullObject
        );
    }

    /**
     * Get/Set attribute of table
     *
     * @param string $name Name of attribute.
     * @param mixed $value Value of attribute to set.
     * @return mixed Value of attribute. If not set return NullObject.
     */              
    public function attribute($name, $value = NULL) {
        $attrName = (($name[0] == '_' || $name[0] == '@')
            ? substr($name, 1)
            : $name
        );

        if ($value === NULL) {
            return (isset($this->attributes[$attrName])
                ? $this->attributes[$attrName]
                : new NullObject
            );
        }
        else {
            $this->attributes[$attrName] = $value;
            return $value;
        }
    }

    /**
     * Get the name list of columns
     * @return array
     */
    public function columnList() {
        return array_keys($this->columns);
    }

    /**
     * Get the name list of columns
     * Alias of columnList().
     * @return array
     */
    public function fieldList() {
        return $this->columnList();
    }

    /**
     * Is all field's content valid?
     * <p>
     * If type of $schemaSet is string, it means name of table.
     * </p><p>
     * This method could be invoked as a class method (static method),                       
     * if the $schemaSet is already a set of schema returned from SchemaSet::get().
     * </p>
     *
     * @param array $data
     * @param array $ignores
     * @param array $invalids Invalid field's.
     * @return bool
     * @access public
     */         
    public function isAllValid(&$data, $ignores = false, &$invalids = false) {
        $invalids = array();
        foreach ($this->columns as $name => &$schema) {
            if ( $ignores and in_array($name, $ignores) )
                continue;
            if ( !$schema->isValid($data->$name) ) {
                //return false;
                $invalids[] = $name;
            }
        }
        return empty($invalids);
    }
} //end class SchemaTable

?>
