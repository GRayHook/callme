#include "callme.h"


int main(int argc, char const *argv[]) {
  if (argc < 2) {
    return 1;
  }
  pid_t chpid;
  const char* argsv[argc];
  int pip[2];
  if(pipe(pip) == -1)
  {
    fprintf(stderr, "Error creating pipe\n");
  }
  for (size_t i = 1; i < argc; i++) {
    argsv[i - 1] = argv[i];
  }
  argsv[argc - 1] = 0;
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
  printf("hello %d\n", chpid);
  return 0;
}
