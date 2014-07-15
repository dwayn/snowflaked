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
#include <unistd.h>

#include "commands.h"
#include "snowflake.h"
#include "stats.h"

void command_get(int fd, token_t *tokens) {
    long int id;
    id = snowflake_id();
    char msg[32];
    sprintf(msg, "+%ld\r\n", id);
    reply(fd, msg);
}

void command_info(int fd, token_t *tokens) {
	char out[200];
	time_t current_time;
	time(&current_time);
        sprintf(out, "+uptime:%d\rversion:%s\rregion:%d\rworker:%d\rseq_cap:%ld\rseq_max:%ld\rids:%ld\rwaits:%ld\r\n",
                (int)(current_time - app_stats.started_at),
                app_stats.version,
                app_stats.region_id,
                app_stats.worker_id,
                app_stats.seq_cap,
                app_stats.seq_max,
                app_stats.ids,
                app_stats.waits
                );
        reply(fd, out);
}

// TODO: Clean the '\r\n' scrub code.
// TODO: Add support for the 'quit' command.
void process_request(int fd, char *input) {
	char* nl;
	nl = strrchr(input, '\r');
	if (nl) { *nl = '\0'; }
	nl = strrchr(input, '\n');
	if (nl) { *nl = '\0'; }
	token_t tokens[MAX_TOKENS];
	size_t ntokens = tokenize_command(input, tokens, MAX_TOKENS);
        if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "GET") == 0) {
		command_get(fd, tokens);
	} else if (ntokens == 2 && strcmp(tokens[COMMAND_TOKEN].value, "INFO") == 0) {
		command_info(fd, tokens);
	} else {
		reply(fd, "-ERROR\r\n");
	}
}

size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens) {
	char *s, *e;
	size_t ntokens = 0;
	for (s = e = command; ntokens < max_tokens - 1; ++e) {
		if (*e == ' ') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
				*e = '\0';
			}
			s = e + 1;
		} else if (*e == '\0') {
			if (s != e) {
				tokens[ntokens].value = s;
				tokens[ntokens].length = e - s;
				ntokens++;
			}
			break;
		}
	}
	tokens[ntokens].value =  *e == '\0' ? NULL : e;
	tokens[ntokens].length = 0;
	ntokens++;
	return ntokens;
}

void reply(int fd, char *buffer) {
	int n = write(fd, buffer, strlen(buffer));
	if (n < 0 || n < strlen(buffer)) {
		printf("ERROR writing to socket");
	}
}
