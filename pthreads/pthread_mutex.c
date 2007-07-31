/* compile with:
   gcc pthread_mutex.c -o pthread_mutex -lpthread -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include  <unistd.h>

#define NUM_THREADS 5

int myglobal = 0;		/* global shared variable */
pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;

/* function prototypes */
void *threadfun(void *arg);

int main() {   
   pthread_t tid[NUM_THREADS];
   int i;

   /* create the threads */
   for (i=0; i<NUM_THREADS; i++) {
      if (pthread_create(&tid[i], NULL, threadfun, NULL)) {
	 fprintf(stderr, "pthread_create() error\n");
	 exit(EXIT_FAILURE);
      }
   }

   /* make sure all threads are done */
   for (i=0; i<NUM_THREADS; i++) {
      if (pthread_join(tid[i], NULL)) {
	 fprintf(stderr, "pthread_join() error\n");
	 exit(EXIT_FAILURE);
      }
   }
   
   printf("myglobal = %d\n", myglobal);

   return EXIT_SUCCESS;
}

void *threadfun(void *arg) {
   int i, j;

   for (i=0; i<5; i++) {
      pthread_mutex_lock(&mymutex); /* Begin of critical region */
      j = myglobal;
      j++;
      sleep(1);			/*  */
      myglobal = j;
      pthread_mutex_unlock(&mymutex); /* End of critical region */
   }
   pthread_exit(NULL);
}

