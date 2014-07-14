## Process this file with automake to produce Makefile.in
AM_CPPFLAGS = @CHECK_CFLAGS@
if HAVE_CHECK
    TESTS = check_snowflake
else
    TESTS = 
endif

noinst_PROGRAMS = $(TESTS)
check_snowflake_SOURCES = check_snowflake.c $(top_builddir)/src/snowflake.c $(top_builddir)/src/snowflake.h $(top_builddir)/src/stats.h
check_snowflake_CFLAGS = @CHECK_CFLAGS@ -g -Wall
# -fprofile-arcs -ftest-coverage
check_snowflake_LDADD = @CHECK_LIBS@