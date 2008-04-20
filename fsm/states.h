#ifndef STATES_H
#define STATES_H

#include "types.h"

/* Function prototypes */
stret_t state_init(state_t **state, size_t size, unsigned int factor);
stret_t state_add_evt(state_t *state, unsigned int key, const char *desc, void (*actionf)(void *data), state_t *newstate);
stret_t state_rem_evt(state_t *state, unsigned int key);
unsigned int state_get_key(state_t *state);
stret_t state_free(state_t *state);
void state_print_evts(const state_t *state, FILE *fp);

#endif    /* STATES_H */
