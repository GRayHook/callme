#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libtg/tgcommon.h"

#define TG_TOKEN "YOUR TOKEN HERE"

int script_argsv(int argc, char *argv[], char *argsv[]);
int exec_argsv(int argc, char *argv[], char *argsv[]);
int do_exec(int argc, char *argsv[], char * text_msg);
int remember_me(tg_message_t *msg);
int read_config();
void print_help();
