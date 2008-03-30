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
    fsm_t *fsm;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 0);

    /* Initialize states */
    if (state_init(&state1, 2<<5, 2) == ST_NOMEM) {
        fsm_free(fsm);
        dief("state_init(): ST_NOMEM");
    }
    if (state_init(&state2, 2<<5, 2) == ST_NOMEM) {
        fsm_free(fsm);
        state_free(state1);
        dief("state_init(): ST_NOMEM");
    }

    /* Construct state transition table */
    if ((state_add_evt(state1,  0, "event0", NULL, state1) == ST_NOMEM) ||
        (state_add_evt(state1,  1, "event1", NULL, state2) == ST_NOMEM) ||
        (state_add_evt(state2,  0, "event0", NULL, state2) == ST_NOMEM) ||
        (state_add_evt(state2,  1, "event1", NULL, state1))) {
        dief("state_add_evt(): ST_NOMEM");
        fsm_free(fsm);
    }

    /* Add states */
    fsm_add_state(fsm, 1, state1);
    fsm_add_state(fsm, 2, state2);

    /* Set initial state */
    fsm_set_state(fsm, 1);

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
