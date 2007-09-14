#include "fsm.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Callback funtions prototypes */
static unsigned int fsm_hashf(const void *key);
static int fsm_cmpf(const void *arg1, const void *arg2);
static void fsm_printf(const void *key, const void *data);

fsmret_t fsm_init(fsm_t **fsm, size_t size, unsigned int factor)
{
    /* Allocate memory fsm's state table */
    if ((*fsm = malloc(sizeof *fsm)) == NULL)
        return FSM_NOMEM;

    if (((*fsm)->sttable = malloc(sizeof *(*fsm)->sttable)) == NULL) {
        free(*fsm);
        return FSM_NOMEM;
    }

    if (htable_init((*fsm)->sttable, size, factor,
                    fsm_hashf, fsm_cmpf, fsm_printf) == HT_NOMEM) {
        free((*fsm)->sttable);
        free(*fsm);
        return FSM_NOMEM;
    }

    return FSM_OK;
}

fsmret_t fsm_add_state(fsm_t *fsm, unsigned int key, state_t *state)
{
    unsigned int *pkey;

    /* Allocate memory for new key */
    if ((pkey = malloc(sizeof *pkey)) == NULL)
        return FSM_NOMEM;

    *pkey = key;

    /* Insert event to hash table */
    if (htable_insert(fsm->sttable, pkey, state) == HT_EXISTS) {
        free(pkey);
        return FSM_EXISTS;
    }

    return FSM_OK;
}

fsmret_t fsm_free(fsm_t *fsm)
{
    htable_free_all_obj(fsm->sttable, HT_FREEKEY);
    htable_free(fsm->sttable);
    free(fsm->sttable);
    free(fsm);

    return FSM_OK;
}

void fsm_print_states(const fsm_t *fsm)
{
    htable_print(fsm->sttable);
}

fsmret_t fsm_set_state(fsm_t *fsm, unsigned int stkey)
{
    state_t *state;

    /* Does this state existin in states' hash table ? */
    if ((state = htable_search(fsm->sttable, &stkey)) == NULL)
        return FSM_NOTFOUND;

    /* Set fsm to new state */
    fsm->cstate = state;

    return FSM_OK;
}

fsmret_t fsm_process_event(fsm_t *fsm, unsigned int evtkey)
{
    event_t *event;

    /* Can the current state handle the incoming event ? */
    if ((event = htable_search(fsm->cstate->evttable, &evtkey)) == NULL)
        return FSM_NOTFOUND;

    /* Execute appropriate action */
    event->evt_actionf(NULL);

    /* Set fsm to new state */
    fsm->cstate = event->evt_newstate;

    return FSM_OK;
}

/* Callback funtions */
static unsigned int fsm_hashf(const void *key)
{
    return *(const unsigned int *)key;
}

static int fsm_cmpf(const void *arg1, const void *arg2)
{
    unsigned int a = *(const unsigned int *)arg1;
    unsigned int b = *(const unsigned int *)arg2;

    if (a > b)
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

static void fsm_printf(const void *key, const void *data)
{
    printf("key: %d ",
           *(const unsigned int *)key);
}
