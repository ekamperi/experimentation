#ifndef FSM_H
#define FSM_H

#include "types.h"

/* Function prototypes */
fsmret_t fsm_init(fsm_t **state, size_t size, unsigned int factor);
fsmret_t fsm_add_state(fsm_t *fsm, unsigned int key, state_t *state);
fsmret_t fsm_free(fsm_t *fsm);
void fsm_print_states(const fsm_t *fsm);
fsmret_t fsm_set_state(fsm_t *fsm, unsigned int stkey);
fsmret_t fsm_process_event(fsm_t *fsm, unsigned int evtkey);

#endif    /* FSM_H */
