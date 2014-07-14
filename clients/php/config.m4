dnl $Id$
dnl config.m4 for extension snowflake

PHP_ARG_ENABLE(snowflake, whether to enable snowflake support,
dnl Make sure that the comment is aligned:
[  --enable-snowflake           Enable snowflake support])

if test "$PHP_SNOWFLAKE" != "no"; then

  dnl # --with-snowflake -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/snowflake.h"  # you most likely want to change this
  dnl if test -r $PHP_SNOWFLAKE/$SEARCH_FOR; then # path given as parameter
  dnl   SNOWFLAKE_DIR=$PHP_SNOWFLAKE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for snowflake files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SNOWFLAKE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SNOWFLAKE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the snowflake distribution])
  dnl fi

  dnl # --with-snowflake -> add include path
  dnl PHP_ADD_INCLUDE($SNOWFLAKE_DIR/include)

  dnl # --with-snowflake -> check for lib and symbol presence
  dnl LIBNAME=snowflake # you may want to change this
  dnl LIBSYMBOL=snowflake # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SNOWFLAKE_DIR/lib, SNOWFLAKE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SNOWFLAKELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong snowflake lib version or lib not found])
  dnl ],[
  dnl   -L$SNOWFLAKE_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(SNOWFLAKE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(snowflake, php_snowflake.c, $ext_shared)
fi