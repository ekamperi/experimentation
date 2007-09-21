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
    if ((*fsm = malloc(sizeof **fsm)) == NULL)
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
    /* There is no need to allocate memory for state's key,
       since this is done in state_init() */

    *state->st_key = key;

    /* Insert state to hash table */
    if (htable_insert(fsm->sttable, state->st_key, state) == HT_EXISTS)
        return FSM_EXISTS;

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

    /* Does this state exist in states' hash table ? */
    if ((state = htable_search(fsm->sttable, &stkey)) == NULL)
        return FSM_NOTFOUND;

    /* Set fsm to new state */
    fsm->cstate = state;

    return FSM_OK;
}

fsmret_t fsm_process_event(fsm_t *fsm, unsigned int evtkey, void *data)
{
    event_t *event;

    /* Can the current state handle the incoming event ? */
    if ((event = htable_search(fsm->cstate->evttable, &evtkey)) == NULL)
        return FSM_NOTFOUND;

    /* Execute appropriate action */
    if (event->evt_actionf != NULL)
        event->evt_actionf(data);

    /* Is the transition made to an existent state ? */
    if ((event->evt_newstate == NULL)
        || (htable_search(fsm->sttable, event->evt_newstate->st_key) == NULL))
            return FSM_NOTFOUND;

    /* Set new state */
    fsm->cstate = event->evt_newstate;

    return FSM_OK;
}

fsmret_t fsm_validate(const fsm_t *fsm)
{
    /* Is FSM empty of states ? */
    if (htable_get_used(fsm->sttable) == 0)
        return FSM_EMPTY;

    return FSM_CLEAN;
}

void fsm_export_to_dot(const fsm_t *fsm, FILE *fp)
{
    const hnode_t *pstnode;
    const hnode_t *pevtnode;
    unsigned int stpos;
    unsigned int evtpos;

    fprintf(fp, "digraph {\n");

    /* Traverse all states of FSM */
    pstnode = NULL;
    stpos = 0;
    while ((pstnode = htable_get_next_elm(fsm->sttable, &stpos, pstnode)) != NULL) {

        /* Traverse all events associated with the current state */
        pevtnode = NULL;
        evtpos = 0;
        while ((pevtnode = htable_get_next_elm(((state_t *)(pstnode->hn_data))->evttable, &evtpos, pevtnode)) != NULL) {
            printf("S%d -> S%d [label=\"E%d\"]\n",
                   *(unsigned int *)pstnode->hn_key,
                   *(unsigned int *)(((event_t *)pevtnode->hn_data)->evt_newstate->st_key),
                   *(unsigned int *)pevtnode->hn_key);
        }
    }

    fprintf(fp, "}\n");
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
