#include "types.h"
#include "states.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

stret_t state_init(state_t *state, size_t size, unsigned int factor)
{
    /* Allocate memory state's event table */
    if (htable_init(&state->evttable, size, factor,
                    state_hashf, state_cmpf, state_printf) == HT_NOMEM)
        return ST_NOMEM;

    return ST_OK;
}

stret_t state_add_evt(state_t *state, unsigned int* key, char *desc, void (*actionf)(void *data), state_t *newstate)
{
    event_t *evt;

    /* Allocate memory for new event */
    if ((evt = malloc(sizeof *evt)) == NULL)
        return ST_NOMEM;

    /* Fill in structure's members */
    strncpy(evt->evt_desc, desc, MAX_EVT_DESC);
    evt->evt_actionf = actionf;
    evt->evt_newstate = newstate;

    /* Insert event to hash table */
    htable_insert(&state->evttable, key, evt);

    return ST_OK;
}

stret_t state_rem_evt(state_t *state, unsigned int *key)
{
    if (htable_remove(&state->evttable, key) == HT_NOTFOUND)
        return ST_NOTFOUND;

    return ST_OK;
}

stret_t state_free(state_t *state)
{
    htable_free(&state->evttable);

    return ST_OK;
}

void state_print_evts(const state_t *state)
{
    htable_print(&state->evttable);
}

/* Callback funtions */
unsigned int state_hashf(const void *key)
{
    return *(unsigned int *)key;
}

int state_cmpf(const void *arg1, const void *arg2)
{
    unsigned int a = *(unsigned int *)arg1;
    unsigned int b = *(unsigned int *)arg2;

    if (a > b)
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

void state_printf(const void *key, const void *data)
{
    printf("key: %d\tdesc: %s ",
           *(unsigned int *)key,
           ((event_t*)data)->evt_desc);
}
