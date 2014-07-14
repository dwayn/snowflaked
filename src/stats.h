#ifndef __SNOWFLAKED_STATS__
#define __SNOWFLAKED_STATS__

#include <time.h>


struct _app_stats {
    time_t started_at;
    char *version;
    long int ids;
    long int waits;
    long int seq_max;
    int region_id;
    int worker_id;
    long int seq_cap;
} app_stats;



#endif