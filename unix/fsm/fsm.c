#include "fsm.h"

#include "states.h"
#include "types.h"

#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Callback funtions prototypes */
static unsigned int fsm_hashf(const void *key);
static int fsm_cmpf(const void *arg1, const void *arg2);
static void fsm_printf(const void *key, const void *data);
static void fsm_pq_lock(const fsm_t *fsm);
static void fsm_pq_unlock(const fsm_t *fsm);

fsmret_t fsm_init(fsm_t **fsm, size_t size, unsigned int factor,
                  unsigned int nqueues)
{
    unsigned int i;

    /* FIXME: Validate input  */

    /* Allocate memory for fsm data structure */
    if ((*fsm = malloc(sizeof **fsm)) == NULL)
        return FSM_ENOMEM;

    /* Allocate memory for fsm's states' hash table */
    if (((*fsm)->sttable = malloc(sizeof *(*fsm)->sttable)) == NULL) {
        free(*fsm);
        return FSM_ENOMEM;
    }

    /* Allocate memory for priority queues */
    if (((*fsm)->pqtable = malloc(nqueues * sizeof *(*fsm)->pqtable)) == NULL) {
        free((*fsm)->sttable);
        free(*fsm);
        return FSM_ENOMEM;
    }

    /* Allocate memory for "mutex object" -- machine dependent code */
    if (((*fsm)->mobj = malloc(sizeof(pthread_mutex_t))) == NULL) {
        free((*fsm)->mobj);
        free((*fsm)->sttable);
        free(*fsm);
        return FSM_ENOMEM;
    }
    /* Machine dependent code */
    pthread_mutex_init((pthread_mutex_t *)(*fsm)->mobj, NULL);

    /* Initialize queues */
    (*fsm)->nqueues = nqueues;
    for (i = 0; i < nqueues; i++)
        STAILQ_INIT(&(*fsm)->pqtable[i]);

    /* Initialize states' hash table */
    if (htable_init((*fsm)->sttable, size, factor,
                    fsm_hashf, fsm_cmpf, fsm_printf) == HT_NOMEM) {
        free((*fsm)->pqtable);
        free((*fsm)->sttable);
        free(*fsm);
        return FSM_ENOMEM;
    }

    return FSM_OK;
}

fsmret_t fsm_add_state(fsm_t *fsm, unsigned int key, state_t *state)
{
    /*
     * There is no need to allocate memory for state's key,
     * since this is done in state_init().
     */
    *state->st_key = key;

    /* Insert state to hash table */
    if (htable_insert(fsm->sttable, state->st_key, state) == HT_EXISTS)
        return FSM_EEXISTS;

    return FSM_OK;
}

fsmret_t fsm_free(fsm_t *fsm)
{
    pqhead_t *phead;
    pqnode_t *pnode;
    htable_iterator_t sit;    /* states iterator */
    unsigned int i;

    /* Free states' table */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(fsm->sttable, &sit)) != NULL)
        state_free(sit.pnode->hn_data);

    /* Shallow free */
    htable_free_all_obj(fsm->sttable, HT_FREEKEY);
    htable_free(fsm->sttable);
    free(fsm->sttable);

    /* Free queues' elements */
    for (i = 0; i < fsm->nqueues; i++) {
        phead = &fsm->pqtable[i];
        while (STAILQ_FIRST(phead) != NULL) {
            pnode = STAILQ_FIRST(phead);
            STAILQ_REMOVE_HEAD(phead, pq_next);
            free(pnode->data);
            free(pnode);
        }
    }

    free(fsm->pqtable);
    free(fsm->mobj);
    free(fsm);

    return FSM_OK;
}

fsmret_t fsm_set_state(fsm_t *fsm, unsigned int stkey)
{
    state_t *state;

    /* Does this state exist in states' hash table ? */
    if ((state = htable_search(fsm->sttable, &stkey)) == NULL)
        return FSM_ENOTFOUND;

    /* Set fsm to new state */
    fsm->cstate = state;

    return FSM_OK;
}

unsigned int fsm_get_current_state(const fsm_t *fsm)
{
    return *fsm->cstate->st_key;
}

fsmret_t fsm_queue_event(fsm_t *fsm, unsigned int evtkey,
                         void *data, size_t size, unsigned int prio)
{
    pqhead_t *phead;
    pqnode_t *pnode;

    if (prio >= fsm->nqueues)
        return FSM_EPRIO;

    /* Allocate memory for new pending event */
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return FSM_ENOMEM;

    pnode->evtkey = evtkey;
    pnode->prio = prio;

    /*
     * Allocate memory for data and copy them over.
     * Note that this strategy leads to memory fragmentation,
     * and should be addressed with a custom memory allocator,
     * in due time.
    */
    if ((pnode->data = malloc(size)) == NULL) {
        free(pnode);
        return FSM_ENOMEM;
    }
    memcpy(pnode->data, data, size);

    /* Get the head of the queue with the appropriate priority */
    phead = &fsm->pqtable[prio];

    /* Insert new event in tail (we serve from head) */
    fsm_pq_lock(fsm);
    STAILQ_INSERT_TAIL(phead, pnode, pq_next);
    fsm_pq_unlock(fsm);

    return FSM_OK;
}
fsmret_t fsm_dequeue_event(fsm_t *fsm)
{
    pqhead_t *phead;
    pqnode_t *pnode;
    unsigned int i;

    /* Scan queues starting from the one with the biggest priority */
    i = fsm->nqueues - 1;
    do {
        phead = &fsm->pqtable[i];
        if ((pnode = STAILQ_FIRST(phead)) != NULL) {
            if (fsm_process_event(fsm, pnode->evtkey, pnode->data) == FSM_ENOTFOUND) {
                /*
                 * FIXME: The event should stay in queue, if it has
                 * a sticky bit. But we haven't implemented such a bitmap
                 * in event's structure yet
                 */
            }

            /* Delete event */
            pnode = STAILQ_FIRST(phead);
            STAILQ_REMOVE_HEAD(phead, pq_next);
            free(pnode->data);    /* Argh, memory fragmentation */
            free(pnode);
            return FSM_OK;
        }
    } while (i-- != 0);

    return FSM_EMPTY;
}

size_t fsm_get_queued_events(const fsm_t *fsm)
{
    const pqhead_t *phead;
    const pqnode_t *pnode;
    size_t i, total;

    total = 0;
    for (i = 0; i < fsm->nqueues; i++) {
        phead = &fsm->pqtable[i];
        STAILQ_FOREACH(pnode, phead, pq_next)
            total++;
    }

    return total;
}

fsmret_t fsm_process_event(fsm_t *fsm, unsigned int evtkey, void *data)
{
    event_t *pevt;

    /* Can the current state handle the incoming event ? */
    if ((pevt = htable_search(fsm->cstate->evttable, &evtkey)) == NULL)
        return FSM_ENOTFOUND;

    /* Execute appropriate action */
    if (pevt->evt_actionf != NULL)
        pevt->evt_actionf(data);

    /* Is the transition made to an existent state ? */
    if ((pevt->evt_newstate == NULL)
        || (htable_search(fsm->sttable, pevt->evt_newstate->st_key) == NULL))
            return FSM_ENOTFOUND;

    /* Set new state */
    fsm->cstate = pevt->evt_newstate;

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
    const state_t *pstate;
    const event_t *pevt;
    htable_iterator_t sit;    /* state iterator */
    htable_iterator_t eit;    /* event iterator */

    fprintf(fp, "digraph {\n");

    /* Traverse all states of FSM */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(fsm->sttable, &sit)) != NULL) {
        /* Traverse all events associated with the current state */
        htable_iterator_init(&eit);
        pstate = sit.pnode->hn_data;
        while ((eit.pnode = htable_get_next_elm(pstate->evttable, &eit)) != NULL) {
            pevt = eit.pnode->hn_data;
            printf("S%u -> S%u [label=\"E%u\"]\n",
                   *(unsigned int *)sit.pnode->hn_key,
                   *(unsigned int *)(pevt->evt_newstate->st_key),
                   *(unsigned int *)eit.pnode->hn_key);
        }
    }

    fprintf(fp, "}\n");
}

void fsm_print_states(const fsm_t *fsm, FILE *fp)
{
    const state_t *pstate;
    htable_iterator_t sit;    /* states iterator */

    /* Traverse all states of FSM */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(fsm->sttable, &sit)) != NULL) {
        pstate = sit.pnode->hn_data;
        fprintf(fp, "state [key = %u]\n",
                *(unsigned int *)(pstate->st_key));
        state_print_evts(pstate, fp);
    }
}

void fsm_minimize(fsm_t *fsm)
{
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
    printf("key: %u ", *(const unsigned int *)key);
}

static void fsm_pq_lock(const fsm_t *fsm)
{
    /* Machine dependent code */
    pthread_mutex_lock((pthread_mutex_t *) fsm->mobj);
}

static void fsm_pq_unlock(const fsm_t *fsm)
{
    /* Machine dependent code */
    pthread_mutex_unlock((pthread_mutex_t *) fsm->mobj);
}
