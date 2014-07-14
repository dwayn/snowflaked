#ifndef __SNOWFLAKE__
#define __SNOWFLAKE__


#define SNOWFLAKE_EPOCH 1388534400000 //Midnight Januarly 1, 2014

#define SNOWFLAKE_TIME_BITS 41
#define SNOWFLAKE_REGIONID_BITS 4
#define SNOWFLAKE_WORKERID_BITS 10
#define SNOWFLAKE_SEQUENCE_BITS 8

struct _snowflake_state {
    // milliseconds since SNOWFLAKE_EPOCH 
    long int time;
    long int seq_max;
    long int worker_id;
    long int region_id;
    long int seq;
    long int time_shift_bits;
    long int region_shift_bits;
    long int worker_shift_bits;
} snowflake_global_state;

long int snowflake_id();
int snowflake_init(int region_id, int worker_id);

#endif /* __SNOWFLAKE__ */