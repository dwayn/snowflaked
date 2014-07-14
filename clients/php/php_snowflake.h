#ifndef PHP_SNOWFLAKE_H
#define PHP_SNOWFLAKE_H

PHP_METHOD(Snowflake, __construct);
PHP_METHOD(Snowflake, connect);
PHP_METHOD(Snowflake, close);
PHP_METHOD(Snowflake, info);
PHP_METHOD(Snowflake, get);

#ifdef PHP_WIN32
#define PHP_SNOWFLAKE_API __declspec(dllexport)
#else
#define PHP_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(snowflake);
PHP_MSHUTDOWN_FUNCTION(snowflake);
PHP_RINIT_FUNCTION(snowflake);
PHP_RSHUTDOWN_FUNCTION(snowflake);
PHP_MINFO_FUNCTION(snowflake);

/* {{{ struct SnowflakeSock */
typedef struct SnowflakeSock_ {
    php_stream     *stream;
    char           *host;
    unsigned short port;
    long           timeout;
    int            failed;
    int            status;
} SnowflakeSock;
/* }}} */

#define snowflake_sock_name "Snowflake Socket Buffer"

#define SNOWFLAKE_DEFAULT_PORT 8008

#define SNOWFLAKE_SOCK_STATUS_FAILED 0
#define SNOWFLAKE_SOCK_STATUS_DISCONNECTED 1
#define SNOWFLAKE_SOCK_STATUS_UNKNOWN 2
#define SNOWFLAKE_SOCK_STATUS_CONNECTED 3

/* properties */
#define SNOWFLAKE_NOT_FOUND 0
#define SNOWFLAKE_STRING 1
#define SNOWFLAKE_SET 2
#define SNOWFLAKE_LIST 3


/* {{{ internal function protos */
void add_constant_long(zend_class_entry *ce, char *name, int value);

PHPAPI void snowflake_check_eof(SnowflakeSock *snowflake_sock TSRMLS_DC);
PHPAPI SnowflakeSock* snowflake_sock_create(char *host, int host_len, unsigned short port, long timeout);
PHPAPI int snowflake_sock_connect(SnowflakeSock *snowflake_sock TSRMLS_DC);
PHPAPI int snowflake_sock_disconnect(SnowflakeSock *snowflake_sock TSRMLS_DC);
PHPAPI int snowflake_sock_server_open(SnowflakeSock *snowflake_sock, int TSRMLS_DC);
PHPAPI char * snowflake_sock_read(SnowflakeSock *snowflake_sock, int *buf_len TSRMLS_DC);
PHPAPI int snowflake_sock_write(SnowflakeSock *snowflake_sock, char *cmd, size_t sz);
PHPAPI void snowflake_free_socket(SnowflakeSock *snowflake_sock);

PHPAPI void snowflake_atomic_increment(INTERNAL_FUNCTION_PARAMETERS, char *keyword TSRMLS_DC);

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
ZEND_END_MODULE_GLOBALS(snowflake)

#define PHP_SNOWFLAKE_VERSION "0.2.0"

#endif