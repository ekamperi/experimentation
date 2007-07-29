#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CUSTOMERS 10
#define MAX_FREESEATS 2

sem_t barsem;
sem_t cussem;
sem_t seasem;   /* mutual exclusion for 'freeseats' */

unsigned int freeseats = MAX_FREESEATS;

/* function prototypes  */
void *barthread(void *arg);
void *custhread(void *arg);

int main() {
   pthread_t bartid;
   pthread_t custid[NUM_CUSTOMERS];
   int i;

   /* initialize the semaphores */
   sem_init(&barsem, 0, 0);
   sem_init(&cussem, 0, 0);
   sem_init(&seasem, 0, 1);

   /* create the barber thread */
   if (pthread_create(&bartid, NULL, barthread, NULL)) {
      perror("pthread_create() error");
      exit(EXIT_FAILURE);
   }

   /* create the customer threads */
   for (i=0; i<NUM_CUSTOMERS; i++) {
      if (pthread_create(&custid[i], NULL, custhread, NULL)) {
	 perror("pthread_create() error");
	 exit(EXIT_FAILURE);
      }
   }

   pthread_join(bartid, NULL);   /* wait for the barber to retire :) */
   return EXIT_SUCCESS;
}

void *barthread(void *arg) {  
   while (1) {
      printf("ZZZzzz\n");
      sem_wait(&cussem);
      sem_wait(&seasem);
      freeseats++;
      sem_post(&barsem);
      sem_post(&seasem);
      printf("The barber is cutting hair\n");
   }
   pthread_exit(NULL);
}

void *custhread(void *arg) {
   printf("Customer has arrived\n");
   sem_wait(&seasem);   
   if (freeseats > 0) {
      freeseats--;
      sem_post(&cussem);
      sem_post(&seasem);
      sem_wait(&barsem);
   }
   else {
      printf("No free seats - customer leaving\n");
      sem_post(&seasem);      
   }
  
   pthread_exit(NULL);
}
