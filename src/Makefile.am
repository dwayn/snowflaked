## Process this file with automake to produce Makefile.in
bin_PROGRAMS	    = snowflaked snowflake sfbench

snowflaked_SOURCES  = snowflaked.c snowflaked.h stats.h commands.c commands.h snowflake.c snowflake.h
snowflaked_CFLAGS   = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

snowflake_SOURCES   = client.c
snowflake_CFLAGS    = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

sfbench_SOURCES	    = benchmark.c
sfbench_CFLAGS	    = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

if DEBUG
    debugprog = snowflaked-debug
else
    debugprog = 
endif
noinst_PROGRAMS		    = $(debugprog)
snowflaked_debug_SOURCES    = snowflaked.c snowflaked.h stats.h commands.c commands.h snowflake.c snowflake.h
snowflaked_debug_CFLAGS	    = -Wall -pg
snowflaked_debug_LDFLAGS    = -pg -lprofiler


