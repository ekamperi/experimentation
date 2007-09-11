#ifndef FSM_H
#define FSM_H

#include "types.h"

/* Function prototypes */
fsmret_t fsm_init(fsm_t **state, size_t size, unsigned int factor);
fsmret_t fsm_add_state(fsm_t *fsm, unsigned int key, state_t *state);
fsmret_t fsm_free(fsm_t *fsm);
void fsm_print_states(const fsm_t *fsm);


/* Callback funtions */
unsigned int fsm_hashf(const void *key);
int fsm_cmpf(const void *arg1, const void *arg2);
void fsm_printf(const void *key, const void *data);

#endif    /* FSM_H */
