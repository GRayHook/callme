#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libtg/tgcommon.h"

#define TG_TOKEN "522362584:AAEX0oDV-rBHB5BgObcfkICT4ei5BCR6jGA"

int remember_me(tg_message_t *msg);
int read_config();
int callme_init_tg();
int switch_mode(int argc, char const *argv[], char *argsv[]);
