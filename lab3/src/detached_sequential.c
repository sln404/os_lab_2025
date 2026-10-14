#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s seed arraysize\n", argv[0]);
    return 1;
  }

  pid_t fork_pid = fork();

  if (fork_pid < 0) {
    perror("fork failed\n");
    return 1;
  } 
  
  if (fork_pid == 0) {
    char *exec_args[] = {
        "./sequential_min_max",
        argv[1],
        argv[2],
        NULL
    };

    execvp(exec_args[0], exec_args);

    perror("execvp failed\n");
    exit(1);

  } else {
    int status;
    waitpid(fork_pid, &status, 0);
    return status;
  }

  return 0;
}
