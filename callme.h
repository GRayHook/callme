#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libtg/tgcommon.h"

#ifndef TG_TOKEN
#error "TG_TOKEN is not defined. Pass it at compile time, e.g. make TG_TOKEN=..."
#endif

#ifndef TG_PROXY
#define TG_PROXY ""
#endif

int script_argsv(int argc, char *argv[], char *argsv[]);
int exec_argsv(int argc, char *argv[], char *argsv[]);
int do_exec(int argc, char *argsv[], char * text_msg);
int remember_me(tg_message_t *msg);
int64_t read_config();
void print_help();
