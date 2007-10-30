#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

/* function prototypes */
void *thread_char_gen(void *arg);
void diep(const char *s);

int main(void)
{
    fsm_t *fsm;
    pthread_t tid_char_gen;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 1);

    /* Create char generator thread */
    if (pthread_create(&tid_char_gen, NULL, thread_char_gen, (void *)thread_char_gen)
        diep("pthread_create");

    /* Free memory */
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

void *thread_char_gen(void *arg)
{
    fsm_t *fsm;

    fsm = (fsm_t *)arg;

    

    pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
