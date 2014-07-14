#ifndef __SNOWFLAKE_COMMANDS__
#define __SNOWFLAKE_COMMANDS__


#define COMMAND_TOKEN		0
#define VALUE_TOKEN		1
#define MAX_TOKENS		8

typedef struct token_s {
    char *value;
    size_t length;
} token_t;

void command_get(int fd, token_t *tokens);
void command_info(int fd, token_t *tokens);
void process_request(int fd, char *input);
size_t tokenize_command(char *command, token_t *tokens, const size_t max_tokens);
void reply(int fd, char *buffer);



#endif /* __SNOWFLAKE_COMMANDS__ */
