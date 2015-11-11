<?php
/**
 * Database Helper class
 *
 * @package library
 * @subpackage database
 * @version: $Revision: 136 $, $Date: 2008-07-09 20:40:39 +0800 (星期三, 09 七月 2008) $
 * @author: $Author: rock $
 * @license: GNU LGPL
 */
class DatabaseHelper {
    /**
     * @see http://blog.roodo.com/rocksaying/archives/5935773.html
     */
    public static function quoteName($s) {
        if (strpos($s, '"') !== false)
            throw new Exception("The identifiers can not contain double quote character.");
        return '"' . $s . '"';
    }

    /**
     * Prepare data for making SQL string.
     *
     * @param string $key Field name
     * @param mixed $value Field value
     * @return string|false Return an escaped and quoted string of $value
     *  if $value is valided. False means an invalid value.
     */
    public static function prepareValue(&$db, &$columnSchema, $value) {
        //$columnSchema = $this->schema->column($key);

        // encode() will also valid data.
        if ( false === ($value = $columnSchema->encode($value)) )
            return false;

        // Is the $value not a validity SQL bit string?
        if (is_null($value)) {
            $value = 'NULL';
        }
        else if (/*$columnSchema->pattern == 'pattern_timestamp' and */$value == 'CURRENT_TIMESTAMP') {
            // TODO: How to write that simply?
            //$value = $value; //nothing
        }
        else if (0 == preg_match("/[xb]'[a-f0-9]*'/i", $value)) {
            /*
            if ($this->db and $this->db->platform() == 'pdo')
                $value = $this->db->conn_id->quote($value);
            else
            */
            $value = $db->quote($value);
        }

        return $value;
    }
}
?>
