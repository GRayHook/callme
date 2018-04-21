#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libtg/tgcommon.h"

#define TG_TOKEN "522362584:AAEX0oDV-rBHB5BgObcfkICT4ei5BCR6jGA"

int script_argsv(int argc, char *argv[], char *argsv[]);
int exec_argsv(int argc, char *argv[], char *argsv[]);
int do_exec(int argc, char *argsv[]);
int remember_me(tg_message_t *msg);
int read_config();
void print_help();
