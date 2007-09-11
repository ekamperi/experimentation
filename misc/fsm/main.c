#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

void foo1(void *data) { printf("foo1()\n"); }
void foo2(void *data) { printf("foo2()\n"); }

#define EVENT_ID 1
#define STATE1_ID 1
#define STATE2_ID 2

int main(void)
{
    state_t *state1, *state2;
    fsm_t *fsm;

    /* Initialize states */
    state_init(&state1, 2<<5, 2);
    state_init(&state2, 2<<5, 2);

    /* Construct state transition table */
    state_add_evt(state1, EVENT_ID, "event1", foo1, state2);
    state_add_evt(state2, EVENT_ID, "event2", foo2, state1);

    state_print_evts(state1);
    state_print_evts(state2);

    /* Initialize fsm */
    fsm_init(&fsm, 2<<5, 2);

    fsm_add_state(fsm, STATE1_ID, state1);
    fsm_add_state(fsm, STATE2_ID, state2);

    fsm_print_states(fsm);

    state_free(state1);
    state_free(state2);
    fsm_free(fsm);

    return EXIT_SUCCESS;
}
