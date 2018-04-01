#include "callme.h"

int main(int argc, char const *argv[]) {
	char pathname[256];
	sprintf(pathname, "%s/.callme", getenv("HOME"));
	FILE *fd;
	if ((fd = fopen(pathname, "r")) == NULL)
	{
		printf("Cannot access config\n");
		return 0;
	}
	int chat_id = 0;
	fscanf(fd, "%d\n", &chat_id);
	fclose(fd);

	if (chat_id == 0) {
		exit(1);
	}

	json_object *content_json;

	tg_start(&content_json, TG_TOKEN);
	tg_callback_bind((char *)"start", &remember_me);

	if (argc < 2) {
		return 1;
	}
	pid_t chpid;
	const char* argsv[argc + 2];
	int pip[2];
	if(pipe(pip) == -1)
	{
		fprintf(stderr, "Error creating pipe\n");
	}
	argsv[0] = "/bin/sh";
	argsv[1] = "-c";
	for (size_t i = 1; i < argc; i++) {
		argsv[i+1] = argv[i];
	}
	argsv[argc + 1] = 0;
	if(!(chpid = fork()))
	{
		close(pip[0]);
		dup2(pip[1],1); // 1 -> stdout
		execvp(argsv[0], (char*const*) argsv);
	}
	close(pip[1]);

	char buf[1];
	while(read(pip[0], buf, 1) > 0)
	{
		write(1, buf, 1); // 1 -> stdout
	}
	close(pip[0]);
	int status;
	waitpid(chpid, &status, WUNTRACED);
	tg_message_t answer;
	char msg_text[256];
	sprintf(msg_text, "Ur command \"");
	for (size_t i = 2; i < argc; i++) {
		sprintf(msg_text, "%s%s ", msg_text, argsv[i]);
	}
	sprintf(msg_text, "%s%s", msg_text, argsv[argc]);
	sprintf(msg_text, "%s\" done!", msg_text);
	strcpy(answer.text, msg_text);
	answer.chat_id = chat_id;
	tg_send_message(&answer);
	return 0;
}

int remember_me(tg_message_t *msg)
{
	tg_message_t answer;
	if (tg_get_command_arg(msg->text, answer.text))
		strcpy(answer.text, "U will recv msg when your command done");
	answer.chat_id = msg->chat_id;
	tg_send_message(&answer);

	printf("%s:%d start recv\n", getenv("USER"), msg->chat_id);
	char pathname[256];
	sprintf(pathname, "%s/.callme", getenv("HOME"));
	FILE *fd;
	if ((fd = fopen(pathname, "w+")) == NULL)
	{
		printf("Cannot access config\n");
		return 0;
	}
	fprintf(fd, "%d\n", msg->chat_id);
	fclose(fd);

	return 0;
}
