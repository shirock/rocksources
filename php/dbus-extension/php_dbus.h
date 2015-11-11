/* 
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2008 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
*/

/* $Id: php_dbus.h 297443 2010-04-04 01:05:09Z derick $ */

#ifndef PHP_DBUS_H
#define PHP_DBUS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Zend/zend_hash.h"

#define PHP_DBUS_VERSION "0.1.0"

extern zend_module_entry dbus_module_entry;
#define phpext_dbus_ptr &dbus_module_entry

PHP_METHOD(Dbus, __construct);
PHP_METHOD(Dbus, addWatch);
PHP_METHOD(Dbus, waitLoop);
PHP_METHOD(Dbus, requestName);
PHP_METHOD(Dbus, registerObject);
PHP_METHOD(Dbus, createProxy);

PHP_METHOD(DbusObject, __construct);
PHP_METHOD(DbusObject, __call);
PHP_METHOD(DbusSignal, __construct);
PHP_METHOD(DbusSignal, matches);
PHP_METHOD(DbusSignal, getData);
PHP_METHOD(DbusSignal, send);

PHP_METHOD(DbusArray, __construct);
PHP_METHOD(DbusArray, getData);
PHP_METHOD(DbusDict, __construct);
PHP_METHOD(DbusDict, getData);
PHP_METHOD(DbusVariant, __construct);
PHP_METHOD(DbusVariant, getData);
PHP_METHOD(DbusSet, __construct);
PHP_METHOD(DbusSet, getData);
PHP_METHOD(DbusStruct, __construct);
PHP_METHOD(DbusStruct, getData);
PHP_METHOD(DbusObjectPath, __construct);
PHP_METHOD(DbusObjectPath, getData);

#define PHP_DBUS_INT_WRAPPER_METHOD_DEF(t) \
	PHP_METHOD(Dbus##t, __construct);

PHP_DBUS_INT_WRAPPER_METHOD_DEF(Byte);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(Bool);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(Int16);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(UInt16);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(Int32);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(UInt32);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(Int64);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(UInt64);
PHP_DBUS_INT_WRAPPER_METHOD_DEF(Double);

PHP_RINIT_FUNCTION(dbus);
PHP_RSHUTDOWN_FUNCTION(dbus);
PHP_MINIT_FUNCTION(dbus);
PHP_MSHUTDOWN_FUNCTION(dbus);
PHP_MINFO_FUNCTION(dbus);

ZEND_BEGIN_MODULE_GLOBALS(dbus)
ZEND_END_MODULE_GLOBALS(dbus)

#ifdef ZTS
#define DBUSG(v) TSRMG(dbus_globals_id, zend_dbus_globals *, v)
#else
#define DBUSG(v) (dbus_globals.v)
#endif

#endif /* PHP_DBUS_H */
