#include <arpa/inet.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
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
        sprintf(out, "Uptime: %d\nVersion: %s\nRegion: %d\nWorker: %d\nSeq_cap: %ld\nSeq_max: %ld\nIDs: %ld\nWaits: %ld\r\n",
                (int)(current_time - app_stats.started_at),
                app_stats.version,
                app_stats.region_id,
                app_stats.worker_id
                app_stats.seq_cap,
                app_stats.seq_max,
                app_stats.ids,
                app_stats.waits
                );
        reply(fd, out);
        
        /* testing rolling up into one reply call so that only one network packet is sent back
	sprintf(out, "Uptime: %d\n", (int)(current_time - app_stats.started_at)); 
        reply(fd, out);
	sprintf(out, "Version: %s\n", app_stats.version); 
        reply(fd, out);
	sprintf(out, "Region: %d\n", app_stats.region_id); 
        reply(fd, out);
	sprintf(out, "Worker: %d\n", app_stats.worker_id); 
        reply(fd, out);
	sprintf(out, "Seq_cap: %ld\n", app_stats.seq_cap); 
        reply(fd, out);
	sprintf(out, "Seq_max: %ld\n", app_stats.seq_max); 
        reply(fd, out);
	sprintf(out, "IDs: %ld\n", app_stats.ids); 
        reply(fd, out);
	sprintf(out, "Waits: %ld\r\n", app_stats.waits); 
        reply(fd, out);
        */
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
