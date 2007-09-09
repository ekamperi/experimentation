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
    htable_t evttable;
} state_t;

typedef enum {
    ST_OK,
    ST_NOMEM
} stret_t;

#endif    /* TYPES_H */
