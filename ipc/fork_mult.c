/* compile with:
   gcc fork_mult.c -o fork_mult -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_CHILDS 5

int main(void)
{
    pid_t pid[NUM_CHILDS];
    int i;

    for (i = 0; i < NUM_CHILDS; i++) {
        if ((pid[i] = fork()) < 0) {    /* fork error */
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] > 0) {          /* parent process */
        }
        else if (pid[i] == 0) {         /* child process */
            printf("Child[%d] = %d\n", i, getpid());

            /* break, or else every child will spawn it's own children */
            break;
        }
    }
   return EXIT_SUCCESS;
}
