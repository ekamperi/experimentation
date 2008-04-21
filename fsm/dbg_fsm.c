#include <assert.h>    /* Arg, ISO C99 only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* for memset() */

#include "fsm.h"
#include "states.h"
#include "types.h"

/* Function prototypes */
void dief(const char *p);

int main(int argc, char *argv[])
{
    state_t *state1;
    state_t *state2;
    state_t *state3;
    state_t *state4;
    fsm_t *fsm;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 0);

    /* Initialize states */
    assert(state_init(&state1, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state2, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state3, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state4, 2<<5, 2) != ST_NOMEM);

    /* Construct state transition table */
    assert(state_add_evt(state1, 0, "e0", NULL, state1));
    assert(state_add_evt(state1, 1, "e1", NULL, state2));
    assert(state_add_evt(state2, 0, "e0", NULL, state2));
    assert(state_add_evt(state2, 1, "e1", NULL, state1));
    assert(state_add_evt(state3, 0, "e0", NULL, state4));

    /* Add states */
    fsm_add_state(fsm, 1, state1);
    fsm_add_state(fsm, 2, state2);
    fsm_add_state(fsm, 3, state3);
    fsm_add_state(fsm, 4, state4);

    /* Set initial state */
    fsm_set_state(fsm, 1);

    /* Scan graph and mark reachable states */
    fsm_mark_reachable_states(fsm);

    /* Print state transition table */
    fsm_print_states(fsm, stdout);

    /* Free memory */
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

void dief(const char *p)
{
    fprintf(stderr, "error: %s\n", p);
    exit(EXIT_FAILURE);
}
