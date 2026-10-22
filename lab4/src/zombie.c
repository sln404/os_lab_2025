#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t child_pid = fork();

    if (child_pid > 0) {
        // Parent
        printf("Parent (PID: %d) sleeps...\n", getpid());
        sleep(30);
        printf("Parent wake up. Finishing.\n");
    } else if (child_pid == 0) {
        // Child
        printf("Child (PID: %d) finished immediately.\n", getpid());
        exit(0);
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
