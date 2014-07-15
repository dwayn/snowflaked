/*
Copyright (c) 2014 Dwayn Matthies <dwayn dot matthies at gmail dot com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <netdb.h>
#include <getopt.h>
#include <unistd.h>

void send_command(int sd, char *command);

int main(int argc, char **argv) {
    char *host = "localhost";
    int port = 8008;

    int c;
    while (1) {
        static struct option long_options[] = {
            {"host", required_argument, 0, 'h'},
            {"port", required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "h:p:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
    }

    int action = -1;

    if (argc - optind == 0) {
        printf("Command not provided.\n");
        printf("usage: client [--host=] [--port=] <command> [... command arguments]\n");
        exit(1);
    }

    if (strcmp(argv[optind], "get") == 0) {
        if (argc - optind != 1) {
            printf("The 'get' command requires 0 command parameters.\n");
            printf("usage: client [--host=] [--port=] next\n");
            exit(1);
        }
        action = 1;
    }

    if (strcmp(argv[optind], "info") == 0) {
        if (argc - optind != 1) {
            printf("The 'info' command requires 0 command parameters.\n");
            printf("usage: client [--host=] [--port=] info\n");
            exit(1);
        }
        action = 2;
    }

    if (action == 0) {
        printf("Invalid command given, should be either get or info.\n");
        printf("usage: client [--host=] [--port=] <command> [... command arguments]\n");
        exit(1);
    }

    struct hostent *hp;
    struct sockaddr_in pin;
    int sd;

    if ((hp = gethostbyname(host)) == 0) {
        perror("gethostbyname");
        exit(1);
    }

    memset(&pin, 0, sizeof (pin));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
    pin.sin_port = htons(port);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (connect(sd, (struct sockaddr *) &pin, sizeof (pin)) == -1) {
        perror("connect");
        exit(1);
    }

    switch (action) {
        case 1:
            send_command(sd, "GET\r\n");
            break;
        case 2:
            send_command(sd, "INFO\r\n");
            break;
        default:
            break;
    }

    close(sd);

    return 0;
}

void send_command(int sd, char *command) {
    if (send(sd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(1);
    }
    char buf[1024];
    int numbytes;
    if ((numbytes = recv(sd, buf, 1024 - 1, 0)) == -1) {
        perror("recv()");
        exit(1);
    }

    char *resp = NULL;
    int buf_len;

    switch (buf[0]) {
        case '-':
            printf("Received Error\n");
            break;
        case '+':
        case ':':
            buf_len = strlen(buf);
            if (buf_len >= 2) {
                resp = malloc(buf_len);
                memcpy(resp, buf + 1, buf_len - 1);
                resp[buf_len] = '\0';
                // rewrite those pesky \r's with \n's so we can read them,
                // and terminate the string at the original \n
                for (int i = 0; i < buf_len; i++) {
                    if (resp[i] == '\n')
                        resp[i] = '\0';
                    if (resp[i] == '\r')
                        resp[i] = '\n';
                }
                printf("%s", resp);
                free(resp);
            } else {
                printf("protocol error \n");
            }
            break;
        default:
            printf("%s\n", buf);
            break;
    }
}
