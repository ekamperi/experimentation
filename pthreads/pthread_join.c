#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 20

/* function prototypes */
void *threadfun(void *arg);

int main(void)
{
    pthread_t tid;
    int i, cnt[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) {
        cnt[i] = i;
        printf("Creating thread: %d\n", i);
      if (pthread_create(&tid, NULL, threadfun, (void *)&cnt[i])) {
          fprintf(stderr, "pthread_create() error\n");
          exit(EXIT_FAILURE);
      }
      if (pthread_join(tid, NULL)) {
          fprintf(stderr, "pthread_join() error\n");
          exit(EXIT_FAILURE);
      }
    }

   return EXIT_SUCCESS;
}

void *threadfun(void *arg)
{
    printf("Hello! I am thread: %d\n", *(int *) arg);
    pthread_exit(NULL);
}
