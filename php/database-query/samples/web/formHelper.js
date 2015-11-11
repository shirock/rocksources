/**
 * Form Helper class
 *
 * @package library
 * @subpackage database
 * @version: $Revision: 35 $, $Date: 2011-09-20 17:13:07 +0800 (二, 20  9月 2011) $
 * @author: $Author: shirock.tw@gmail.com $
 * @license: GNU LGPL
 */
function FormReset(){}
var formReset = new FormReset;

function getByName(n,t,b) {
    var bb = (b ? b : document);
    var tt = (t ? t : '*');
    var ts = bb.getElementsByTagName(tt);
    var ds = [], ids = 0;

    for(var i=0; i < ts.length; ++i)
        if(ts[i].name == n) 
            ds[ids++] = ts[i];
    return (ds.length ? ds : false);
}

/**
 * formValues :
 * {
 *   "tableName1": {
 *      "fieldName1": value,
 *      "fieldName2": value
 *   },
 *   "tableName2": {
 *      "fieldName1": value
 *   }
 * }
 */
FormReset.prototype.values = false;
FormReset.prototype.exec = function (formValues) {
    if ( !formValues )
        formValues = this.values;

    for (var tableName in formValues) {
        var form = document.getElementById(tableName);
        for (var fieldName in formValues[tableName]) {
            var elm = getByName(fieldName, '*', form);
            if (elm) {
                for (var i = 0; i < elm.length; ++i) 
                    elm[i].value = formValues[tableName][fieldName];
            }
        }
    
        for (var fieldName in formValues[tableName]) {
            var elm = document.getElementById(tableName + '[' + fieldName + ']');
            if (elm)
                elm.value = formValues[tableName][fieldName];
        }
    }
}

FormReset.prototype.fill = function (formID, formValues) {
    if ( !formValues )
        formValues = this.values;

    var form = document.getElementById(formID);
    for (var fieldName in formValues) {
        var elm = getByName(fieldName, '*', form);
        if (elm) {
            for (var i = 0; i < elm.length; ++i) 
                elm[i].value = formValues[fieldName];
        }
    }
}

//end of function FormReset


function FormChecker(schema, callback) {
    this.callback = (callback ? callback : false);
    this.schema = (schema ? schema : false);
}

var formChecker = new FormChecker;

//FormChecker.prototype.schema = false;
//FormChecker.prototype.callback = false;

FormChecker.prototype.parseFieldId = function (id) {
    var matches = id.match(/^(\w+)\[(\w+)\]$/);
    if (matches)
        return {tableName: matches[1], fieldName: matches[2]};
    else
        return false;
}

FormChecker.prototype.fieldCheck = function (fieldId, tableName, formSchema) {
    var predefinedPatterns = {
        'ctype_digit'   : '^\\d*$',
        'ctype_alnum'   : '^\\w*$',
        'ctype_alpha'   : '^[a-zA-Z]*$',
        'pattern_email' : '^[\\w\\.]+@[\\w\\.]+$',
        'pattern_ip'    : '^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$',
        'pattern_domain_name'   : '[\\w\\.]+',
        'pattern_timestamp'     : '^\\d{2,4}-\\d{2}-\\d{2}(\\s\\d{2}:\\d{2}:\\d{2})?(\\s[\\w\\+]+)?$'
    };

    if ( !formSchema )
        formSchema = this.schema;

    var schemaNameParts = this.parseFieldId(fieldId);
    var tableName;
    var fieldName;
    if (schemaNameParts) {
        tableName = schemaNameParts.tableName;
        fieldName = schemaNameParts.fieldName;
    }
    else {
        tableName = tableName;
        fieldName = fieldId;
    }

    if ( !formSchema || !formSchema[tableName] || !formSchema[tableName][fieldName] )
        return true;

    //if (value = document.getElementById(fieldId))
    //    value = value.value;
    var elm, value;
//    alert(fieldId);
    elm = document.getElementById(fieldId);

    if (!elm)
        return true; // 無此欄, 略過檢查.

    if ( !elm && (formSchema[tableName][fieldName]['default'] !== false) ) {
        //IGNORE //alert('no ' + formField);
        return true;
    }
    else {
        value = elm.value;
    }

    // 空字串處理，視其有無預設值
    if (value == '') {
        return formSchema[tableName][fieldName]['default'] !== false;
    }
    /*
    //非 'string' type 之欄位，若無預設值，則不允許輸入空字串
    if (value == ''
        && formSchema[tableName][fieldName]['type'] != 'string'
        && formSchema[tableName][fieldName]['default'] === false)
    {
        return false;
    }
    */

    var pattern = formSchema[tableName][fieldName].pattern;

    if (pattern == false) //no pattern, nothing to check.
        return true;

    if (predefinedPatterns[pattern])
        pattern = predefinedPatterns[pattern];

    var patternObject = RegExp(pattern);
    return patternObject.test(value);
}

FormChecker.prototype.submitCheck = function (formSchema, callback) {
    if ( !formSchema )
        formSchema = this.schema;
    if ( !callback )
        callback = this.callback;

    var fieldId;
    var allValid = true;

    for (var tableName in formSchema) {
        for (var fieldName in formSchema[tableName]) {
            if (fieldName.charAt(0) == '_')
                continue;
            fieldId = tableName + '[' + fieldName + ']';

            if ( !this.fieldCheck(fieldId) ) {
                formSchema[tableName][fieldName].validness = false;
                allValid = false
            }
            else {
                formSchema[tableName][fieldName].validness = true;
            }
            if (callback)
                callback(fieldId, formSchema[tableName][fieldName].validness);
        } //end for (fieldName in formSchema[tableName])

        for (var fieldName in formSchema[tableName]) {
            if (fieldName.charAt(0) == '_')
                continue;
            fieldId = fieldName;

            if ( !this.fieldCheck(fieldId, tableName) ) {
                formSchema[tableName][fieldName].validness = false;
                allValid = false
            }
            else {
                formSchema[tableName][fieldName].validness = true;
            }
            if (callback)
                callback(fieldId, formSchema[tableName][fieldName].validness);
        } //end for (fieldName in formSchema[tableName])

    } //end for (tableName in formSchema)
//    alert('allValid: ' + allValid);
    return allValid;
}
//end of var formChecker
