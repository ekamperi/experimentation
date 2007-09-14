#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

void foo1(void *data);
void foo2(void *data);

void foo1(void *data) { printf("foo1()\n"); }
void foo2(void *data) { printf("foo2()\n"); }

#define NSTATES 10
#define NEVENTS 10

int main(void)
{
    state_t *state[NSTATES];
    fsm_t *fsm;
    unsigned int i, j;
    void (*foo[2])(void *) = { foo1, foo2 };

    /* Initialize states */
    printf("Iniatilizing states\n");
    for (i = 0; i < NSTATES; i++)
        state_init(&state[i], 2<<5, 3);

    /* Construct state transition table */
    printf("Constructing state transition table\n");
    for (i = 0; i < NSTATES; i++)
        for (j = 0; j < NEVENTS; j++)
            state_add_evt(state[i], j, "e", foo[j % 2], state[i]);

    /* Initialize fsm */
    printf("Initializing fsm\n");
    fsm_init(&fsm, 2<<5, 3);

    /* Add states */
    printf("Adding states to fsm\n");
    for (i = 0; i < NSTATES; i++)
        fsm_add_state(fsm, i, state[i]);

    /* Set initial state */
    fsm_set_state(fsm, 0);

    printf("Processing events\n");
    for (i = 0; i < NEVENTS; i++)
        fsm_process_event(fsm, i);

    printf("Freeing\n");
    /* Free memory */
    for (i = 0; i < NSTATES; i++)
        state_free(state[i]);
    fsm_free(fsm);

    return EXIT_SUCCESS;
}
