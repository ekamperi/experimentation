#ifndef TYPES_H
#define TYPES_H

#include "htable.h"

#define MAX_EVT_DESC 64

typedef struct event {
    char evt_desc[MAX_EVT_DESC];
    void (*evt_actionf)(void *data);
    struct state *evt_newstate;
} event_t;

typedef struct state {
    htable_t *evttable;
    void *st_key;
} state_t;

typedef enum {
    ST_OK,
    ST_EXISTS,
    ST_NOMEM,
    ST_NOTFOUND
} stret_t;

typedef struct fsm {
    htable_t *sttable;
    state_t *cstate;    /* current state of fsm */
} fsm_t;

typedef enum {
    FSM_OK,
    FSM_EXISTS,
    FSM_NOMEM,
    FSM_NOTFOUND
} fsmret_t;

#endif    /* TYPES_H */
