#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>

#include "snowflake.h"
#include "snowflaked.h"
#include "stats.h"
#include "commands.h"

void on_read(int fd, short ev, void *arg) {
    struct client *client = (struct client *) arg;
    char buf[64];
    int len = read(fd, buf, sizeof (buf));
    if (len == 0) {
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    } else if (len < 0) {
        printf("Socket failure, disconnecting client: %s", strerror(errno));
        close(fd);
        event_del(&client->ev_read);
        free(client);
        return;
    }
    process_request(fd, buf);
}

void on_accept(int fd, short ev, void *arg) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof (client_addr);
    struct client *client;
    client_fd = accept(fd, (struct sockaddr *) &client_addr, &client_len);
    if (client_fd == -1) {
        warn("accept failed");
        return;
    }
    if (setnonblock(client_fd) < 0) {
        warn("failed to set client socket non-blocking");
    }
    client = calloc(1, sizeof (*client));
    if (client == NULL) {
        err(1, "malloc failed");
    }
    event_set(&client->ev_read, client_fd, EV_READ | EV_PERSIST, on_read, client);
    event_add(&client->ev_read, NULL);
}

int main(int argc, char **argv) {
    int region_id = -1;
    int worker_id = -1;
    int port = SERVER_PORT;
    timeout = 60;
    static int daemon_mode = 0;

    int c;
    while (1) {
        static struct option long_options[] = {
            {"daemon", no_argument, &daemon_mode, 1},
            {"region", required_argument, 0, 'r'},
            {"worker", required_argument, 0, 'w'},
            {"port", required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "r:w:p:?", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;
            case 'r':
                region_id = atoi(optarg);
                break;
            case 'w':
                worker_id = atoi(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case '?':
                printf("Usage: snowflaked -r REGIONID -w WORKERID [-p PORT(8008)] [--daemon]\n\n");
                exit(0);
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
    }
//    if (daemon_mode == 1) {
//        daemonize();
//    }
    if (region_id < 0 || worker_id < 0) {
        printf("Region ID and Worker ID must both be provided\n");
        exit(1);
    }

    time(&app_stats.started_at);
    app_stats.version = "00.02.00";
    int test = snowflake_init(region_id, worker_id);
    if (test < 0)
        exit(1);

    int listen_fd;
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    struct event ev_accept;
    event_init();
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        err(1, "listen failed");
    }
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof (reuseaddr_on)) == -1) {
        err(1, "setsockopt failed");
    }
    memset(&listen_addr, 0, sizeof (listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr *) &listen_addr, sizeof (listen_addr)) < 0) {
        err(1, "bind failed");
    }
    if (listen(listen_fd, 5) < 0) {
        err(1, "listen failed");
    }
    if (setnonblock(listen_fd) < 0) {
        err(1, "failed to set server socket to non-blocking");
    }
    event_set(&ev_accept, listen_fd, EV_READ | EV_PERSIST, on_accept, NULL);
    event_add(&ev_accept, NULL);
    event_dispatch();

    return 0;
}

int setnonblock(int fd) {
    int flags;
    flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return flags;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        return -1;
    }
    return 0;
}

// this does not work quite correctly...need to dig into it more

//void daemonize() {
//    int i, lfp;
//    char str[10];
//    /* already a daemon */
//    if (getppid() == 1) {
//        return;
//    }
//    i = fork();
//    if (i < 0) { /* fork error */
//        exit(1);
//    }
//    if (i > 0) { /* parent exits */
//        exit(0);
//    }
//    setsid();
//    for (i = getdtablesize(); i >= 0; --i) { /* close all descriptors */
//        close(i);
//    }
//    i = open("/dev/null", O_RDWR);
//    dup(i);
//    dup(i);
//    umask(027);
//    chdir(RUNNING_DIR);
//    lfp = open(LOCK_FILE, O_RDWR | O_CREAT, 0640);
//    if (lfp < 0) {
//        exit(1);
//    }
//    if (lockf(lfp, F_TLOCK, 0) < 0) {
//        exit(0);
//    }
//    sprintf(str, "%d\n", getpid());
//    write(lfp, str, strlen(str));
//}
