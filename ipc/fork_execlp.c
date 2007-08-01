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

    else if (pid == 0) {       /* child process */
        if (execlp("date", "date", (char *)0) < 0) {
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

   return EXIT_SUCCESS;
}
