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

/* $Id$ */

#include "php.h"
#include "php_main.h"
#include "php_globals.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_versioning.h"
#include "php_dbus.h"
#include "zend_extensions.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "zend_hash.h"
#include "dbus/dbus.h"
#include "libxml/parser.h"
#include "libxml/parserInternals.h"
#include "introspect.h"

#ifndef Z_ADDREF_P
#define Z_ADDREF_P(z) ((z)->refcount++)
#endif

#if PHP_MINOR_VERSION > 3
# define INIT_OBJ_PROP	object_properties_init(&intern->std, class_type);
#else
# define INIT_OBJ_PROP	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_dbus_object___call, 0, 0, 2)
	ZEND_ARG_INFO(0, function_name)
	ZEND_ARG_INFO(0, arguments)
ZEND_END_ARG_INFO()
/* }}} */

const zend_function_entry dbus_funcs_dbus[] = {
	PHP_ME(Dbus, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(Dbus, addWatch,    NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Dbus, waitLoop,    NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Dbus, requestName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Dbus, registerObject, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Dbus, createProxy, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_object[] = {
	PHP_ME(DbusObject, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PRIVATE)
	PHP_ME(DbusObject, __call,      arginfo_dbus_object___call, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_signal[] = {
	PHP_ME(DbusSignal, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusSignal, matches,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(DbusSignal, getData,     NULL, ZEND_ACC_PUBLIC)
	PHP_ME(DbusSignal, send,        NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_array[] = {
	PHP_ME(DbusArray, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusArray, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_dict[] = {
	PHP_ME(DbusDict, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusDict, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

#define PHP_DBUS_INT_WRAPPER_DEF(s,t) \
	const zend_function_entry dbus_funcs_dbus_##s[] = { \
		PHP_ME(Dbus##t, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC) \
		{NULL, NULL, NULL} \
	};

PHP_DBUS_INT_WRAPPER_DEF(byte, Byte);
PHP_DBUS_INT_WRAPPER_DEF(bool, Bool);
PHP_DBUS_INT_WRAPPER_DEF(int16, Int16);
PHP_DBUS_INT_WRAPPER_DEF(uint16,UInt16);
PHP_DBUS_INT_WRAPPER_DEF(int32, Int32);
PHP_DBUS_INT_WRAPPER_DEF(uint32,UInt32);
PHP_DBUS_INT_WRAPPER_DEF(int64, Int64);
PHP_DBUS_INT_WRAPPER_DEF(uint64,UInt64);
PHP_DBUS_INT_WRAPPER_DEF(double, Double);

const zend_function_entry dbus_funcs_dbus_variant[] = {
	PHP_ME(DbusVariant, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusVariant, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_set[] = {
	PHP_ME(DbusSet, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusSet, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_struct[] = {
	PHP_ME(DbusStruct, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusStruct, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

const zend_function_entry dbus_funcs_dbus_object_path[] = {
	PHP_ME(DbusObjectPath, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(DbusObjectPath, getData,     NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

#define PHP_DBUS_CALL_FUNCTION     1
#define PHP_DBUS_RETURN_FUNCTION   2

static void dbus_register_classes(TSRMLS_D);
static zval * dbus_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC);
static int php_dbus_handle_reply(zval *return_value, DBusMessage *msg, int always_array TSRMLS_DC);
static int php_dbus_append_parameters(DBusMessage *msg, zval *data, xmlNode *inXml, int type TSRMLS_DC);
static int php_dbus_fetch_child_type(zval *child TSRMLS_DC);
/* }}} */

ZEND_DECLARE_MODULE_GLOBALS(dbus)
static PHP_GINIT_FUNCTION(dbus);

/* {{{ INI Settings */
PHP_INI_BEGIN()
PHP_INI_END()
/* }}} */

#if PHP_VERSION_ID < 50300
#define dbus_set_error_handling php_set_error_handling
#else
static inline void dbus_set_error_handling(error_handling_t error_handling, zend_class_entry *exception_class TSRMLS_DC)
{
	    zend_replace_error_handling(error_handling, exception_class, NULL TSRMLS_CC);
}
#endif

zend_class_entry *dbus_ce_dbus, *dbus_ce_dbus_object, *dbus_ce_dbus_signal;
zend_class_entry *dbus_ce_dbus_array, *dbus_ce_dbus_dict, *dbus_ce_dbus_variant;
zend_class_entry *dbus_ce_dbus_variant, *dbus_ce_dbus_set, *dbus_ce_dbus_struct;
zend_class_entry *dbus_ce_dbus_object_path;
zend_class_entry *dbus_ce_dbus_exception, *dbus_ce_dbus_exception_service, *dbus_ce_dbus_exception_method;

static zend_object_handlers dbus_object_handlers_dbus, dbus_object_handlers_dbus_object;
static zend_object_handlers dbus_object_handlers_dbus_signal;
static zend_object_handlers dbus_object_handlers_dbus_array, dbus_object_handlers_dbus_dict;
static zend_object_handlers dbus_object_handlers_dbus_variant, dbus_object_handlers_dbus_set;
static zend_object_handlers dbus_object_handlers_dbus_struct, dbus_object_handlers_dbus_object_path;

typedef struct _php_dbus_obj php_dbus_obj;
typedef struct _php_dbus_object_obj php_dbus_object_obj;
typedef struct _php_dbus_signal_obj php_dbus_signal_obj;
typedef struct _php_dbus_array_obj php_dbus_array_obj;
typedef struct _php_dbus_dict_obj php_dbus_dict_obj;
typedef struct _php_dbus_variant_obj php_dbus_variant_obj;
typedef struct _php_dbus_set_obj php_dbus_set_obj;
typedef struct _php_dbus_struct_obj php_dbus_struct_obj;
typedef struct _php_dbus_object_path_obj php_dbus_object_path_obj;
typedef struct _php_dbus_data_array php_dbus_data_array;

struct _php_dbus_obj {
	zend_object     std;
	DBusConnection *con;
	int             useIntrospection;
	HashTable       objects; /* A hash with all the registered objects that can be called */
};

struct _php_dbus_object_obj {
	zend_object      std;
	php_dbus_obj    *dbus;
	char            *destination;
	char            *path;
	char            *interface;
	xmlDocPtr        introspect_xml_doc;
    xmlNode         *introspect_xml;
};

#define PHP_DBUS_SIGNAL_IN  1
#define PHP_DBUS_SIGNAL_OUT 2

struct _php_dbus_signal_obj {
	zend_object      std;
	php_dbus_obj    *dbus;
	DBusMessage     *msg;
	char            *object;
	char            *interface;
	char            *signal;
	int              direction;
};

struct _php_dbus_array_obj {
	zend_object      std;
	long             type;
	char            *signature;
	zval            *elements;
};

struct _php_dbus_dict_obj {
	zend_object      std;
	long             type;
	char            *signature;
	zval            *elements;
};

struct _php_dbus_variant_obj {
	zend_object      std;
	zval            *data;
};

struct _php_dbus_set_obj {
	zend_object      std;
	int              element_count;
	zval           **elements;
};

struct _php_dbus_struct_obj {
	zend_object      std;
	zval            *elements;
};

struct _php_dbus_object_path_obj {
	zend_object      std;
	char            *path;
};

#define PHP_DBUS_SETUP_TYPE_OBJ(t,dt) \
	zend_class_entry *dbus_ce_dbus_##t; \
	static zend_object_handlers dbus_object_handlers_dbus_##t; \
	typedef struct _php_dbus_##t##_obj php_dbus_##t##_obj; \
	struct _php_dbus_##t##_obj { \
		zend_object      std; \
		dt               data; \
	};

PHP_DBUS_SETUP_TYPE_OBJ(byte,unsigned char);
PHP_DBUS_SETUP_TYPE_OBJ(bool,dbus_bool_t);
PHP_DBUS_SETUP_TYPE_OBJ(int16,dbus_int16_t);
PHP_DBUS_SETUP_TYPE_OBJ(uint16,dbus_uint16_t);
PHP_DBUS_SETUP_TYPE_OBJ(int32,dbus_int32_t);
PHP_DBUS_SETUP_TYPE_OBJ(uint32,dbus_uint32_t);
PHP_DBUS_SETUP_TYPE_OBJ(int64,dbus_int64_t);
PHP_DBUS_SETUP_TYPE_OBJ(uint64,dbus_uint64_t);
PHP_DBUS_SETUP_TYPE_OBJ(double,double);

struct _php_dbus_data_array {
	int count;
	void **data;
	int size;
};

#define DBUS_SET_CONTEXT \
	zval *object; \
	object = getThis(); \
   
#define DBUS_FETCH_OBJECT	\
	php_date_obj *obj;	\
	DBUS_SET_CONTEXT; \
	if (object) {	\
		if (zend_parse_parameters_none() == FAILURE) {	\
			return;	\
		}	\
	} else {	\
		if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, NULL, "O", &object, dbus_ce_dbus) == FAILURE) {	\
			RETURN_FALSE;	\
		}	\
	}	\
	obj = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);	\

#define DBUS_CHECK_INITIALIZED(member, class_name) \
	if (!(member)) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The " #class_name " object has not been correctly initialized by its constructor"); \
		RETURN_FALSE; \
	}

static void dbus_object_free_storage_dbus(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_object(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_signal(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_array(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_dict(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_variant(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_set(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_struct(void *object TSRMLS_DC);
static void dbus_object_free_storage_dbus_object_path(void *object TSRMLS_DC);

static zend_object_value dbus_object_new_dbus(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_object(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_signal(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_array(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_dict(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_variant(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_set(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_struct(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value dbus_object_new_dbus_object_path(zend_class_entry *class_type TSRMLS_DC);

static HashTable *dbus_byte_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_bool_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_int16_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_uint16_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_int32_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_uint32_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_int64_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_uint64_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_double_get_properties(zval *object TSRMLS_DC);

static HashTable *dbus_array_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_dict_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_variant_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_set_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_struct_get_properties(zval *object TSRMLS_DC);
static HashTable *dbus_object_path_get_properties(zval *object TSRMLS_DC);

static int dbus_variant_initialize(php_dbus_variant_obj *dbusobj, zval *data TSRMLS_DC);

#define PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(t) \
	static void dbus_object_free_storage_dbus_##t(void *object TSRMLS_DC); \
	static zend_object_value dbus_object_new_dbus_##t(zend_class_entry *class_type TSRMLS_DC);

PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(byte);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(bool);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(int16);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(uint16);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(int32);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(uint32);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(int64);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(uint64);
PHP_DBUS_FORWARD_DECL_TYPE_FUNCS(double);

static zend_object_value dbus_object_clone_dbus(zval *this_ptr TSRMLS_DC);

static int dbus_object_compare_dbus(zval *d1, zval *d2 TSRMLS_DC);

/* This is need to ensure that session extension request shutdown occurs 1st, because it uses the dbus extension */ 
static const zend_module_dep dbus_deps[] = {
	{NULL, NULL, NULL}
};

#ifdef COMPILE_DL_DBUS
ZEND_GET_MODULE(dbus)
#endif

/* {{{ Module struct */
zend_module_entry dbus_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	dbus_deps,
	"dbus",                     /* extension name */
	NULL,                       /* function list */
	PHP_MINIT(dbus),            /* process startup */
	PHP_MSHUTDOWN(dbus),        /* process shutdown */
	PHP_RINIT(dbus),            /* request startup */
	PHP_RSHUTDOWN(dbus),        /* request shutdown */
	PHP_MINFO(dbus),            /* extension info */
	PHP_DBUS_VERSION,           /* extension version */
	PHP_MODULE_GLOBALS(dbus),   /* globals descriptor */
	PHP_GINIT(dbus),            /* globals ctor */
	NULL,                       /* globals dtor */
	NULL,                       /* post deactivate */
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */


/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(dbus)
{
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(dbus)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(dbus)
{
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(dbus)
{
	REGISTER_INI_ENTRIES();
	dbus_register_classes(TSRMLS_C);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(dbus)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(dbus)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Dbus support", "enabled");
	php_info_print_table_row(2, "Version", PHP_DBUS_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

#define PHP_DBUS_REGISTER_TYPE_CLASS(t,n) \
	{ \
		zend_class_entry ce_dbus_##t; \
		INIT_CLASS_ENTRY(ce_dbus_##t, n, dbus_funcs_dbus_##t); \
		ce_dbus_##t.create_object = dbus_object_new_dbus_##t; \
		dbus_ce_dbus_##t = zend_register_internal_class_ex(&ce_dbus_##t, NULL, NULL TSRMLS_CC); \
		memcpy(&dbus_object_handlers_dbus_##t, zend_get_std_object_handlers(), sizeof(zend_object_handlers)); \
		dbus_object_handlers_dbus_##t.get_properties = dbus_##t##_get_properties; \
	}

static void dbus_register_classes(TSRMLS_D)
{
	zend_class_entry ce_dbus, ce_dbus_object, ce_dbus_array, ce_dbus_dict;
	zend_class_entry ce_dbus_variant, ce_dbus_signal, ce_dbus_set, ce_dbus_struct;
	zend_class_entry ce_dbus_object_path;
	zend_class_entry ce_dbus_exception, ce_dbus_exception_service, ce_dbus_exception_method;

	INIT_CLASS_ENTRY(ce_dbus, "Dbus", dbus_funcs_dbus);
	ce_dbus.create_object = dbus_object_new_dbus;
	dbus_ce_dbus = zend_register_internal_class_ex(&ce_dbus, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus.clone_obj = dbus_object_clone_dbus;
	dbus_object_handlers_dbus.compare_objects = dbus_object_compare_dbus;

	zend_declare_class_constant_long(dbus_ce_dbus, "BYTE", sizeof("BYTE")-1, DBUS_TYPE_BYTE TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "BOOLEAN", sizeof("BOOLEAN")-1, DBUS_TYPE_BOOLEAN TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "INT16", sizeof("INT16")-1, DBUS_TYPE_INT16 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "UINT16", sizeof("UINT16")-1, DBUS_TYPE_UINT16 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "INT32", sizeof("INT32")-1, DBUS_TYPE_INT32 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "UINT32", sizeof("UINT32")-1, DBUS_TYPE_UINT32 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "INT64", sizeof("INT64")-1, DBUS_TYPE_INT64 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "UINT64", sizeof("UINT64")-1, DBUS_TYPE_UINT64 TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "DOUBLE", sizeof("DOUBLE")-1, DBUS_TYPE_DOUBLE TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "STRING", sizeof("STRING")-1, DBUS_TYPE_STRING TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "ARRAY", sizeof("ARRAY")-1, DBUS_TYPE_ARRAY TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "VARIANT", sizeof("VARIANT")-1, DBUS_TYPE_VARIANT TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "STRUCT", sizeof("STRUCT")-1, DBUS_TYPE_STRUCT TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "OBJECT_PATH", sizeof("OBJECT_PATH")-1, DBUS_TYPE_OBJECT_PATH TSRMLS_CC);

	zend_declare_class_constant_long(dbus_ce_dbus, "BUS_SESSION", sizeof("BUS_SESSION")-1, DBUS_BUS_SESSION TSRMLS_CC);
	zend_declare_class_constant_long(dbus_ce_dbus, "BUS_SYSTEM", sizeof("BUS_SYSTEM")-1, DBUS_BUS_SYSTEM TSRMLS_CC);

	INIT_CLASS_ENTRY(ce_dbus_exception, "DbusException", NULL);
	dbus_ce_dbus_exception = zend_register_internal_class_ex(&ce_dbus_exception, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	dbus_ce_dbus_exception->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce_dbus_exception_service, "DbusExceptionServiceUnknown", NULL);
	dbus_ce_dbus_exception_service = zend_register_internal_class_ex(&ce_dbus_exception_service, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	dbus_ce_dbus_exception_service->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce_dbus_exception_method, "DbusExceptionUnknownMethod", NULL);
	dbus_ce_dbus_exception_method = zend_register_internal_class_ex(&ce_dbus_exception_method, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
	dbus_ce_dbus_exception_method->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce_dbus_object, "DbusObject", dbus_funcs_dbus_object);
	ce_dbus_object.create_object = dbus_object_new_dbus_object;
	dbus_ce_dbus_object = zend_register_internal_class_ex(&ce_dbus_object, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_object, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	INIT_CLASS_ENTRY(ce_dbus_signal, "DbusSignal", dbus_funcs_dbus_signal);
	ce_dbus_signal.create_object = dbus_object_new_dbus_signal;
	dbus_ce_dbus_signal = zend_register_internal_class_ex(&ce_dbus_signal, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_signal, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	INIT_CLASS_ENTRY(ce_dbus_array, "DbusArray", dbus_funcs_dbus_array);
	ce_dbus_array.create_object = dbus_object_new_dbus_array;
	dbus_ce_dbus_array = zend_register_internal_class_ex(&ce_dbus_array, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_array, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_array.get_properties = dbus_array_get_properties;

	INIT_CLASS_ENTRY(ce_dbus_dict, "DbusDict", dbus_funcs_dbus_dict);
	ce_dbus_dict.create_object = dbus_object_new_dbus_dict;
	dbus_ce_dbus_dict = zend_register_internal_class_ex(&ce_dbus_dict, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_dict, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_dict.get_properties = dbus_dict_get_properties;

	INIT_CLASS_ENTRY(ce_dbus_variant, "DbusVariant", dbus_funcs_dbus_variant);
	ce_dbus_variant.create_object = dbus_object_new_dbus_variant;
	dbus_ce_dbus_variant = zend_register_internal_class_ex(&ce_dbus_variant, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_variant, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_variant.get_properties = dbus_variant_get_properties;

	INIT_CLASS_ENTRY(ce_dbus_set, "DbusSet", dbus_funcs_dbus_set);
	ce_dbus_set.create_object = dbus_object_new_dbus_set;
	dbus_ce_dbus_set = zend_register_internal_class_ex(&ce_dbus_set, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_set, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_set.get_properties = dbus_set_get_properties;

	INIT_CLASS_ENTRY(ce_dbus_struct, "DbusStruct", dbus_funcs_dbus_struct);
	ce_dbus_struct.create_object = dbus_object_new_dbus_struct;
	dbus_ce_dbus_struct = zend_register_internal_class_ex(&ce_dbus_struct, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_struct, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_struct.get_properties = dbus_struct_get_properties;

	INIT_CLASS_ENTRY(ce_dbus_object_path, "DbusObjectPath", dbus_funcs_dbus_object_path);
	ce_dbus_object_path.create_object = dbus_object_new_dbus_object_path;
	dbus_ce_dbus_object_path = zend_register_internal_class_ex(&ce_dbus_object_path, NULL, NULL TSRMLS_CC);
	memcpy(&dbus_object_handlers_dbus_object_path, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	dbus_object_handlers_dbus_object_path.get_properties = dbus_object_path_get_properties;

	PHP_DBUS_REGISTER_TYPE_CLASS(byte, "DbusByte");
	PHP_DBUS_REGISTER_TYPE_CLASS(bool, "DbusBool");
	PHP_DBUS_REGISTER_TYPE_CLASS(int16, "DbusInt16");
	PHP_DBUS_REGISTER_TYPE_CLASS(uint16, "DbusUInt16");
	PHP_DBUS_REGISTER_TYPE_CLASS(int32, "DbusInt32");
	PHP_DBUS_REGISTER_TYPE_CLASS(uint32, "DbusUInt32");
	PHP_DBUS_REGISTER_TYPE_CLASS(int64, "DbusInt64");
	PHP_DBUS_REGISTER_TYPE_CLASS(uint64, "DbusUInt64");
	PHP_DBUS_REGISTER_TYPE_CLASS(double, "DbusDouble");
}

static inline zend_object_value dbus_object_new_dbus_ex(zend_class_entry *class_type, php_dbus_obj **ptr TSRMLS_DC)
{
	php_dbus_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_obj));
	memset(intern, 0, sizeof(php_dbus_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_ex(class_type, NULL TSRMLS_CC);
}

static zend_object_value dbus_object_clone_dbus(zval *this_ptr TSRMLS_DC)
{
	php_dbus_obj *new_obj = NULL;
	php_dbus_obj *old_obj = (php_dbus_obj *) zend_object_store_get_object(this_ptr TSRMLS_CC);
	zend_object_value new_ov = dbus_object_new_dbus_ex(old_obj->std.ce, &new_obj TSRMLS_CC);
	
	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);
	
	return new_ov;
}

static int dbus_object_compare_dbus(zval *d1, zval *d2 TSRMLS_DC)
{
	return 0;
}

static void dbus_object_free_storage_dbus(void *object TSRMLS_DC)
{
	php_dbus_obj *intern = (php_dbus_obj *)object;

	if (intern->con) {
		dbus_connection_unref(intern->con);
	}
	if (intern->objects.arBuckets) {
		zend_hash_destroy(&intern->objects);
	}

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS Object*/
static inline zend_object_value dbus_object_new_dbus_object_ex(zend_class_entry *class_type, php_dbus_object_obj **ptr TSRMLS_DC)
{
	php_dbus_object_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_object_obj));
	memset(intern, 0, sizeof(php_dbus_object_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_object, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_object;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_object(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_object_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_object(void *object TSRMLS_DC)
{
	php_dbus_object_obj *intern = (php_dbus_object_obj *)object;

	xmlFreeDoc(intern->introspect_xml_doc);
	if (intern->destination) {
		efree(intern->destination);
	}
	if (intern->path) {
		efree(intern->path);
	}
	if (intern->interface) {
		efree(intern->interface);
	}
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS Signal */
static inline zend_object_value dbus_object_new_dbus_signal_ex(zend_class_entry *class_type, php_dbus_signal_obj **ptr TSRMLS_DC)
{
	php_dbus_signal_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_signal_obj));
	memset(intern, 0, sizeof(php_dbus_signal_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_signal, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_signal;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_signal(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_signal_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_signal(void *object TSRMLS_DC)
{
	php_dbus_signal_obj *intern = (php_dbus_signal_obj *)object;

	dbus_message_unref(intern->msg);

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	
	if (intern->interface) {
		efree(intern->interface);
	}
	if (intern->signal) {
		efree(intern->signal);
	}
	if (intern->object) {
		efree(intern->object);
	}
	efree(object);
}

/* DBUS Array*/
static inline zend_object_value dbus_object_new_dbus_array_ex(zend_class_entry *class_type, php_dbus_array_obj **ptr TSRMLS_DC)
{
	php_dbus_array_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_array_obj));
	memset(intern, 0, sizeof(php_dbus_array_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_array, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_array;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_array(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_array_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_array(void *object TSRMLS_DC)
{
	php_dbus_array_obj *intern = (php_dbus_array_obj *)object;

	if (intern->signature) {
		efree(intern->signature);
	}
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS DICT */
static inline zend_object_value dbus_object_new_dbus_dict_ex(zend_class_entry *class_type, php_dbus_dict_obj **ptr TSRMLS_DC)
{
	php_dbus_dict_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_dict_obj));
	memset(intern, 0, sizeof(php_dbus_dict_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_dict, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_dict;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_dict(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_dict_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_dict(void *object TSRMLS_DC)
{
	php_dbus_dict_obj *intern = (php_dbus_dict_obj *)object;

	if (intern->signature) {
		efree(intern->signature);
	}
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS VARIANT */
static inline zend_object_value dbus_object_new_dbus_variant_ex(zend_class_entry *class_type, php_dbus_variant_obj **ptr TSRMLS_DC)
{
	php_dbus_variant_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_variant_obj));
	memset(intern, 0, sizeof(php_dbus_variant_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_variant, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_variant;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_variant(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_variant_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_variant(void *object TSRMLS_DC)
{
	php_dbus_variant_obj *intern = (php_dbus_variant_obj *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS SET */
static inline zend_object_value dbus_object_new_dbus_set_ex(zend_class_entry *class_type, php_dbus_set_obj **ptr TSRMLS_DC)
{
	php_dbus_set_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_set_obj));
	memset(intern, 0, sizeof(php_dbus_set_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_set, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_set;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_set(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_set_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_set(void *object TSRMLS_DC)
{
	php_dbus_set_obj *intern = (php_dbus_set_obj *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	
	if (intern->elements) {
		int i;
		
		for (i = 0; i < intern->element_count; i++) {
			if (intern->elements[i]) {
				zval_ptr_dtor(&intern->elements[i]);
			}
		}
		efree(intern->elements);
	}
	
	efree(object);
}

/* DBUS Struct*/
static inline zend_object_value dbus_object_new_dbus_struct_ex(zend_class_entry *class_type, php_dbus_struct_obj **ptr TSRMLS_DC)
{
	php_dbus_struct_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_struct_obj));
	memset(intern, 0, sizeof(php_dbus_struct_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_struct, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_struct;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_struct(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_struct_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_struct(void *object TSRMLS_DC)
{
	php_dbus_struct_obj *intern = (php_dbus_struct_obj *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

/* DBUS Object Path*/
static inline zend_object_value dbus_object_new_dbus_object_path_ex(zend_class_entry *class_type, php_dbus_object_path_obj **ptr TSRMLS_DC)
{
	php_dbus_object_path_obj *intern;
	zend_object_value retval;
	zval *tmp;

	intern = emalloc(sizeof(php_dbus_object_path_obj));
	memset(intern, 0, sizeof(php_dbus_object_path_obj));
	if (ptr) {
		*ptr = intern;
	}
	
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	INIT_OBJ_PROP;
	
	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_object_path, NULL TSRMLS_CC);
	retval.handlers = &dbus_object_handlers_dbus_object_path;
	
	return retval;
}

static zend_object_value dbus_object_new_dbus_object_path(zend_class_entry *class_type TSRMLS_DC)
{
	return dbus_object_new_dbus_object_path_ex(class_type, NULL TSRMLS_CC);
}

static void dbus_object_free_storage_dbus_object_path(void *object TSRMLS_DC)
{
	php_dbus_object_path_obj *intern = (php_dbus_object_path_obj *)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(object);
}

#define PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(t,pt,pf) \
	static inline zend_object_value dbus_object_new_dbus_##t##_ex(zend_class_entry *class_type, php_dbus_##t##_obj **ptr TSRMLS_DC) \
	{ \
		php_dbus_##t##_obj *intern; \
		zend_object_value retval; \
		zval *tmp; \
 \
		intern = emalloc(sizeof(php_dbus_##t##_obj)); \
		memset(intern, 0, sizeof(php_dbus_##t##_obj)); \
		if (ptr) { \
			*ptr = intern; \
		} \
		 \
		zend_object_std_init(&intern->std, class_type TSRMLS_CC); \
		INIT_OBJ_PROP; \
		 \
		retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) dbus_object_free_storage_dbus_##t, NULL TSRMLS_CC); \
		retval.handlers = &dbus_object_handlers_dbus_##t; \
		 \
		return retval; \
	} \
 \
	static zend_object_value dbus_object_new_dbus_##t(zend_class_entry *class_type TSRMLS_DC) \
	{ \
		return dbus_object_new_dbus_##t##_ex(class_type, NULL TSRMLS_CC); \
	} \
 \
	static void dbus_object_free_storage_dbus_##t(void *object TSRMLS_DC) \
	{ \
		php_dbus_##t##_obj *intern = (php_dbus_##t##_obj *)object; \
 \
		zend_object_std_dtor(&intern->std TSRMLS_CC); \
		efree(object); \
	} \
 \
	static HashTable *dbus_##t##_get_properties(zval *object TSRMLS_DC) \
	{ \
		HashTable *props; \
		zval *zv; \
		php_dbus_##t##_obj *intern; \
 \
		intern = (php_dbus_##t##_obj *) zend_object_store_get_object(object TSRMLS_CC); \
		MAKE_STD_ZVAL(zv); \
		Z_TYPE_P(zv) = pt; \
		(*zv).value.pf = intern->data; \
 \
		props = intern->std.properties; \
 \
		zend_hash_update(props, "value", 6, (void*)&zv, sizeof(zval *), NULL); \
 \
		return props; \
	}


PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(byte,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(bool,IS_BOOL,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(int16,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(uint16,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(int32,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(uint32,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(int64,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(uint64,IS_LONG,lval);
PHP_DBUS_DEFINE_TYPE_OBJ_FUNCS(double,IS_DOUBLE,dval);

/* Advanced Interface */
static zval * dbus_instantiate(zend_class_entry *pce, zval *object TSRMLS_DC)
{
	if (!object) {
		ALLOC_ZVAL(object);
	}

	Z_TYPE_P(object) = IS_OBJECT;
	object_init_ex(object, pce);
#ifdef Z_SET_REFCOUNT_P
	Z_SET_REFCOUNT_P(object, 1);
	Z_UNSET_ISREF_P(object);
#else
	object->refcount = 1;
	object->is_ref = 0;
#endif
	return object;
}

void dbus_registered_object_dtor(void *ptr)
{
}

static int dbus_initialize(php_dbus_obj *dbusobj, int type, int introspect TSRMLS_DC)
{
	DBusError err;
	DBusConnection *con;
	dbus_error_init(&err);

	con = dbus_bus_get(type, &err);
	if (dbus_error_is_set(&err)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	dbusobj->con = con;
	dbusobj->useIntrospection = introspect;
	zend_hash_init(&dbusobj->objects, 16, NULL, dbus_registered_object_dtor, 0);

	return 1;
}


/* {{{ proto Dbus::__construct([int type[, int useIntrospection]])
   Creates new Dbus object
*/
PHP_METHOD(Dbus, __construct)
{
	long type = DBUS_BUS_SESSION;
	long introspect = 0;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll", &type, &introspect)) {
		dbus_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), type, introspect TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

static int dbus_object_initialize(php_dbus_object_obj *dbusobj, php_dbus_obj *dbus, char *destination, char *path, char *interface TSRMLS_DC)
{
	dbusobj->dbus = dbus;
	dbusobj->destination = estrdup(destination);
	dbusobj->path = estrdup(path);
	dbusobj->interface = interface ? estrdup(interface) : NULL;

	return 1;
}

static void php_dbus_introspect(php_dbus_object_obj *dbusobj, php_dbus_obj* dbus, char *dest, char *path, char *interface TSRMLS_DC)
{
	DBusMessage     *msg;
	DBusMessageIter  iter;
	DBusPendingCall* pending;
	long             bool_false = 0;

	msg = dbus_message_new_method_call(dest, path, "org.freedesktop.DBus.Introspectable", "Introspect");
	dbus_message_iter_init_append(msg, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &bool_false);

	if (!dbus_connection_send_with_reply(dbus->con, msg, &pending, -1)) {
		dbus_message_unref(msg);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory.");
	}

	if (NULL == pending) { 
		dbus_message_unref(msg);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pending call null.");
	}
	dbus_connection_flush(dbus->con);

	/* free message */
	dbus_message_unref(msg);

	/* block until we recieve a reply */
	dbus_pending_call_block(pending);

	/* get the reply message */
	msg = dbus_pending_call_steal_reply(pending);

	if (msg == NULL || dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_METHOD_RETURN) {
		dbus_pending_call_unref(pending);
	} else {
		DBusMessageIter args;
		dbus_int64_t stat;

		if (!dbus_message_iter_init(msg, &args)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", dbus_message_get_error_name(msg));
			return;
		}
		dbus_message_iter_get_basic(&args, &stat);
		{
			dbusobj->introspect_xml_doc = xmlParseMemory((char*) stat, strlen((char*) stat));
			dbusobj->introspect_xml = php_dbus_find_interface_node(dbusobj->introspect_xml_doc, interface);
		}

		dbus_message_unref(msg);   
		dbus_pending_call_unref(pending);
	}
}

/* {{{ proto DbusObject::__construct(Dbus $dbus, string destination, string path, string interface)
   Creates new DbusObject object
*/
PHP_METHOD(DbusObject, __construct)
{
	zval *object;
	php_dbus_obj *dbus;
	char *destination, *path, *interface;
	int   destination_len, path_len, interface_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Osss!",
		&object, dbus_ce_dbus,
		&destination, &destination_len, &path, &path_len, 
		&interface, &interface_len))
	{
		Z_ADDREF_P(object);
		dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);
		dbus_object_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), dbus, destination, path, interface TSRMLS_CC);
		if (dbus->useIntrospection) {
			php_dbus_introspect(zend_object_store_get_object(getThis() TSRMLS_CC), dbus, destination, path, interface TSRMLS_CC);
		}
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto Dbus::createProxy(string destination, string path, string interface)
   Creates new DbusObject object
*/
PHP_METHOD(Dbus, createProxy)
{
	zval *object = getThis();
	php_dbus_obj *dbus;
	php_dbus_object_obj *dbus_object;
	char *destination, *path, *interface;
	int   destination_len, path_len, interface_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss!",
		&destination, &destination_len, &path, &path_len, 
		&interface, &interface_len))
	{
		dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);
		Z_ADDREF_P(object);
		dbus_instantiate(dbus_ce_dbus_object, return_value TSRMLS_CC);
		dbus_object = (php_dbus_object_obj *) zend_object_store_get_object(return_value TSRMLS_CC);
		dbus_object_initialize(dbus_object, dbus, destination, path, interface TSRMLS_CC);
		if (dbus->useIntrospection) {
			php_dbus_introspect(dbus_object, dbus, destination, path, interface TSRMLS_CC);
		}
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

static void php_dbus_accept_incoming_signal(DBusMessage *msg, zval **return_value TSRMLS_DC)
{
	php_dbus_signal_obj *signalobj;
	dbus_instantiate(dbus_ce_dbus_signal, *return_value TSRMLS_CC);
	signalobj = (php_dbus_signal_obj *) zend_object_store_get_object(*return_value TSRMLS_CC);
	signalobj->msg = msg;
	signalobj->direction = PHP_DBUS_SIGNAL_IN;
}

static void php_dbus_do_error_message(php_dbus_obj *dbus, DBusMessage *msg, char *type, const char *message)
{
	DBusMessage *reply;

	/* it's a different kind of method, so send the
	 * "unknown method" error */
	dbus_uint32_t serial = 0;
	reply = dbus_message_new_error(msg, type, message);
	dbus_connection_send(dbus->con, reply, &serial);
	dbus_connection_flush(dbus->con);
	dbus_message_unref(reply);
}

static void php_dbus_do_method_call(php_dbus_obj *dbus, DBusMessage *msg, char *class, const char *member TSRMLS_DC)
{
	HashTable *params_ar;
	int num_elems, element = 0;
	zval *params, ***method_args = NULL, *retval_ptr;
	zval *callback, *object = NULL;
	DBusMessage *reply;

	ALLOC_ZVAL(params);
	php_dbus_handle_reply(params, msg, 1 TSRMLS_CC);

	ALLOC_ZVAL(callback);
	array_init(callback);
	add_next_index_string(callback, class, 0);
	add_next_index_string(callback, member, 0);

	params_ar = HASH_OF(params);
	if (params_ar) {
		num_elems = zend_hash_num_elements(params_ar);
		method_args = (zval ***) safe_emalloc(sizeof(zval **), num_elems, 0);

		for (zend_hash_internal_pointer_reset(params_ar);
			zend_hash_get_current_data(params_ar, (void **) &(method_args[element])) == SUCCESS;
			zend_hash_move_forward(params_ar)
		) {
			element++;
		}
	} else {
		num_elems = 0;
		method_args = (zval ***) safe_emalloc(sizeof(zval **), num_elems, 0);
	}

	if (call_user_function_ex(EG(function_table), &object, callback, &retval_ptr, num_elems, method_args, 0, NULL TSRMLS_CC) == SUCCESS) {
		if (retval_ptr) {
			reply = dbus_message_new_method_return(msg);
			php_dbus_append_parameters(reply, retval_ptr, NULL, PHP_DBUS_RETURN_FUNCTION TSRMLS_CC);

			if (!dbus_connection_send(dbus->con, reply, NULL)) {
				dbus_message_unref(msg);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory.");
			}

			dbus_connection_flush(dbus->con);

			/* free message */
			dbus_message_unref(reply);
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call %s()", Z_STRVAL_P(callback));
	}
	efree(method_args);
}

static void php_dbus_accept_incoming_method_call(php_dbus_obj *dbus, DBusMessage *msg, zval **return_value TSRMLS_DC)
{
	char *key, *class, *interface;

	/* See if we can find a class mapping for this object */
	interface = dbus_message_get_interface(msg);
	spprintf(&key, 0, "%s:%s", dbus_message_get_path(msg), interface ? interface : "(null)");
	if (zend_hash_find(&(dbus->objects), key, strlen(key) + 1, (void**) &class) == SUCCESS) {
		zend_class_entry **pce;

		/* We found a registered class, now lets see if this class is actually available */
		if (zend_lookup_class(class, strlen(class), &pce TSRMLS_CC) == SUCCESS) {
			char *lcname;
			const char *member = dbus_message_get_member(msg);
			zend_class_entry *ce;

			ce = *pce;

			/* Now we have the class, we can see if the callback method exists */
			lcname = zend_str_tolower_dup(member, strlen(member));
			if (!zend_hash_exists(&ce->function_table, lcname, strlen(member) + 1)) {
				/* If no method is found, we try to see whether we
				 * can do some introspection stuff for our built-in
				 * classes. */
				if (strcmp("introspect", lcname) == 0) {
				} else {
					php_dbus_do_error_message(dbus, msg, DBUS_ERROR_UNKNOWN_METHOD, member);
					efree(lcname);
				}
			} else {
				php_dbus_do_method_call(dbus, msg, class, member TSRMLS_CC);
			}
		} else {
			php_dbus_do_error_message(dbus, msg, DBUS_ERROR_UNKNOWN_METHOD, "call back class not found");
		}
	} else {
		php_dbus_do_error_message(dbus, msg, DBUS_ERROR_UNKNOWN_METHOD, "call back class not registered");
	}
}

/* {{{ proto Dbus::waitLoop()
   Checks for received signals or method calls.
*/
PHP_METHOD(Dbus, waitLoop)
{
	zval *object;
	long timeout = 0;
	DBusMessage *msg;
	php_dbus_obj *dbus;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O|l", &object, dbus_ce_dbus, &timeout)) {
		RETURN_FALSE;
	}

	dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);

	dbus_connection_read_write(dbus->con, timeout);
	msg = dbus_connection_pop_message(dbus->con);

	if (msg != NULL) {
		switch (dbus_message_get_type(msg)) {
			case DBUS_MESSAGE_TYPE_SIGNAL:
				php_dbus_accept_incoming_signal(msg, &return_value TSRMLS_CC);
				break;
			case DBUS_MESSAGE_TYPE_METHOD_CALL:
				php_dbus_accept_incoming_method_call(dbus, msg, &return_value TSRMLS_CC);
				break;
		}
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto Dbus::requestName(string name)
   Requests a name to be assigned to this Dbus connection.
*/
PHP_METHOD(Dbus, requestName)
{
	zval *object;
	DBusError err;
	int ret;
	char* name;
	int name_len;
	php_dbus_obj *dbus;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"Os", &object, dbus_ce_dbus, &name, &name_len)) {
		RETURN_FALSE;
	}

	dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);
	dbus_error_init(&err);

	/* request our name on the bus and check for errors */
	ret = dbus_bus_request_name(dbus->con, name, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not Primary Owner (%d)\n", ret);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto Dbus::registerObject(string path, string interface, string class)
   Registers a callback class for the path, and with the specified interface
*/
PHP_METHOD(Dbus, registerObject)
{
	zval *object;
	char *path, *interface, *class;
	int path_len, interface_len, class_len;
	php_dbus_obj *dbus;
	char *key;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"Oss!s", &object, dbus_ce_dbus, &path, &path_len, &interface, &interface_len,
		&class, &class_len)) {
		RETURN_FALSE;
	}
	dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);

	/* Create the key out of the path and interface */
	spprintf(&key, 0, "%s:%s", path, interface ? interface : "(null)");
	if (interface == NULL) {
		interface_len = 6;
	}

	/* Add class name to hash */
	zend_hash_add(&(dbus->objects), key, path_len + interface_len + 2, (void*) estrdup(class), strlen(class)+1, NULL);
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */


static int dbus_append_var(zval **val, php_dbus_data_array *data_array, DBusMessageIter *iter, char *type_hint TSRMLS_DC);

static int dbus_append_var_php_array(php_dbus_data_array *data_array, DBusMessageIter *iter, zval *array TSRMLS_DC)
{
	DBusMessageIter listiter;
	char type_string[2];
	int type = DBUS_TYPE_STRING;
	zval **entry;

	/* First we find the first element, to find the type that we need to set.
	 * We'll only consider elements of the type that the first element of the
	 * array has */
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	if (zend_hash_get_current_data(Z_ARRVAL_P(array), (void **)&entry) == SUCCESS) {
		type = php_dbus_fetch_child_type(*entry TSRMLS_CC);
	}

	type_string[0] = (char) type;
	type_string[1] = '\0';

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, type_string, &listiter);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	while (zend_hash_get_current_data(Z_ARRVAL_P(array), (void **)&entry) == SUCCESS) {
		if (php_dbus_fetch_child_type(*entry TSRMLS_CC) == type) {
			dbus_append_var(entry, data_array, &listiter, NULL TSRMLS_CC);
		}
		zend_hash_move_forward(Z_ARRVAL_P(array));
	}
	dbus_message_iter_close_container(iter, &listiter);

	return 1;
}

static int dbus_append_var_array(php_dbus_data_array *data_array, DBusMessageIter *iter, php_dbus_array_obj *obj TSRMLS_DC)
{
	DBusMessageIter listiter;
	char *type_string;
	zval **entry;

	if (obj->signature) {
		type_string = ecalloc(1, 1 + strlen(obj->signature));
		strcpy(type_string, obj->signature);
	} else {
		type_string = emalloc(2);
		type_string[0] = (char) obj->type;
		type_string[1] = '\0';
	}

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, type_string, &listiter);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(obj->elements));
	while (zend_hash_get_current_data(Z_ARRVAL_P(obj->elements), (void **)&entry) == SUCCESS) {
		if (php_dbus_fetch_child_type(*entry TSRMLS_CC) == obj->type) {
			dbus_append_var(entry, data_array, &listiter, NULL TSRMLS_CC);
		}
		zend_hash_move_forward(Z_ARRVAL_P(obj->elements));
	}
	dbus_message_iter_close_container(iter, &listiter);

	return 1;
}

static int dbus_append_var_dict(php_dbus_data_array *data_array, DBusMessageIter *iter, php_dbus_dict_obj *obj TSRMLS_DC)
{
	DBusMessageIter listiter, dictiter;
	char *type_string;
	zval **entry;
	char *key;
	uint key_len;
	ulong num_index;

	if (obj->signature) {
		if (strlen(obj->signature) < 4) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", "Signature should be at least four characters");
			return 0;
		}
		if (obj->signature[0] != '{' || obj->signature[strlen(obj->signature)-1] != '}') {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", "The signature needs to start with { and end with }");
			return 0;
		}
		if (obj->signature[1] != 's') {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", "Only string keys are supported so far");
		}

		type_string = ecalloc(1, 1 + strlen(obj->signature));
		strcpy(type_string, obj->signature);
	} else {
		type_string = emalloc(5);
		type_string[0] = '{';
		type_string[1] = 's';
		type_string[2] = (char) obj->type;
		type_string[3] = '}';
		type_string[4] = '\0';
	}

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, type_string, &listiter);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(obj->elements));
	while (zend_hash_get_current_data(Z_ARRVAL_P(obj->elements), (void **)&entry) == SUCCESS) {
		if (zend_hash_get_current_key_ex(Z_ARRVAL_P(obj->elements), &key, &key_len, &num_index, 0, NULL) == HASH_KEY_IS_STRING) {
			dbus_message_iter_open_container(&listiter, DBUS_TYPE_DICT_ENTRY, NULL, &dictiter );
			dbus_message_iter_append_basic(&dictiter, DBUS_TYPE_STRING, &key);
			dbus_append_var(entry, data_array, &dictiter, NULL TSRMLS_CC);
			dbus_message_iter_close_container(&listiter, &dictiter);
		}
		zend_hash_move_forward(Z_ARRVAL_P(obj->elements));
	}

	dbus_message_iter_close_container(iter, &listiter);

	return 1;
}

#define PHP_DBUS_MARSHAL_FIND_TYPE_CASE(t,s) \
	if (obj->ce == dbus_ce_dbus_##t) { \
		return DBUS_TYPE_##s; \
	}

static int php_dbus_fetch_child_type(zval *child TSRMLS_DC)
{
	zend_object     *obj;

	switch (Z_TYPE_P(child)) {
		case IS_BOOL:
			return DBUS_TYPE_BOOLEAN;
		case IS_LONG:
			return DBUS_TYPE_INT32;
		case IS_DOUBLE:
			return DBUS_TYPE_DOUBLE;
		case IS_STRING:
			return DBUS_TYPE_STRING;
		case IS_OBJECT:
			/* We need to check for DbusArray, DbusDict and DbusVariant and the DBus types */
			obj = (zend_object *) zend_object_store_get_object(child TSRMLS_CC);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(variant, VARIANT);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(array, ARRAY);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(struct, STRUCT);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(object_path, OBJECT_PATH);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(byte, BYTE);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(bool, BOOLEAN);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(int16, INT16);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(uint16, UINT16);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(int32, INT32);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(uint32, UINT32);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(int64, INT64);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(uint64, UINT64);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE(double, DOUBLE);
	}

	return IS_NULL;
}

#define PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(t,s) \
	if (obj->ce == dbus_ce_dbus_##t) { \
		return DBUS_TYPE_##s##_AS_STRING; \
	}

static char* php_dbus_fetch_child_type_as_string(zval *child TSRMLS_DC)
{
	zend_object     *obj;

	switch (Z_TYPE_P(child)) {
		case IS_BOOL:
			return DBUS_TYPE_BOOLEAN_AS_STRING;
		case IS_LONG:
			return DBUS_TYPE_INT32_AS_STRING;
		case IS_DOUBLE:
			return DBUS_TYPE_DOUBLE_AS_STRING;
		case IS_STRING:
			return DBUS_TYPE_STRING_AS_STRING;
		case IS_OBJECT:
			/* We need to check for DbusArray, DbusDict and DbusVariant and the DBus types */
			obj = (zend_object *) zend_object_store_get_object(child TSRMLS_CC);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(variant, VARIANT);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(array, ARRAY);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(struct, STRUCT);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(object_path, OBJECT_PATH);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(byte, BYTE);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(bool, BOOLEAN);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(int16, INT16);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(uint16, UINT16);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(int32, INT32);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(uint32, UINT32);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(int64, INT64);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(uint64, UINT64);
			PHP_DBUS_MARSHAL_FIND_TYPE_CASE_AS_STRING(double, DOUBLE);
	}

	return NULL;
}

static int dbus_append_var_variant(php_dbus_data_array *data_array, DBusMessageIter *iter, php_dbus_variant_obj *obj TSRMLS_DC)
{
	DBusMessageIter variant;
	char *type;

	type = php_dbus_fetch_child_type_as_string(obj->data TSRMLS_CC);
	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, type, &variant );
	dbus_append_var(&(obj->data), data_array, &variant, NULL TSRMLS_CC);
	dbus_message_iter_close_container(iter, &variant);

	return 1;
}

static int dbus_append_var_set(php_dbus_data_array *data_array, DBusMessageIter *iter, php_dbus_set_obj *obj TSRMLS_DC)
{
	int   i;

	for (i = 0; i < obj->element_count; i++) {
		dbus_append_var(&(obj->elements[i]), data_array, iter, NULL TSRMLS_CC);
	}

	return 1;
}

static int dbus_append_var_struct(php_dbus_data_array *data_struct, DBusMessageIter *iter, php_dbus_struct_obj *obj TSRMLS_DC)
{
	DBusMessageIter listiter;
	zval **entry;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(obj->elements));
	dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, NULL, &listiter);
	while (zend_hash_get_current_data(Z_ARRVAL_P(obj->elements), (void **)&entry) == SUCCESS) {
		dbus_append_var(entry, data_struct, &listiter, NULL TSRMLS_CC);

		zend_hash_move_forward(Z_ARRVAL_P(obj->elements));
	}
	dbus_message_iter_close_container(iter, &listiter);

	return 1;
}

static int dbus_append_var_object_path(php_dbus_data_array *data_struct, DBusMessageIter *iter, php_dbus_object_path_obj *obj TSRMLS_DC)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &obj->path);

	return 1;
}

#define PHP_DBUS_APPEND_TYPE_FUNC(t,dt) \
	static int dbus_append_var_##t(DBusMessageIter *iter, long val) \
	{ \
		dbus_message_iter_append_basic(iter, DBUS_TYPE_##dt, &val); \
		return 1; \
	}

PHP_DBUS_APPEND_TYPE_FUNC(byte, BYTE);
PHP_DBUS_APPEND_TYPE_FUNC(bool, BOOLEAN);
PHP_DBUS_APPEND_TYPE_FUNC(double, DOUBLE);
PHP_DBUS_APPEND_TYPE_FUNC(int16, INT16);
PHP_DBUS_APPEND_TYPE_FUNC(uint16, UINT16);
PHP_DBUS_APPEND_TYPE_FUNC(int32, INT32);
PHP_DBUS_APPEND_TYPE_FUNC(uint32, UINT32);
PHP_DBUS_APPEND_TYPE_FUNC(int64, INT64);
PHP_DBUS_APPEND_TYPE_FUNC(uint64, UINT64);

static void** php_dbus_get_data_ptr(php_dbus_data_array *d)
{
	d->count++;
	if (d->count == d->size) {
		d->data = erealloc(d->data, (64 + d->size) * sizeof(void**));
		d->size += 64;
	}
	return &d->data[d->count-1];
}

#define PHP_DBUS_MARSHAL_TO_DBUS_CASE(t) \
	if (obj->ce == dbus_ce_dbus_##t) { \
		dbus_append_var_##t(iter, ((php_dbus_##t##_obj*) obj)->data); \
	}

static int dbus_append_var(zval **val, php_dbus_data_array *data_array, DBusMessageIter *iter, char *type_hint TSRMLS_DC)
{
	zend_object     *obj;
	void           **data_copy;

	switch (Z_TYPE_PP(val)) {
		case IS_BOOL:
			data_copy = php_dbus_get_data_ptr(data_array);
			*data_copy = emalloc(sizeof(dbus_bool_t));
			**(dbus_bool_t**)data_copy = (dbus_bool_t) Z_LVAL_PP(val);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, *data_copy);
			break;
		case IS_LONG: {
			char type = type_hint ? (char)type_hint[0] : 0;
			switch (type)
			{
				case 'u':
					data_copy = php_dbus_get_data_ptr(data_array);
					*data_copy = emalloc(sizeof(dbus_uint32_t));
					**(dbus_uint32_t**)data_copy = (dbus_uint32_t) Z_LVAL_PP(val);
					dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, *data_copy);
					break;

				default:
					data_copy = php_dbus_get_data_ptr(data_array);
					*data_copy = emalloc(sizeof(dbus_int32_t));
					**(dbus_int32_t**)data_copy = (dbus_int32_t) Z_LVAL_PP(val);
					dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, *data_copy);
			}
			break;
		}
		case IS_DOUBLE:
			data_copy = php_dbus_get_data_ptr(data_array);
			*data_copy = emalloc(sizeof(double));
			**(double**)data_copy = (double) Z_DVAL_PP(val);
			dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, *data_copy);
			break;
		case IS_STRING:
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &Z_STRVAL_PP(val));
			break;
		case IS_ARRAY:
			dbus_append_var_php_array(data_array, iter, *val TSRMLS_CC);
			break;
		case IS_OBJECT:
			/* We need to check for DbusArray, DbusDict, DbusVariant, DbusSet and the DBus types */
			obj = (zend_object *) zend_object_store_get_object(*val TSRMLS_CC);
			if (obj->ce == dbus_ce_dbus_array) {
				dbus_append_var_array(data_array, iter, (php_dbus_array_obj*) obj TSRMLS_CC);
			}
			if (obj->ce == dbus_ce_dbus_dict) {
				dbus_append_var_dict(data_array, iter, (php_dbus_dict_obj*) obj TSRMLS_CC);
			}
			if (obj->ce == dbus_ce_dbus_variant) {
				dbus_append_var_variant(data_array, iter, (php_dbus_variant_obj*) obj TSRMLS_CC);
			}
			if (obj->ce == dbus_ce_dbus_set) {
				dbus_append_var_set(data_array, iter, (php_dbus_set_obj*) obj TSRMLS_CC);
			}
			if (obj->ce == dbus_ce_dbus_struct) {
				dbus_append_var_struct(data_array, iter, (php_dbus_struct_obj*) obj TSRMLS_CC);
			}
			if (obj->ce == dbus_ce_dbus_object_path) {
				dbus_append_var_object_path(data_array, iter, (php_dbus_object_path_obj*) obj TSRMLS_CC);
			}
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(byte);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(bool);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(int16);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(uint16);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(int32);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(uint32);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(int64);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(uint64);
			PHP_DBUS_MARSHAL_TO_DBUS_CASE(double);
	}

	return 1;
}

static int php_dbus_append_parameters(DBusMessage *msg, zval *data, xmlNode *inXml, int type TSRMLS_DC)
{
	DBusMessageIter dbus_args;
	HashPosition pos;
	zval **entry;
	php_dbus_data_array data_array;
	int i;
	xmlNode **it = NULL;

	if (inXml) {
		it = &inXml->children;
	}

	data_array.count = 0;
	data_array.data = emalloc(64 * sizeof(void**));
	data_array.size = 64;
	dbus_message_iter_init_append(msg, &dbus_args);

	if (type == PHP_DBUS_CALL_FUNCTION) {
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(data), &pos);
		while (zend_hash_get_current_data_ex(Z_ARRVAL_P(data), (void **)&entry, &pos) == SUCCESS) {
			char *sig = NULL;
			if (it) {
				it = php_dbus_get_next_sig(it, &sig);
			}
			dbus_append_var(entry, &data_array, &dbus_args, it ? sig : NULL TSRMLS_CC);
			zend_hash_move_forward_ex(Z_ARRVAL_P(data), &pos);
		}
	} else if (type == PHP_DBUS_RETURN_FUNCTION) {
		dbus_append_var(&data, &data_array, &dbus_args, NULL TSRMLS_CC);
	}

	for (i = 0; i < data_array.count; i++) {
		efree(data_array.data[i]);
	}
	efree(data_array.data);

	return 1;
}

static zval* php_dbus_to_zval(DBusMessageIter *args, zval **key TSRMLS_DC)
{
	zval *return_value;
	DBusMessageIter subiter;
	int arg_type = 0;
	
	*key = NULL;
	MAKE_STD_ZVAL(return_value);
	arg_type = dbus_message_iter_get_arg_type(args);
	switch (arg_type) {
		case DBUS_TYPE_ARRAY:
			{
				int init = 0;

				dbus_message_iter_recurse(args, &subiter);
				do {
					zval *new_key = NULL;
					php_dbus_dict_obj *dictobj;
					php_dbus_array_obj *arrayobj;
					zval *val = php_dbus_to_zval(&subiter, &new_key TSRMLS_CC);

					if (new_key) {
						if (!init) {
							dbus_instantiate(dbus_ce_dbus_dict, return_value TSRMLS_CC);
							dictobj = (php_dbus_dict_obj*) zend_object_store_get_object(return_value TSRMLS_CC);
							dictobj->type = php_dbus_fetch_child_type(val TSRMLS_CC);
							MAKE_STD_ZVAL(dictobj->elements);
							array_init(dictobj->elements);
							init = 1;
						}

						if (val && Z_TYPE_P(new_key) == IS_STRING) {
							add_assoc_zval_ex(dictobj->elements, Z_STRVAL_P(new_key), Z_STRLEN_P(new_key) + 1, val);
						} else if (val && Z_TYPE_P(new_key) == IS_LONG) {
							add_index_zval(dictobj->elements, Z_LVAL_P(new_key), val);
						}
					} else {
						if (!init) {
							dbus_instantiate(dbus_ce_dbus_array, return_value TSRMLS_CC);
							arrayobj = (php_dbus_array_obj*) zend_object_store_get_object(return_value TSRMLS_CC);
							arrayobj->type = val ? php_dbus_fetch_child_type(val TSRMLS_CC) : DBUS_TYPE_INVALID;
							MAKE_STD_ZVAL(arrayobj->elements);
							array_init(arrayobj->elements);
							arrayobj->signature = estrdup(dbus_message_iter_get_signature(&subiter));
							init = 1;
						}

						if (val) {
							add_next_index_zval(arrayobj->elements, val);
						}
					}
				} while (dbus_message_iter_next(&subiter));
			}
			break;
		case DBUS_TYPE_DICT_ENTRY:
			{
				zval *new_key = NULL;

				dbus_message_iter_recurse(args, &subiter);
					
				*key = php_dbus_to_zval(&subiter, &new_key TSRMLS_CC);
				dbus_message_iter_next(&subiter);
				return_value = php_dbus_to_zval(&subiter, &new_key TSRMLS_CC);
			}
			break;
		case DBUS_TYPE_VARIANT:
			{
				zval *new_key = NULL, *val;
				php_dbus_variant_obj *variantobj;

				dbus_message_iter_recurse(args, &subiter);
				val = php_dbus_to_zval(&subiter, &new_key TSRMLS_CC);

				dbus_instantiate(dbus_ce_dbus_variant, return_value TSRMLS_CC);
				variantobj = (php_dbus_variant_obj*) zend_object_store_get_object(return_value TSRMLS_CC);
				variantobj->data = val;
			}
			break;
		case DBUS_TYPE_STRUCT:
			{
				php_dbus_struct_obj *structobj;

				dbus_instantiate(dbus_ce_dbus_struct, return_value TSRMLS_CC);
				structobj = (php_dbus_struct_obj*) zend_object_store_get_object(return_value TSRMLS_CC);
				MAKE_STD_ZVAL(structobj->elements);
				array_init(structobj->elements);

				dbus_message_iter_recurse(args, &subiter);
				do {
					zval *new_key = NULL;
					zval *val = php_dbus_to_zval(&subiter, &new_key TSRMLS_CC);
					add_next_index_zval(structobj->elements, val);
				} while (dbus_message_iter_next(&subiter));
			}
			break;
		case DBUS_TYPE_OBJECT_PATH:
			{
				php_dbus_object_path_obj *object_pathobj;
				dbus_int64_t stat;

				dbus_instantiate(dbus_ce_dbus_object_path, return_value TSRMLS_CC);
				object_pathobj = (php_dbus_object_path_obj*) zend_object_store_get_object(return_value TSRMLS_CC);
				dbus_message_iter_get_basic(args, &stat);
				object_pathobj->path = estrdup((char*) stat);
			}
			break;
		case 0:
			return NULL;
			break;
		default:
			if (dbus_message_iter_get_arg_type(args) == DBUS_TYPE_DOUBLE) {
				double stat;
				dbus_message_iter_get_basic(args, &stat);
				RETVAL_DOUBLE((double) stat);
			} else {
				dbus_int64_t stat;
				dbus_message_iter_get_basic(args, &stat);
				switch (dbus_message_iter_get_arg_type(args)) {
					case DBUS_TYPE_BOOLEAN: RETVAL_BOOL((dbus_bool_t) stat);  break;
					case DBUS_TYPE_BYTE:    RETVAL_LONG((unsigned char) stat); break;
					case DBUS_TYPE_INT16:   RETVAL_LONG((dbus_int16_t) stat);  break;
					case DBUS_TYPE_UINT16:  RETVAL_LONG((dbus_uint16_t) stat); break;
					case DBUS_TYPE_INT32:   RETVAL_LONG((dbus_int32_t) stat);  break;
					case DBUS_TYPE_UINT32:  RETVAL_LONG((dbus_uint32_t) stat); break;
					case DBUS_TYPE_INT64:   RETVAL_LONG((dbus_int64_t) stat);  break;
					case DBUS_TYPE_UINT64:  RETVAL_LONG((dbus_uint64_t) stat); break;
					case DBUS_TYPE_STRING:
						RETVAL_STRING((char*) stat, 1);
						break;
				}
			}
	}
	return return_value;
}

static int php_dbus_handle_reply(zval *return_value, DBusMessage *msg, int always_array TSRMLS_DC)
{
	zend_class_entry *exception_ce = NULL;
	DBusMessageIter   args;
	dbus_int64_t      stat;
	zval             *val;

	if (dbus_message_get_type(msg) == DBUS_MESSAGE_TYPE_ERROR) {
		const char* error_msg_str = dbus_message_get_error_name(msg);

		if (!strcmp(error_msg_str, DBUS_ERROR_SERVICE_UNKNOWN)) {
			exception_ce = dbus_ce_dbus_exception_service;
		} else if (!strcmp(error_msg_str, DBUS_ERROR_UNKNOWN_METHOD)) {
			exception_ce = dbus_ce_dbus_exception_method;
		} else {
			exception_ce = dbus_ce_dbus_exception;
		}

		dbus_set_error_handling(EH_THROW, exception_ce TSRMLS_CC);
		if (!dbus_message_iter_init(msg, &args)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", dbus_message_get_error_name(msg));
			return 0;
		}
		dbus_message_iter_get_basic(&args, &stat);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s: %s", dbus_message_get_error_name(msg), (char*) stat);
		dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
	}

	ZVAL_NULL(return_value);
	// No return values
	if (!dbus_message_iter_init(msg, &args)) {
		return 0;
	}

	// if we're setting up a method call to be called by PHP, we just need a normal array
	if (always_array) {
		array_init(return_value);
		do {
			zval *key = NULL;
			val = php_dbus_to_zval(&args, &key TSRMLS_CC);
			add_next_index_zval(return_value, val);
		} while (dbus_message_iter_next(&args));
	} else {
		if (dbus_message_iter_has_next(&args)) {
			// More than one return value
			php_dbus_set_obj *set;

			dbus_instantiate(dbus_ce_dbus_set, return_value TSRMLS_CC);
			set = (php_dbus_set_obj *) zend_object_store_get_object(return_value TSRMLS_CC);
			set->element_count = 0;
			set->elements = (zval **) safe_emalloc(sizeof(zval*), 64, 0);

			do {
				zval *key = NULL;
				val = php_dbus_to_zval(&args, &key TSRMLS_CC);

				set->elements[set->element_count] = val;
				set->element_count++;
			} while (dbus_message_iter_next(&args));
		} else {
			// Exactly one return value
			zval *key = NULL;
			val = php_dbus_to_zval(&args, &key TSRMLS_CC);
			*return_value = *val;
		}
	}

	return 1;
}

PHP_METHOD(DbusObject, __call)
{
	char *name;
	int name_len;
	zval *data;
	zval *object;
	php_dbus_object_obj *dbus_object;
	DBusMessage *msg;
	DBusPendingCall* pending;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"Osz", &object, dbus_ce_dbus_object, &name, &name_len, &data)) {
		RETURN_FALSE;
	}
	dbus_object = (php_dbus_object_obj *) zend_object_store_get_object(object TSRMLS_CC);

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);

	msg = dbus_message_new_method_call(dbus_object->destination, dbus_object->path, dbus_object->interface, name);
	php_dbus_append_parameters(msg, data, dbus_object->introspect_xml ? php_dbus_find_method_node(dbus_object->introspect_xml->children, name) : NULL, PHP_DBUS_CALL_FUNCTION TSRMLS_CC);

	/* send message and get a handle for a reply */
	if (!dbus_connection_send_with_reply(dbus_object->dbus->con, msg, &pending, -1)) {
		dbus_message_unref(msg);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory.");
	}

	if (NULL == pending) { 
		dbus_message_unref(msg);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Pending call null.");
	}
	dbus_connection_flush(dbus_object->dbus->con);

	/* free message */
	dbus_message_unref(msg);

	/* block until we recieve a reply */
	dbus_pending_call_block(pending);

	/* get the reply message */
	msg = dbus_pending_call_steal_reply(pending);

	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	if (msg == NULL) {
		dbus_pending_call_unref(pending);
		RETURN_NULL();
	} else {
		php_dbus_handle_reply(return_value, msg, 0 TSRMLS_CC);
		dbus_message_unref(msg);   
		dbus_pending_call_unref(pending);
	}
}

PHP_METHOD(Dbus, addWatch)
{
	char *interface = NULL, *member = NULL;
	int interface_len, member_len;
	zval *object;
	php_dbus_obj *dbus_object;
	DBusError err;
	char *match_str;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O|ss", &object, dbus_ce_dbus, &interface, &interface_len, &member, &member_len)) {
		RETURN_FALSE;
	}
	dbus_object = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);

	if (interface && member) {
		spprintf(&match_str, 0, "type='signal',interface='%s',member='%s'", interface, member);
	} else if (interface) {
		spprintf(&match_str, 0, "type='signal',interface='%s'", interface);
	} else {
		spprintf(&match_str, 0, "type='signal'");
	}

	dbus_error_init(&err);
	dbus_bus_add_match(dbus_object->con, match_str, &err);
	efree(match_str);
	dbus_connection_flush(dbus_object->con);
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
	if (dbus_error_is_set(&err)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Match error (%s)", err.message);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

static int dbus_signal_initialize(php_dbus_signal_obj *dbusobj, php_dbus_obj *dbus, char *object, char *interface, char *signal TSRMLS_DC)
{
	dbusobj->dbus = dbus;
	dbusobj->object = estrdup(object);
	dbusobj->interface = estrdup(interface);
	dbusobj->signal = estrdup(signal);

	return 1;
}


/* {{{ proto DbusSignal::__construct(Dbus $dbus, string object, string interface, string signal)
   Creates new DbusSignal object
*/
PHP_METHOD(DbusSignal, __construct)
{
	zval *object;
	php_dbus_obj *dbus;
	char *object_name, *interface, *signal;
	int   object_name_len, interface_len, signal_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Osss",
		&object, dbus_ce_dbus,
		&object_name, &object_name_len, &interface, &interface_len, 
		&signal, &signal_len))
	{
		Z_ADDREF_P(object);
		dbus = (php_dbus_obj *) zend_object_store_get_object(object TSRMLS_CC);
		dbus_signal_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), dbus, object_name, interface, signal TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusSignal, matches)
{
	char *interface;
	int interface_len;
	char *method;
	int method_len;
	zval *object;
	php_dbus_signal_obj *signal_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"Oss", &object, dbus_ce_dbus_signal, &interface, &interface_len, &method, &method_len)) {
		RETURN_FALSE;
	}
	signal_obj = (php_dbus_signal_obj *) zend_object_store_get_object(object TSRMLS_CC);

	if (dbus_message_is_signal(signal_obj->msg, interface, method)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

PHP_METHOD(DbusSignal, getData)
{
	zval *object;
	php_dbus_signal_obj *signal_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_signal)) {
		RETURN_FALSE;
	}
	signal_obj = (php_dbus_signal_obj *) zend_object_store_get_object(object TSRMLS_CC);

	if (signal_obj->direction == PHP_DBUS_SIGNAL_OUT) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This signal is outgoing, and therefore does not have data.");
		RETURN_FALSE;
	}

	php_dbus_handle_reply(return_value, signal_obj->msg, 0 TSRMLS_CC);
}

PHP_METHOD(DbusSignal, send)
{
	zval ***data = NULL;
	int     elements = ZEND_NUM_ARGS();
	php_dbus_data_array data_array;
	php_dbus_signal_obj *signal_obj;
	DBusMessageIter dbus_args;
	int i;
	dbus_uint32_t serial = 0;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	signal_obj = (php_dbus_signal_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (signal_obj->direction == PHP_DBUS_SIGNAL_IN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This signal is incoming, and therefore can not be send.");
		RETURN_FALSE;
	}

	data = (zval ***) safe_emalloc(elements, sizeof(zval **), 1);
	if (FAILURE == zend_get_parameters_array_ex(elements, data)) {
		efree(data);
		return;
	}

	data_array.count = 0;
	data_array.data = emalloc(64 * sizeof(void**));
	data_array.size = 64;
	signal_obj->msg = dbus_message_new_signal(signal_obj->object, signal_obj->interface, signal_obj->signal);
	dbus_message_iter_init_append(signal_obj->msg, &dbus_args);

	for (i = 0; i < elements; i++) {
		dbus_append_var(data[i], &data_array, &dbus_args, NULL TSRMLS_CC);
	}

	/* send the message and flush the connection */
	if (!dbus_connection_send(signal_obj->dbus->con, signal_obj->msg, &serial)) {
		dbus_message_unref(signal_obj->msg);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Out of memory.");
	}
	dbus_connection_flush(signal_obj->dbus->con);

	for (i = 0; i < data_array.count; i++) {
		efree(data_array.data[i]);
	}
	efree(data_array.data);
	efree(data);

	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}

static int dbus_array_initialize(php_dbus_array_obj *dbusobj, long type, zval *elements, char *signature TSRMLS_DC)
{
	dbusobj->type = type;
	Z_ADDREF_P(elements);
	dbusobj->elements = elements;
	dbusobj->signature = signature ? estrdup(signature) : NULL;

	return 1;
}

static HashTable *dbus_array_get_properties(zval *object TSRMLS_DC)
{
	HashTable *props;
	php_dbus_array_obj *array_obj;
	zval *sig;

	array_obj = (php_dbus_array_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = array_obj->std.properties;

	if (array_obj->signature) {
		MAKE_STD_ZVAL(sig);
		ZVAL_STRING(sig, array_obj->signature, 1);
		zend_hash_update(props, "signature", 10, (void*)&sig, sizeof(zval *), NULL);
	}

	zend_hash_update(props, "array", 6, (void*)&array_obj->elements, sizeof(zval *), NULL);
	Z_ADDREF_P(array_obj->elements);

	return props;
}

/* {{{ proto DbusArray::__construct(int $type, array $elements [, string signature] )
   Creates new DbusArray object
*/
PHP_METHOD(DbusArray, __construct)
{
	long  type;
	zval *array;
	char *signature = NULL;
	int   signature_len;

#warning test whether signature is present when type == struct

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la|s", &type, &array, &signature, &signature_len)) {
		dbus_array_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), type, array, signature TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusArray, getData)
{
	zval *object;
	php_dbus_array_obj *array_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_array)) {
		RETURN_FALSE;
	}
	array_obj = (php_dbus_array_obj *) zend_object_store_get_object(object TSRMLS_CC);
	*return_value = *array_obj->elements;
	zval_copy_ctor(return_value);
}

static int dbus_dict_initialize(php_dbus_dict_obj *dbusobj, long type, zval *elements, char *signature TSRMLS_DC)
{
	dbusobj->type = type;
	Z_ADDREF_P(elements);
	dbusobj->elements = elements;
	dbusobj->signature = signature ? estrdup(signature) : NULL;

	return 1;
}

static HashTable *dbus_dict_get_properties(zval *object TSRMLS_DC)
{
	HashTable *props;
	php_dbus_dict_obj *dict_obj;
	zval *sig;

	dict_obj = (php_dbus_dict_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = dict_obj->std.properties;

	if (dict_obj->signature) {
		MAKE_STD_ZVAL(sig);
		ZVAL_STRING(sig, dict_obj->signature, 1);
		zend_hash_update(props, "signature", 10, (void*)&sig, sizeof(zval *), NULL);
	}

	zend_hash_update(props, "dict", 5, (void*)&dict_obj->elements, sizeof(zval *), NULL);
	Z_ADDREF_P(dict_obj->elements);

	return props;
}

/* {{{ proto DbusDict::__construct(int $type, dict $elements)
   Creates new DbusDict object
*/
PHP_METHOD(DbusDict, __construct)
{
	long  type;
	zval *dict;
	char *signature = NULL;
	int   signature_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "la|s", &type, &dict, &signature, &signature_len)) {
		dbus_dict_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), type, dict, signature TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusDict, getData)
{
	zval *object;
	php_dbus_dict_obj *dict_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_dict)) {
		RETURN_FALSE;
	}
	dict_obj = (php_dbus_dict_obj *) zend_object_store_get_object(object TSRMLS_CC);
	*return_value = *dict_obj->elements;
	zval_copy_ctor(return_value);
}

static int dbus_variant_initialize(php_dbus_variant_obj *dbusobj, zval *data TSRMLS_DC)
{
	Z_ADDREF_P(data);
	dbusobj->data = data;
	return 1;
}

static HashTable *dbus_variant_get_properties(zval *object TSRMLS_DC)
{
	HashTable *props;
	php_dbus_variant_obj *variant_obj;

	variant_obj = (php_dbus_variant_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = variant_obj->std.properties;

	zend_hash_update(props, "variant", 8, (void*)&variant_obj->data, sizeof(zval *), NULL);
	Z_ADDREF_P(variant_obj->data);

	return props;
}

/* {{{ proto DbusVariant::__construct(mixed $data)
   Creates new DbusVariant object
*/
PHP_METHOD(DbusVariant, __construct)
{
	zval *data;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &data)) {
		dbus_variant_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), data TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusVariant, getData)
{
	zval *object;
	php_dbus_variant_obj *variant_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_variant)) {
		RETURN_FALSE;
	}
	variant_obj = (php_dbus_variant_obj *) zend_object_store_get_object(object TSRMLS_CC);
	*return_value = *variant_obj->data;
	zval_copy_ctor(return_value);
}

static int dbus_set_initialize(php_dbus_set_obj *dbusobj, zval ***data, int elements TSRMLS_DC)
{
	int i;

	dbusobj->elements = ecalloc(sizeof(zval *), elements);
	dbusobj->element_count = elements;

	for (i = 0; i < elements; i++) {
		dbusobj->elements[i] = *data[i];
	}
	return 1;
}

static HashTable *dbus_set_get_properties(zval *object TSRMLS_DC)
{
	int i;
	HashTable *props;
	php_dbus_set_obj *set_obj;
	zval *set_contents;

	set_obj = (php_dbus_set_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = set_obj->std.properties;

	MAKE_STD_ZVAL(set_contents);
	array_init(set_contents);
	for (i = 0; i < set_obj->element_count; i++) {
		Z_ADDREF_P(set_obj->elements[i]);
		add_next_index_zval(set_contents, set_obj->elements[i]);
	}

	zend_hash_update(props, "set", 4, (void*)&set_contents, sizeof(zval*), NULL);

	return props;
}


/* {{{ proto DbusSet::__construct(mixed $data)
   Creates new DbusSet object
*/
PHP_METHOD(DbusSet, __construct)
{
	zval ***data = NULL;
	int     elements = ZEND_NUM_ARGS();

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	data = (zval ***) safe_emalloc(elements, sizeof(zval **), 1);
	if (SUCCESS == zend_get_parameters_array_ex(elements, data)) {
		dbus_set_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), data, elements TSRMLS_CC);
	}
	efree(data);
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusSet, getData)
{
	int i;
	zval *object;
	php_dbus_set_obj *set_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_set)) {
		RETURN_FALSE;
	}
	set_obj = (php_dbus_set_obj *) zend_object_store_get_object(object TSRMLS_CC);
	array_init(return_value);
	for (i = 0; i < set_obj->element_count; i++) {
		Z_ADDREF_P(set_obj->elements[i]);
		add_next_index_zval(return_value, set_obj->elements[i]);
	}
}

static int dbus_struct_initialize(php_dbus_struct_obj *dbusobj, char *signature, zval *elements TSRMLS_DC)
{
	Z_ADDREF_P(elements);
	dbusobj->elements = elements;

	return 1;
}

static HashTable *dbus_struct_get_properties(zval *object TSRMLS_DC)
{
	HashTable *props;
	php_dbus_struct_obj *struct_obj;

	struct_obj = (php_dbus_struct_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = struct_obj->std.properties;

	zend_hash_update(props, "struct", 7, (void*)&struct_obj->elements, sizeof(zval *), NULL);
	Z_ADDREF_P(struct_obj->elements);

	return props;
}

/* {{{ proto DbusStruct::__construct(string $signature, struct $elements)
   Creates new DbusStruct object
*/
PHP_METHOD(DbusStruct, __construct)
{
	zval *array;
	char *signature;
	int   signature_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &signature, &signature_len, &array)) {
		dbus_struct_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), signature, array TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusStruct, getData)
{
	zval *object;
	php_dbus_struct_obj *struct_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_struct)) {
		RETURN_FALSE;
	}
	struct_obj = (php_dbus_struct_obj *) zend_object_store_get_object(object TSRMLS_CC);
	*return_value = *struct_obj->elements;
	zval_copy_ctor(return_value);
}

static int dbus_object_path_initialize(php_dbus_object_path_obj *dbusobj, char *path TSRMLS_DC)
{
	dbusobj->path = estrdup(path);

	return 1;
}

static HashTable *dbus_object_path_get_properties(zval *object TSRMLS_DC)
{
	HashTable *props;
	php_dbus_object_path_obj *object_path_obj;
	zval *path;

	object_path_obj = (php_dbus_object_path_obj *) zend_object_store_get_object(object TSRMLS_CC);

	props = object_path_obj->std.properties;

	MAKE_STD_ZVAL(path);
	ZVAL_STRING(path, object_path_obj->path, 1);

	zend_hash_update(props, "path", 5, (void*)&path, sizeof(zval*), NULL);

	return props;
}

/* {{{ proto DbusObjectPath::__construct(string $path)
   Creates new DbusObjectPath object
*/
PHP_METHOD(DbusObjectPath, __construct)
{
	zval *array;
	char *path;
	int   path_len;

	dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC);
	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len)) {
		dbus_object_path_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), path TSRMLS_CC);
	}
	dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
}
/* }}} */

PHP_METHOD(DbusObjectPath, getData)
{
	zval *object;
	php_dbus_object_path_obj *object_path_obj;

	if (FAILURE == zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(),
		"O", &object, dbus_ce_dbus_object_path)) {
		RETURN_FALSE;
	}
	object_path_obj = (php_dbus_object_path_obj *) zend_object_store_get_object(object TSRMLS_CC);

	RETURN_STRING(object_path_obj->path, 1);
}


#define PHP_DBUS_INT_WRAPPER(t,s,n) \
	static int dbus_##s##_initialize(php_dbus_##s##_obj *dbusobj, t data TSRMLS_DC) \
	{ \
		dbusobj->data = data; \
		return 1; \
	} \
	PHP_METHOD(n, __construct) \
	{ \
		long data; \
		dbus_set_error_handling(EH_THROW, NULL TSRMLS_CC); \
		if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &data)) { \
			dbus_##s##_initialize(zend_object_store_get_object(getThis() TSRMLS_CC), (t)data TSRMLS_CC); \
		} \
		dbus_set_error_handling(EH_NORMAL, NULL TSRMLS_CC); \
	}

PHP_DBUS_INT_WRAPPER(unsigned char,byte,  DbusByte);
PHP_DBUS_INT_WRAPPER(dbus_bool_t,  bool,  DbusBool);
PHP_DBUS_INT_WRAPPER(dbus_int16_t, int16, DbusInt16);
PHP_DBUS_INT_WRAPPER(dbus_uint16_t,uint16,DbusUInt16);
PHP_DBUS_INT_WRAPPER(dbus_int32_t, int32, DbusInt32);
PHP_DBUS_INT_WRAPPER(dbus_uint32_t,uint32,DbusUInt32);
PHP_DBUS_INT_WRAPPER(dbus_int64_t, int64, DbusInt64);
PHP_DBUS_INT_WRAPPER(dbus_uint64_t,uint64,DbusUInt64);
PHP_DBUS_INT_WRAPPER(double,       double,DbusDouble);
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
