#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    if ((pid = fork()) < 0) {  /* fork error */
        perror("fork");
        exit(EXIT_FAILURE);
    }

    else if (pid > 0)          /* parent process */
        printf("Parent's pid: %d\n", getpid());

    else if (pid == 0)         /* child process */
        printf("Child's pid: %d\n", getpid());

   return EXIT_SUCCESS;
}
