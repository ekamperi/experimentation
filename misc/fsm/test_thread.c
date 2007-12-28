#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

/* function prototypes */
void *thread_char_gen(void *arg);
void dief(const char *s);
void diep(const char *s);
void print_char(void *data);

int main(void)
{
    fsm_t *fsm;
    state_t *steadystate;
    pthread_t tid_char_gen;
    int c;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 1);

    /* Initialize state */
    if (state_init(&steadystate, 2<<8, 2) == ST_NOMEM) {
        fsm_free(fsm);
        dief("state_init(): ST_NOMEM");
    }

    /* Add events to state
     *
     * We have only one state named "steady state",
     * which handles all events from 'A' to 'Z'.
     */
    for (c = 'A'; c < 'Z'; c++) {
        if (state_add_evt(steadystate, c, "", print_char, steadystate) == ST_NOMEM) {
            dief("state_add_evt(): ST_NOMEM");
            fsm_free(fsm);
        }
    }

    /* Add steady state to fsm */
    fsm_add_state(fsm, 0, steadystate);

    /* Set initial state */
    fsm_set_state(fsm, 0);

    /* Create char generator thread (it acts as event generator) */
    if (pthread_create(&tid_char_gen, NULL, thread_char_gen, (void *)fsm))
        diep("pthread_create");

    /* Wait until all events have been queued */
    pthread_join(tid_char_gen, NULL);

    /* Main thread acts as event consumer */
    while (fsm_get_queued_events(fsm) != 0)
        fsm_dequeue_event(fsm);

    /* Free memory */
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

void *thread_char_gen(void *arg)
{
    fsm_t *fsm;
    size_t c, i;

    /* Get a pointer to the fsm we are interested in */
    fsm = (fsm_t *)arg;

    /* Initialize random number generator */
    srand(time(NULL));

    /* Broadcast events */
    for (i = 0; i < 100; i++) {
        c = 'A' + rand() % 26;    /* from 'A' to 'Z' */
        fsm_queue_event(fsm, c, &c, 1, 0);
    }

    pthread_exit(NULL);
}

void dief(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
    exit(EXIT_FAILURE);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

void print_char(void *data)
{
    printf("%c", *(char *)data);
}
