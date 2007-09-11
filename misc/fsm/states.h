#ifndef STATES_H
#define STATES_H

#include "types.h"

/* Function prototypes */
stret_t state_init(state_t **state, size_t size, unsigned int factor);
stret_t state_add_evt(state_t *state, unsigned int key, const char *desc, void (*actionf)(void *data), state_t *newstate);
stret_t state_rem_evt(state_t *state, unsigned int key);
stret_t state_free(state_t *state);
void state_print_evts(const state_t *state);

/* Callback funtions */
unsigned int state_hashf(const void *key);
int state_cmpf(const void *arg1, const void *arg2);
void state_printf(const void *key, const void *data);

#endif    /* STATES_H */
