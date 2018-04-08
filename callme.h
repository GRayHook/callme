#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libtg/tgcommon.h"

#define TG_TOKEN "522362584:AAEX0oDV-rBHB5BgObcfkICT4ei5BCR6jGA"

int switch_mode(int argc, char *argv[]);
int script_argsv(int chat_id, int argc, char *argv[], char *argsv[]);
int exec_argsv(int chat_id, int argc, char *argv[], char *argsv[]);
int do_exec(int chat_id, int argc, char *argsv[]);
int remember_me(tg_message_t *msg);
int read_config();
