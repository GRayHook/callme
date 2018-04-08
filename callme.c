#include "callme.h"

int main(int argc, char *argv[])
{
	json_object *content_json;
	if (tg_start(&content_json, TG_TOKEN) != 0) return 1;

	switch_mode(argc, argv);

	return 0;
}

int switch_mode(int argc, char *argv[])
{
	if (argc < 2) exit(0);

	char *argsv[argc + 1];

	switch (argv[1][1]) {
		case 'n':
			printf("Send '%s' to CallMeBot\n", getenv("USER"));
			tg_callback_bind((char *)"start", &remember_me);
			sleep(200);
			break;

		case 'e':
			printf("Execute command\n");
			exec_argsv(argc, argv, argsv);
			break;

		case 's':
			printf("Do script\n");
			script_argsv(argc, argv, argsv);
			break;

		default:
			printf("gimme command -n / -e / -s\n");
	}

	do_exec(argc, argsv);

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
	for (size_t i = 2; i < argc; i++) argsv[i - 2] = argv[i];

	argsv[argc - 2] = 0;

	return 0;
}

int do_exec(int argc, char *argsv[])
{
	int chat_id;
	if ((chat_id = read_config()) == -1) return 1;

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

	tg_message_t answer;
	strcpy(answer.text, "Ur command '");

	int i = 0;
	while(argsv[i] != 0)
	{
		strcpy(answer.text + strlen(answer.text), argsv[i++]);
		strcpy(answer.text + strlen(answer.text), " ");
	}

	strcpy(answer.text + strlen(answer.text) - 1, "' done!");

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
