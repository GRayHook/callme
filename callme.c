#include "callme.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		print_help();
		return 1;
	}

	json_object *content_json;
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
			exit(1);
		}

		switch (argv[shift][1])
		{
			case 'h':
				print_help();
				exit(0);
				break;

			case 'n':
				printf("Send '/start %s' to @CommandNotifyBot\n", getenv("USER"));
				tg_callback_bind((char *)"start", &remember_me);
				sleep(200);
				printf("Timed out\n");
				break;

			case 'm':
				shift++;
				if (shift == argc)
				{
					print_help();
					exit(1);
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
				printf("Unknown key '-%c'!\n", argv[1][1]);
				print_help();
				exit(1);
		}
		shift++;
	}

	if (NULL != do_cmd)
		do_cmd(argc, argv, argsv);
	else
		argsc = -1;

	do_exec(argsc, argsv, text_msg);

	return 0;
}

int script_argsv(int argc, char *argv[], char *argsv[])
{
	argsv[0] = "/bin/sh";
	argsv[1] = "-c";

	argsv[2] = argv[2];

	argsv[argc] = 0;

	return 0;
}

int exec_argsv(int argc, char *argv[], char *argsv[])
{
	size_t start = 2;
	size_t i = 0;

	for (; i < argc; i++)
	{
		if (!strcmp(argv[i], "-m"))
			start = i + 2;
	}

	for (i = start; i < argc; i++) argsv[i - 2] = argv[i];

	argsv[argc - 2] = 0;

	return 0;
}

int do_exec(int argc, char *argsv[], char * text_msg)
{
	int chat_id;
	if ((chat_id = read_config()) == -1) return 1;

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

		waitpid(chpid, NULL, WUNTRACED);
	}

	tg_message_t answer;
	if (NULL != text_msg)
		strcpy(answer.text, text_msg);
	else
	{
		strcpy(answer.text, "Ur command '");

		int i = 0;
		while(argsv[i] != 0)
		{
			strcpy(answer.text + strlen(answer.text), argsv[i++]);
			strcpy(answer.text + strlen(answer.text), " ");
		}

		strcpy(answer.text + strlen(answer.text) - 1, "' done!");
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
		exit(1);
	}
	fprintf(fd, "%d\n", msg->chat_id);
	fclose(fd);

	exit(0);
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
