<?php
require 'schema_table.php';

/**
 * Schema class
 *
 * It use JSON document to descript schema.
 *
 * @see http://blog.roodo.com/rocksaying/archives/5717961.html
 * @see http://blog.roodo.com/rocksaying/archives/2644922.html
 * @package library
 * @subpackage database
 * @version: $Revision: 53 $, $Date: 2012-11-06 15:31:51 +0800 (二, 06 11月 2012) $
 * @author: $Author: shirock.tw@gmail.com $
 * @license: GNU LGPL
 */
class Schema {
    /**
     * Schemas
     * @var array
     */
    protected $schemas = false;
    

    /**
     * Constructor, load schema set.
     * <p>
     * <pre>
     * {
     *   "tableName1": {
     *     "fieldName": {
     *       "type": "string",
     *       "pattern": false,
     *       "encoder": "Php::md5",
     *       "decoder": "DatabaseRowTest::maskPassword",
     *       "default": false,
     *       "notice": false
     *     },
     *     "fieldName": {
     *       "type": "string",
     *       "pattern": false,
     *       "encoder": "Php::md5",
     *       "decoder": "DatabaseRowTest::maskPassword",
     *       "default": false,
     *       "notice": false
     *     }
     *   },
     *   "tableName2": {
     *     "fieldName": {
     *       "type": "string",
     *       "pattern": false,
     *       "encoder": "Php::md5",
     *       "decoder": "DatabaseRowTest::maskPassword",
     *       "default": false,
     *       "notice": false
     *     }
     *   }
     * }
     * </pre>
     * </p>
     * <p>
     * The attributes of field could be ignored.
     * </p>
     *
     * @param string|array Path of schema file (default is 'db_schema.js'), or
     *        an array contains schemas.
     */
    public function __construct($schemaSource = false) {
        if (is_array($schemaSource) ) {
            $schemas = $schemaSource;
        }
        else {
            if ( !$schemaSource )
                $schemaSource = 'db_schema.js';

            $schemas = json_decode(file_get_contents($schemaSource));
        }
        if ($schemas === NULL) { // issue:2
            throw new UnexpectedValueException('The json cannot be decoded or if the encoded data is deeper than the recursion limit.');
        }
        foreach ($schemas as $tableName => $schema) {
            //Always assume the first char of table name is cap.
            //$this->schemas[ucfirst($tableName)] = new SchemaTable($schema);
            $this->schemas[$tableName] = new SchemaTable($schema);
        }
    }

    /**
     * Get schema of table
     *
     * @param string $tableName Name of table.
     * @return object All field's schema in the table.
     */
    public function table($tableName) {
        //$tableName = ucfirst($tableName);
        return (isset($this->schemas[$tableName])
            ? $this->schemas[$tableName]
            : new NullObject
        );
    }

    /**
     * Get schema of table
     *
     * Alias of table()
     *
     * @param string $tableName Name of table.
     * @return object All field's schema in the table.
     */
    public function __get($tableName) {
        return $this->table($tableName);
    }

    /**
     * Isset schema of table
     * @param $tableName
     * @return bool isset
     */
    public function __isset($tableName) {
        return isset($this->schemas[$tableName]);
    }
} //end class Schema
?>
