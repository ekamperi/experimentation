#ifndef FSM_H
#define FSM_H

#include "types.h"
#include <stdio.h>    /* for FILE type */

/* Function prototypes */
fsmret_t fsm_init(fsm_t **state, size_t size, unsigned int factor, unsigned int nqueues);
fsmret_t fsm_add_state(fsm_t *fsm, unsigned int key, state_t *state);
fsmret_t fsm_free(fsm_t *fsm);
void fsm_print_states(const fsm_t *fsm);
fsmret_t fsm_set_state(fsm_t *fsm, unsigned int stkey);
unsigned int fsm_get_current_state(const fsm_t *fsm);
fsmret_t fsm_queue_event(fsm_t *fsm, unsigned int evtkey, void *data, size_t size, unsigned int prio);
fsmret_t fsm_dequeue_event(fsm_t *fsm);
size_t fsm_get_queued_events(const fsm_t *fsm);
fsmret_t fsm_process_event(fsm_t *fsm, unsigned int evtkey, void *data);
fsmret_t fsm_validate(const fsm_t *fsm);
void fsm_export_to_dot(const fsm_t *fsm, FILE *fp);

#endif    /* FSM_H */
