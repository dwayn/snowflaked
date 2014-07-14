#include "stats.h"
#include "snowflake.h"
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

long int snowflake_id() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - SNOWFLAKE_EPOCH;
    long int id = 0L;

    // Catch NTP clock adjustment that rolls time backwards or the sequence number has overflowed
    if ((snowflake_global_state.seq > snowflake_global_state.seq_max ) || snowflake_global_state.time > millisecs) {
        ++app_stats.waits;
        while (snowflake_global_state.time >= millisecs) {
            gettimeofday(&tp, NULL);
            millisecs = tp.tv_sec * 1000 + tp.tv_usec / 1000 - SNOWFLAKE_EPOCH;
        }
    }

    if (snowflake_global_state.time < millisecs) {
        snowflake_global_state.time = millisecs;
        snowflake_global_state.seq = 0L;
    }
    
    
    id = (millisecs << snowflake_global_state.time_shift_bits) 
            | (snowflake_global_state.region_id << snowflake_global_state.region_shift_bits) 
            | (snowflake_global_state.worker_id << snowflake_global_state.worker_shift_bits) 
            | (snowflake_global_state.seq++); 

    if (app_stats.seq_max < snowflake_global_state.seq)
        app_stats.seq_max = snowflake_global_state.seq;
    
    ++app_stats.ids;
    return id;
}

int snowflake_init(int region_id, int worker_id) {
    int max_region_id = (1 << SNOWFLAKE_REGIONID_BITS) - 1;
    if(region_id < 0 || region_id > max_region_id){
        printf("Region ID must be in the range : 0-%d\n", max_region_id);
        return -1;
    }
    int max_worker_id = (1 << SNOWFLAKE_WORKERID_BITS) - 1;
    if(worker_id < 0 || worker_id > max_worker_id){
        printf("Worker ID must be in the range: 0-%d\n", max_worker_id);
        return -1;
    }
    
    snowflake_global_state.time_shift_bits   = SNOWFLAKE_REGIONID_BITS + SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    snowflake_global_state.region_shift_bits = SNOWFLAKE_WORKERID_BITS + SNOWFLAKE_SEQUENCE_BITS;
    snowflake_global_state.worker_shift_bits = SNOWFLAKE_SEQUENCE_BITS;
    
    snowflake_global_state.worker_id    = worker_id;
    snowflake_global_state.region_id    = region_id;
    snowflake_global_state.seq_max      = (1L << SNOWFLAKE_SEQUENCE_BITS) - 1;
    snowflake_global_state.seq          = 0L;
    snowflake_global_state.time         = 0L;
    app_stats.seq_cap                   = snowflake_global_state.seq_max;
    app_stats.waits                     = 0L;
    app_stats.seq_max                   = 0L;
    app_stats.ids                       = 0L;
    app_stats.region_id                 = region_id;
    app_stats.worker_id                 = worker_id;
    return 1;
}



