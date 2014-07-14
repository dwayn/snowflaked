#ifndef __SNOWFLAKED__
#define __SNOWFLAKED__


#include <event.h>

#define SERVER_PORT			8008
#define RUNNING_DIR			"/tmp"

struct client {
    struct event ev_read;
};

int timeout;

int main(int argc, char **argv);
void on_read(int fd, short ev, void *arg);
void on_accept(int fd, short ev, void *arg);
int setnonblock(int fd);
void daemonize();




#endif /* __SNOWFLAKED__ */
