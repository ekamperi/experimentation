#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

#define EVT_NO_SLASH_POINTER  1
#define EVT_NO_POINTER_SLASH  2
#define EVT_SLASH_POINTER     3
#define EVT_POINTER_SLASH     4

#define ST_NO_COMMENT         1
#define ST_COMMENT            2

/* Function prototypes */
void print_char(void *data);
unsigned int evt_get_key(const fsm_t *fsm, char **p);

unsigned int evt_get_key(const fsm_t *fsm, char **p)
{
    unsigned int stkey;

    /* Get current state's key of FSM */
    stkey = *fsm->cstate->st_key;

    if (stkey == ST_NO_COMMENT) {
        if (**p == '/' && (*p)[1] == '*') {
            *p += 2;
            return EVT_SLASH_POINTER;
        }
        else {
            *p += 1;
            return EVT_NO_SLASH_POINTER;
        }
    }
    else if (stkey == ST_COMMENT) {
        if (**p == '*' && (*p)[1] == '/') {
            *p += 2;
            return EVT_POINTER_SLASH;
        }
        else {
            *p += 1;
            return EVT_NO_POINTER_SLASH;
        }
    }

    /* Never reached */
    *p += 1;
    return -1;
}

void print_char(void *data)
{
    printf("%c", *(char *)data);
}

int main(void)
{
    state_t *st_no_comment;
    state_t *st_comment;
    fsm_t *fsm;
    char *p, str[] = "/*This /*is a*/ test*/";

    /* Initialize states */
    state_init(&st_no_comment, 2<<5, 2);
    state_init(&st_comment, 2<<5, 2);

    /* Construct state transition table */
    state_add_evt(st_no_comment, EVT_NO_SLASH_POINTER, "", print_char, st_no_comment);
    state_add_evt(st_no_comment, EVT_SLASH_POINTER, "", NULL, st_comment);

    state_add_evt(st_comment, EVT_NO_POINTER_SLASH, "", NULL, st_comment);
    state_add_evt(st_comment, EVT_POINTER_SLASH, "", NULL, st_no_comment);

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5);

    /* Add states */
    fsm_add_state(fsm, ST_NO_COMMENT, st_no_comment);
    fsm_add_state(fsm, ST_COMMENT, st_comment);

    /* Set initial state */
    fsm_set_state(fsm, ST_NO_COMMENT);

    p = str;
    while (*p != '\0')
        fsm_process_event(fsm, evt_get_key(fsm, &p), p);

    /* Free memory */
    state_free(st_no_comment);
    state_free(st_comment);

    fsm_free(fsm);

    return EXIT_SUCCESS;
}
