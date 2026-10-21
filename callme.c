#include "callme.h"

/* The library keeps a detached polling thread inside libcurl/json-c.
 * exit() would run atexit handlers (OpenSSL/curl/stdio cleanup) while
 * that thread is still mid-request, freeing state it uses -> rare
 * use-after-free at process end. Flush buffers and terminate the whole
 * process at once instead. */
static void finish(int code) __attribute__((noreturn));
static void finish(int code)
{
	fflush(stdout);
	fflush(stderr);
	_exit(code);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		print_help();
		return 1;
	}

	json_object *content_json = NULL;
	if (*TG_PROXY != '\0')
	{
		/* tg_set_proxy() in libtg copies into a fixed 64-byte buffer */
		if (strlen(TG_PROXY) >= 64)
		{
			fprintf(stderr, "TG_PROXY too long (max 63 chars): %zu\n", strlen(TG_PROXY));
			return 1;
		}
		tg_set_proxy(TG_PROXY);
	}
	if (tg_start(&content_json, TG_TOKEN) != 0) return 1;

	char *argsv[argc + 1];
	size_t argsc = argc;

	int (*do_cmd)(int argc, char *argv[], char *argsv[]) = NULL;
	char * text_msg = NULL;

	int shift = 1;
	while (shift < argc)
	{
		if (NULL != do_cmd)
			break;
		if ('-' != argv[shift][0])
		{
			print_help();
			finish(1);
		}

		switch (argv[shift][1])
		{
			case 'h':
				print_help();
				finish(0);
				break;

			case 'n':
				printf("Send '/start %s' to @CommandNotifyBot\n", getenv("USER"));
				tg_callback_bind((char *)"start", &remember_me);
				sleep(200);
				printf("Timed out\n");
				finish(1);
				break;

			case 'm':
				shift++;
				if (shift == argc)
				{
					print_help();
					finish(1);
				}
				printf("I will send message \"%s\"\n", argv[shift]);
				text_msg = argv[shift];
				argsc -= 2;
				break;

			case 'e':
				printf("Execute command\n");
				do_cmd = exec_argsv;
				break;

			case 's':
				printf("Do script\n");
				do_cmd = script_argsv;
				break;

			default:
				printf("Unknown key '-%c'!\n", argv[shift][1]);
				print_help();
				finish(1);
		}
		shift++;
	}

	if (NULL != do_cmd)
		do_cmd(argc, argv, argsv);
	else
		argsc = -1;

	do_exec(argsc, argsv, text_msg);

	finish(0);
}

/* Index right after the -e/-s key, skipping '-m msg' if present */
static size_t cmd_start(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-m")) { i++; continue; }
		if (!strcmp(argv[i], "-e") || !strcmp(argv[i], "-s"))
			return (size_t)i + 1;
	}
	return (size_t)argc;
}

int script_argsv(int argc, char *argv[], char *argsv[])
{
	size_t start = cmd_start(argc, argv);

	argsv[0] = "/bin/sh";
	argsv[1] = "-c";
	argsv[2] = (start < (size_t)argc) ? argv[start] : "";
	argsv[3] = 0;

	return 0;
}

int exec_argsv(int argc, char *argv[], char *argsv[])
{
	size_t start = cmd_start(argc, argv);
	size_t i;

	for (i = start; i < (size_t)argc; i++) argsv[i - start] = argv[i];

	argsv[i - start] = 0;

	return 0;
}

int do_exec(int argc, char *argsv[], char * text_msg)
{
	int chat_id;
	if ((chat_id = read_config()) == -1) return 1;

	int status = 0;
	if (0 < argc)
	{
		pid_t chpid;
		int pip[2];

		if(pipe(pip) == -1)
		{
			fprintf(stderr, "Error creating pipe\n");
		}

		if(!(chpid = fork()))
		{
			close(pip[0]);
			dup2(pip[1],1); // 1 -> stdout
			fprintf(stdout, "You will recieve notify...\n"); // 1 -> stdout
			execvp(argsv[0], (char*const*) argsv);
		}
		close(pip[1]);

		char buf[1];
		while(read(pip[0], buf, 1) > 0)
		{
			write(1, buf, 1); // 1 -> stdout
		}

		close(pip[0]);

		do
		{
			waitpid(chpid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	tg_message_t answer;
	if (NULL != text_msg)
		snprintf(answer.text, sizeof(answer.text), "%s", text_msg);
	else
	{
		char cmd[900];
		size_t off = 0;
		int i = 0;
		while (argsv[i] != 0 && off < sizeof(cmd) - 1)
		{
			int n = snprintf(cmd + off, sizeof(cmd) - off, "%s ", argsv[i++]);
			if (n < 0 || (size_t)n >= sizeof(cmd) - off) break;
			off += n;
		}
		if (off) cmd[off - 1] = '\0'; // drop trailing space
		else cmd[0] = '\0';

		char exit_code[8] = "";
		snprintf(exit_code, sizeof(exit_code), "%d", WEXITSTATUS(status));
		snprintf(answer.text, sizeof(answer.text),
		         "Ur command '%s' done!\nExit code is %s", cmd, exit_code);
	}

	answer.chat_id = chat_id;
	tg_send_message(&answer);
	return 0;
}

int remember_me(tg_message_t *msg)
{
	tg_message_t answer;
	answer.chat_id = msg->chat_id;

	char username[64];
	if (tg_get_command_arg(msg->text, username))
	{
		sprintf(answer.text, "%s%s",
		        "Send me your username (/start 'username')",
		        " and start ./callme -n");
		tg_send_message(&answer);
		return 0;
	}

	if (strcmp(username, getenv("USER")))
	{
		strcpy(answer.text, "Username invalid");
		tg_send_message(&answer);
		return 0;
	}

	strcpy(answer.text, "I remember you");
	tg_send_message(&answer);

	printf("%s:%d start recv\n", username, msg->chat_id);

	char pathname[256];
	sprintf(pathname, "%s/.callme", getenv("HOME"));
	FILE *fd;
	if ((fd = fopen(pathname, "w")) == NULL)
	{
		printf("Cannot access config\n");
		finish(1);
	}
	fprintf(fd, "%d\n", msg->chat_id);
	fclose(fd);

	finish(0);
}

int read_config()
{
	char pathname[256];
	sprintf(pathname, "%s/.callme", getenv("HOME"));

	FILE *fd;
	if ((fd = fopen(pathname, "r")) == NULL)
	{
		printf("Cannot access config\n");
		return -1;
	}

	int chat_id = -1;
	fscanf(fd, "%d\n", &chat_id);

	fclose(fd);

	return chat_id;
}

void print_help()
{
	printf("Usage: callme [-n | [-m message] [ -e program | -s 'script' ]]\n");
	printf("Keys:\n");
	printf("\t-m — Text message to send it instead of executing command.\n");
	printf("\t\tMust be placed before other commands.\n");
	printf("\t-n — Make config. You have to send your username\n");
	printf("\t\tafter start to Telegram-bot @CommandNotifyBot.\n");
	printf("\t\tMust be only one in args\n");
	printf("\t-e — Execute program. All gived args will exec\n");
	printf("\t\tlike that executing without 'callme -e'.\n");
	printf("\t-s — Run gived srcipt. It's like run:\n");
	printf("\t\tcallme -e bash -c 'script'.\n");
}
