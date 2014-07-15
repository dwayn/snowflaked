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
