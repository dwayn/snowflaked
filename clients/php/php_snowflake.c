/*
Copyright (c) 2014 Dwayn Matthies <dwayn dot matthies at gmail dot com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include <zend_exceptions.h>

static int le_snowflake_sock;
static zend_class_entry *snowflake_ce;
static zend_class_entry *snowflake_exception_ce;
static zend_class_entry *spl_ce_RuntimeException = NULL;

ZEND_DECLARE_MODULE_GLOBALS(snowflake)

static zend_function_entry snowflake_functions[] = {
	 PHP_ME(Snowflake, __construct, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Snowflake, connect, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Snowflake, close, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Snowflake, get, NULL, ZEND_ACC_PUBLIC)
	 PHP_ME(Snowflake, info, NULL, ZEND_ACC_PUBLIC)
	 PHP_MALIAS(Snowflake, open, connect, NULL, ZEND_ACC_PUBLIC)
	 {NULL, NULL, NULL}
};

zend_module_entry snowflake_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"snowflake",
	NULL,
	PHP_MINIT(snowflake),
	PHP_MSHUTDOWN(snowflake),
	PHP_RINIT(snowflake),
	PHP_RSHUTDOWN(snowflake),
	PHP_MINFO(snowflake),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_SNOWFLAKE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SNOWFLAKE
ZEND_GET_MODULE(snowflake)
#endif

void add_constant_long(zend_class_entry *ce, char *name, int value) {
	zval *constval;
	constval = pemalloc(sizeof(zval), 1);
	INIT_PZVAL(constval);
	ZVAL_LONG(constval, value);
	zend_hash_add(&ce->constants_table, name, 1 + strlen(name), (void*)&constval, sizeof(zval*), NULL);
}

/**
 * This command behave somehow like printf, except that strings need 2 arguments:
 * Their data and their size (strlen).
 * Supported formats are: %d, %i, %s
 */
static int snowflake_cmd_format(char **ret, char *format, ...) {
	char *p, *s;
	va_list ap;

	int total = 0, sz, ret_sz;
	int i, ci;
	unsigned int u;
	double dbl;
	char *double_str;
	int double_len;

	int stage;
	for (stage = 0; stage < 2; ++stage) {
		va_start(ap, format);
		total = 0;
		for (p = format; *p; ) {
			if (*p == '%') {
				switch (*(p+1)) {
					case 's':
						s = va_arg(ap, char*);
						sz = va_arg(ap, int);
						if (stage == 1) {
							memcpy((*ret) + total, s, sz);
						}
						total += sz;
						break;
					case 'f':
						/* use spprintf here */
						dbl = va_arg(ap, double);
						double_len = spprintf(&double_str, 0, "%f", dbl);
						if (stage == 1) {
							memcpy((*ret) + total, double_str, double_len);
						}
						total += double_len;
						efree(double_str);
						break;
					case 'i':
					case 'd':
						i = va_arg(ap, int);
						/* compute display size of integer value */
						sz = 0;
						ci = abs(i);
						while (ci>0) {
								ci = (ci/10);
								sz += 1;
						}
						if (i == 0) { /* log 0 doesn't make sense. */
								sz = 1;
						} else if(i < 0) { /* allow for neg sign as well. */
								sz++;
						}
						if (stage == 1) {
							sprintf((*ret) + total, "%d", i);
						} 
						total += sz;
						break;
				}
				p++;
			} else {
				if (stage == 1) {
					(*ret)[total] = *p;
				}
				total++;
			}
			p++;
		}
		if (stage == 0) {
			ret_sz = total;
			(*ret) = emalloc(ret_sz+1);
		} else {
			(*ret)[ret_sz] = 0;
			return ret_sz;
		}
	}
}

PHPAPI SnowflakeSock* snowflake_sock_create(char *host, int host_len, unsigned short port, long timeout) {
	SnowflakeSock *snowflake_sock;
	snowflake_sock = emalloc(sizeof *snowflake_sock);
	snowflake_sock->host = emalloc(host_len + 1);
	snowflake_sock->stream = NULL;
	snowflake_sock->status = SNOWFLAKE_SOCK_STATUS_DISCONNECTED;

	memcpy(snowflake_sock->host, host, host_len);
	snowflake_sock->host[host_len] = '\0';
	snowflake_sock->port = port;
	snowflake_sock->timeout = timeout;

	return snowflake_sock;
}

PHPAPI int snowflake_sock_connect(SnowflakeSock *snowflake_sock TSRMLS_DC) {
	struct timeval tv, *tv_ptr = NULL;
	char *host = NULL, *hash_key = NULL, *errstr = NULL;
	int host_len, err = 0;

	if (snowflake_sock->stream != NULL) {
		snowflake_sock_disconnect(snowflake_sock TSRMLS_CC);
	}

	tv.tv_sec  = snowflake_sock->timeout;
	tv.tv_usec = 0;

	host_len = spprintf(&host, 0, "%s:%d", snowflake_sock->host, snowflake_sock->port);

	if (tv.tv_sec != 0) {
		tv_ptr = &tv;
	}
	snowflake_sock->stream = php_stream_xport_create(host, host_len, ENFORCE_SAFE_MODE, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, hash_key, tv_ptr, NULL, &errstr, &err);

	efree(host);

	if (!snowflake_sock->stream) {
		efree(errstr);
		return -1;
	}

	php_stream_auto_cleanup(snowflake_sock->stream);

	if (tv.tv_sec != 0) {
		php_stream_set_option(snowflake_sock->stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
	}
	php_stream_set_option(snowflake_sock->stream, PHP_STREAM_OPTION_WRITE_BUFFER, PHP_STREAM_BUFFER_NONE, NULL);

	snowflake_sock->status = SNOWFLAKE_SOCK_STATUS_CONNECTED;
	return 0;
}

PHPAPI int snowflake_sock_server_open(SnowflakeSock *snowflake_sock, int force_connect TSRMLS_DC) {
	int res = -1;
	switch (snowflake_sock->status) {
		case SNOWFLAKE_SOCK_STATUS_DISCONNECTED:
			return snowflake_sock_connect(snowflake_sock TSRMLS_CC);
		case SNOWFLAKE_SOCK_STATUS_CONNECTED:
			res = 0;
		break;
		case SNOWFLAKE_SOCK_STATUS_UNKNOWN:
			if (force_connect > 0 && snowflake_sock_connect(snowflake_sock TSRMLS_CC) < 0) {
				res = -1;
			} else {
				res = 0;
				snowflake_sock->status = SNOWFLAKE_SOCK_STATUS_CONNECTED;
			}
		break;
	}
	return res;
}

PHPAPI int snowflake_sock_disconnect(SnowflakeSock *snowflake_sock TSRMLS_DC) {
	int res = 0;
	if (snowflake_sock->stream != NULL) {
		//snowflake_sock_write(snowflake_sock, "QUIT", sizeof("QUIT") - 1);
		snowflake_sock->status = SNOWFLAKE_SOCK_STATUS_DISCONNECTED;
		php_stream_close(snowflake_sock->stream);
		snowflake_sock->stream = NULL;
		res = 1;
	}
	return res;
}

PHPAPI char *snowflake_sock_read(SnowflakeSock *snowflake_sock, int *buf_len TSRMLS_DC) {
	char inbuf[1024];
	char *resp = NULL;

	snowflake_check_eof(snowflake_sock TSRMLS_CC);
	php_stream_gets(snowflake_sock->stream, inbuf, 1024);

	switch (inbuf[0]) {
		case '-':
			return NULL;
		case '+':
		case ':':
			*buf_len = strlen(inbuf) - 2;
			if (*buf_len >= 2) {
				resp = emalloc(1+*buf_len);
				memcpy(resp, inbuf, *buf_len);
				resp[*buf_len] = 0;
				return resp;
			} else {
				printf("protocol error \n");
				return NULL;
			}
		default:
			printf("protocol error, got '%c' as reply type byte\n", inbuf[0]);
	}
	return NULL;
}

PHPAPI int snowflake_sock_write(SnowflakeSock *snowflake_sock, char *cmd, size_t sz) {
	snowflake_check_eof(snowflake_sock TSRMLS_CC);
	return php_stream_write(snowflake_sock->stream, cmd, sz);
	return 0;
}

PHPAPI void snowflake_check_eof(SnowflakeSock *snowflake_sock TSRMLS_DC) {
	int eof = php_stream_eof(snowflake_sock->stream);
	while (eof) {
		snowflake_sock->stream = NULL;
		snowflake_sock_connect(snowflake_sock TSRMLS_CC);
		eof = php_stream_eof(snowflake_sock->stream);
	}
}

PHPAPI int snowflake_sock_get(zval *id, SnowflakeSock **snowflake_sock TSRMLS_DC) {
	zval **socket;
	int resource_type;

	if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socket", sizeof("socket"), (void **) &socket) == FAILURE) {
		return -1;
	}

	*snowflake_sock = (SnowflakeSock *) zend_list_find(Z_LVAL_PP(socket), &resource_type);
	if (!*snowflake_sock || resource_type != le_snowflake_sock) {
			return -1;
	}

	return Z_LVAL_PP(socket);
}

/**
 * snowflake_free_socket
 */
PHPAPI void snowflake_free_socket(SnowflakeSock *snowflake_sock) {
	efree(snowflake_sock->host);
	efree(snowflake_sock);
}

PHPAPI zend_class_entry *snowflake_get_exception_base(int root TSRMLS_DC) {
#if HAVE_SPL
	if (!root) {
			if (!spl_ce_RuntimeException) {
					zend_class_entry **pce;
					if (zend_hash_find(CG(class_table), "runtimeexception", sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
							spl_ce_RuntimeException = *pce;
							return *pce;
					}
			} else {
					return spl_ce_RuntimeException;
			}
	}
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
	return zend_exception_get_default();
#else
	return zend_exception_get_default(TSRMLS_C);
#endif
}

static void snowflake_destructor_snowflake_sock(zend_rsrc_list_entry * rsrc TSRMLS_DC) {
	SnowflakeSock *snowflake_sock = (SnowflakeSock *) rsrc->ptr;
	snowflake_sock_disconnect(snowflake_sock TSRMLS_CC);
	snowflake_free_socket(snowflake_sock);
}

PHP_MINIT_FUNCTION(snowflake) {
	zend_class_entry snowflake_class_entry;
	INIT_CLASS_ENTRY(snowflake_class_entry, "Snowflake", snowflake_functions);
	snowflake_ce = zend_register_internal_class(&snowflake_class_entry TSRMLS_CC);

	zend_class_entry snowflake_exception_class_entry;
	INIT_CLASS_ENTRY(snowflake_exception_class_entry, "SnowflakeException", NULL);
	snowflake_exception_ce = zend_register_internal_class_ex(
		&snowflake_exception_class_entry,
		snowflake_get_exception_base(0 TSRMLS_CC),
		NULL TSRMLS_CC
	);

	le_snowflake_sock = zend_register_list_destructors_ex(
		snowflake_destructor_snowflake_sock,
		NULL,
		snowflake_sock_name, module_number
	);
	// XXX: Scrub these
	add_constant_long(snowflake_ce, "SNOWFLAKE_NOT_FOUND", SNOWFLAKE_NOT_FOUND);
	add_constant_long(snowflake_ce, "SNOWFLAKE_STRING", SNOWFLAKE_STRING);
	add_constant_long(snowflake_ce, "SNOWFLAKE_SET", SNOWFLAKE_SET);
	add_constant_long(snowflake_ce, "SNOWFLAKE_LIST", SNOWFLAKE_LIST);
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(snowflake) {
	return SUCCESS;
}

PHP_RINIT_FUNCTION(snowflake) {
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(snowflake) {
	return SUCCESS;
}

PHP_MINFO_FUNCTION(snowflake) {
	php_info_print_table_start();
	php_info_print_table_header(2, "Snowflake Support", "enabled");
	php_info_print_table_row(2, "Version", PHP_SNOWFLAKE_VERSION);
	php_info_print_table_end();
}

PHP_METHOD(Snowflake, __construct) {
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		RETURN_FALSE;
	}
}

PHP_METHOD(Snowflake, connect) {
	zval *object;
	int host_len, id;
	char *host = NULL;
	long port;

	struct timeval timeout = {0L, 0L};
	SnowflakeSock *snowflake_sock	 = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll", &object, snowflake_ce, &host, &host_len, &port, &timeout.tv_sec) == FAILURE) {
		RETURN_FALSE;
	}
        
        if(!port) {
            port = SNOWFLAKE_DEFAULT_PORT;
        }

	if (timeout.tv_sec < 0L || timeout.tv_sec > INT_MAX) {
		zend_throw_exception(snowflake_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
		RETURN_FALSE;
	}

	snowflake_sock = snowflake_sock_create(host, host_len, port, timeout.tv_sec);

	if (snowflake_sock_server_open(snowflake_sock, 1 TSRMLS_CC) < 0) {
		snowflake_free_socket(snowflake_sock);
		zend_throw_exception_ex(
			snowflake_exception_ce,
			0 TSRMLS_CC,
			"Can't connect to %s:%d",
			host,
			port
		);
		RETURN_FALSE;
	}

	id = zend_list_insert(snowflake_sock, le_snowflake_sock);
	add_property_resource(object, "socket", id);

	RETURN_TRUE;
}

PHP_METHOD(Snowflake, close) {
	zval *object;
	SnowflakeSock *snowflake_sock = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
		&object, snowflake_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (snowflake_sock_get(object, &snowflake_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	if (snowflake_sock_disconnect(snowflake_sock TSRMLS_CC)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHPAPI void snowflake_boolean_response(INTERNAL_FUNCTION_PARAMETERS, SnowflakeSock *snowflake_sock TSRMLS_DC) {
	char *response;
	int response_len;
	char ret;

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}
	ret = response[0];
	efree(response);

	if (ret == '+') {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHPAPI void snowflake_long_response(INTERNAL_FUNCTION_PARAMETERS, SnowflakeSock *snowflake_sock TSRMLS_DC) {
	char *response;
	int response_len;

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	if(response[0] == ':') {
		long ret = atol(response + 1);
		efree(response);
		RETURN_LONG(ret);
	} else {
		efree(response);
		RETURN_FALSE;
	}
}

PHPAPI void snowflake_bulk_double_response(INTERNAL_FUNCTION_PARAMETERS, SnowflakeSock *snowflake_sock TSRMLS_DC) {

	char *response;
	int response_len;	 

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	double ret = atof(response);
	efree(response);
	RETURN_DOUBLE(ret);
}

PHPAPI void snowflake_1_response(INTERNAL_FUNCTION_PARAMETERS, SnowflakeSock *snowflake_sock TSRMLS_DC) {

	char *response;
	int response_len;
	char ret;

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
		RETURN_FALSE;
	}

	ret = response[1];
	efree(response);

	if (ret == '1') {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_METHOD(Snowflake, get) {
	zval *object;
	SnowflakeSock *snowflake_sock;
	char *val = NULL, *cmd, *response, *cur, *pos;
	int cmd_len, response_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, snowflake_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (snowflake_sock_get(object, &snowflake_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	cmd_len = spprintf(&cmd, 0, "GET\r\n");
	if (snowflake_sock_write(snowflake_sock, cmd, cmd_len) < 0) {
		efree(cmd);
		RETURN_FALSE;
	}
	efree(cmd);

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
            zend_throw_exception(snowflake_exception_ce, "Error getting ID", 1000 TSRMLS_CC);
            RETURN_FALSE;
	}
        
        cur = response;
        if (cur[0] == '+') {
            cur += 1;
            pos = cur + response_len - 1;
            val = emalloc(pos - cur + 1);
            memcpy(val, cur, pos-cur);
            val[pos-cur] = 0;
            RETURN_STRING(val, 0);
        }
	
}

PHP_METHOD(Snowflake, info) {

	zval *object;
	SnowflakeSock *snowflake_sock;

	char cmd[] = "INFO\r\n", *response, *key;
	int cmd_len = sizeof(cmd)-1, response_len;
	long ttl;
	char *cur, *pos, *value;
	int is_numeric;
	char *p;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, snowflake_ce) == FAILURE) {
		RETURN_FALSE;
	}

	if (snowflake_sock_get(object, &snowflake_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	if (snowflake_sock_write(snowflake_sock, cmd, cmd_len) < 0) {
		RETURN_FALSE;
	}

	if ((response = snowflake_sock_read(snowflake_sock, &response_len TSRMLS_CC)) == NULL) {
            zend_throw_exception(snowflake_exception_ce, "Error getting server info", 1000 TSRMLS_CC);
            RETURN_FALSE;
	}

	array_init(return_value);

	cur = response;
        cur += 1; // get past the first reply type byte
	while(1) {
		/* key */
		pos = strchr(cur, ':');
		if(pos == NULL) {
			break;
		}
		key = emalloc(pos - cur + 1);
		memcpy(key, cur, pos-cur);
		key[pos-cur] = 0;

		/* value */
		cur = pos + 1;
		pos = strchr(cur, '\r');
		if(pos == NULL) {
			break;
		}
		value = emalloc(pos - cur + 1);
		memcpy(value, cur, pos-cur);
		value[pos-cur] = 0;
		pos += 1; /* \r, \n */
		cur = pos;

		is_numeric = 1;
		for(p = value; *p; ++p) {
			if(*p < '0' || *p > '9') {
				is_numeric = 0;
				break;
			}
		}

		if(is_numeric == 1) {
			add_assoc_long(return_value, key, atol(value));
			efree(value);
		} else {
			add_assoc_string(return_value, key, value, 0);
		}
		efree(key);
	}
}

/* vim: set tabstop=4 expandtab: */