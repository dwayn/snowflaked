#Dependencies
##Library dependencies:
* libevent 2.x
* pkgconfig >= 0.28
* check 0.9.8  (need to test if check lib detection is working properly)


##Build dependencies:
* libtoolize/glibtoolize
* autotools (aclocal, autoheader, autoconf, automake)


##Debug build dependencies:
* gperftools



#Building:
* ./bootstrap.sh
* ./configure [--enable-debug]
* make


#TODO
* Documentation
* Tests for snowflaked components
* Tests for php-snowflake
* Need to run through memory profiler