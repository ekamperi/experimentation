/* compile with:
   gcc fork_execlp.c -o fork_execlp -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;
    int retval;

    if ((pid = fork()) < 0) {  /* fork error */
        perror("fork");
        exit(EXIT_FAILURE);
    }

    else if (pid > 0)          /* parent process */
        wait(&retval);

    else if (pid == 0) {       /* child process */
        if (execlp("date", "date", (char *)0) < 0) {
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    /* whatever goes here, will be executed from the
       parent AND the child process as well  */

   return EXIT_SUCCESS;
}
