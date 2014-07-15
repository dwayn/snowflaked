# Process this file with autoconf to produce a configure script.

# Prelude.
AC_PREREQ([2.59])
AC_INIT([Snowflaked], [0.2.0], [dwayn dot matthies at gmail dot com])

# unique source file --- primitive safety check 
AC_CONFIG_SRCDIR([src/snowflaked.c])

# place to put some extra build scripts installed
AC_CONFIG_AUX_DIR([build-aux])


AC_CONFIG_MACRO_DIR([m4])

# fairly severe build strictness
# change foreign to gnu or gnits to comply with gnu standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.9.6  no-dependencies subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Checks for libraries.

# AC_CHECK_LIB([m], [cos])
# pthread should not be needed now
# AC_CHECK_LIB([pthread], [pthread_create]) 

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.  
#  AM_PATH_CHECK([MINIMUM-VERSION,
#                [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
# AM_PATH_CHECK()

PKG_CHECK_MODULES(CHECK, [check >= 0.9.8], [have_check=yes], [have_check=no])
AM_CONDITIONAL(HAVE_CHECK, test x"$have_check" = "xyes")


# LIBEVENT_URL=http://www.monkey.org/~provos/libevent/
AC_CHECK_LIB(event, event_set, , [AC_MSG_ERROR(libevent is required.  You can get it from $LIBEVENT_URL)])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_ARG_ENABLE(gcov,
	      AS_HELP_STRING([--enable-gcov],
                             [enable test coverage with gcov @<:@default=no@:>@]),
              [case "${enableval}" in
                 yes) gcov=true ;;
                 no)  gcov=false ;;
                 *)   AC_MSG_ERROR([bad value ${enableval} for --enable-gcov]) ;;
               esac],
              [gcov=false])

AM_CONDITIONAL(GCOV, test x"$gcov" = "xtrue")

AC_ARG_ENABLE(debug,
              [  --enable-debug    Turn on debugging],
              [case "${enableval}" in
                  yes) debug=true ;;
                  no)  debug=false ;;
                  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
              esac],[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

# Output files
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])

AC_OUTPUT


