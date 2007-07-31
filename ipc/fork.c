#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
   pid_t pid;
   int a = 1;

   printf("START\n");

   if ((pid = fork()) < 0) {    /* fork error */
      fprintf(stderr, "fork() error\n");
      exit(EXIT_FAILURE);
   }
   else if (pid > 0) {		/* parent process */
      a += 2;
      printf("Parent's pid: %d\n", getpid());
      printf("Parent sees: %d\n", a);
   }
   else if (pid == 0) {		/* child process */
      a += 10;
      printf("Child's pid: %d\n", getpid());
      printf("Child sees: %d\n", a);
   }

   printf("%d\n", a);
   printf("xixixi\n");
  
   return 0;
} 
