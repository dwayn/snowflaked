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

#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

const int LOW = 1;
const int HIGH = 500;

void send_command(int sd, char *command, int iteration, int modval);

int main(int argc, char **argv) {
    char *host = "localhost";
    int port = 8008;
    int iterations = 1000;
    int modval = 1;

    int c;
    while (1) {
        static struct option long_options[] = {
            {"host", required_argument, 0, 'h'},
            {"port", required_argument, 0, 'p'},
            {"iterations", required_argument, 0, 'n'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "h:p:n:m:", long_options, &option_index);
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
            case 'n':
                iterations = atoi(optarg);
                break;
            case 'm':
                modval = atoi(optarg);
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
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

    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);

    int n = 0;
    char msg[10];
    sprintf(msg, "GET\r\n");
    while (n < iterations) {
        if(n % modval == 0)
            printf("Send: GET ... ");
        send_command(sd, msg, n, modval);
        n++;
    }
    send_command(sd, "INFO\r\n", 0, 1);

    close(sd);

    return 0;
}

void send_command(int sd, char *command, int iteration, int modval) {
    if (send(sd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(1);
    }
    char buf[300];
    int numbytes;
    if ((numbytes = recv(sd, buf, 300 - 1, 0)) == -1) {
        perror("recv()");
        exit(1);
    }
    buf[numbytes] = '\0';
    if (iteration % modval == 0)
        printf("Receive: %s", buf);
}
