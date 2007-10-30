#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

#define EVT_NO_START_COMMENT  1
#define EVT_NO_END_COMMENT    2
#define EVT_START_COMMENT     3
#define EVT_END_COMMENT       4

#define ST_NO_COMMENT         1
#define ST_COMMENT            2

/* Function prototypes */
unsigned int evt_get_key(const fsm_t *fsm, char **p, unsigned int *depth);
void print_char(void *data);

unsigned int evt_get_key(const fsm_t *fsm, char **p, unsigned int *depth)
{
    unsigned int stkey;

    /* Get current state's key of FSM */
    stkey = fsm_get_current_state(fsm);

    if (stkey == ST_NO_COMMENT) {
        if (**p == '/' && (*p)[1] == '*') {
            *p += 2;
            return EVT_START_COMMENT;
        }
        else {
            *p += 1;
            return EVT_NO_START_COMMENT;
        }
    }
    else if (stkey == ST_COMMENT) {
        if (**p == '*' && (*p)[1] == '/') {
            *p += 2;
            if (*depth == 0)
                return EVT_END_COMMENT;
            else {
                (*depth)--;
                return EVT_NO_END_COMMENT;
            }
        }
        else if (**p == '/' && (*p)[1] == '*') {
            *p +=2 ;
            (*depth)++;
            return EVT_NO_END_COMMENT;
        }
        else {
            *p += 1;
            return EVT_NO_END_COMMENT;
        }
    }

    /* Normally, this would never reached */
    *p += 1;
    return -1;
}

void print_char(void *data)
{
    printf("%c", *(char *)data);
}

int main(void)
{
    char *p, str[] = "This is a test\n/*This /*is a*/ test*/Isn't it nice? :)\n";
    state_t *st_no_comment, *st_comment;
    fsm_t *fsm;
    unsigned int depth;

    /* Initialize states */
    state_init(&st_no_comment, 2<<5, 2);
    state_init(&st_comment, 2<<5, 2);

    /* Construct state transition table */
    state_add_evt(st_no_comment, EVT_NO_START_COMMENT, "", print_char, st_no_comment);
    state_add_evt(st_no_comment, EVT_START_COMMENT, "", NULL, st_comment);

    state_add_evt(st_comment, EVT_NO_END_COMMENT, "", NULL, st_comment);
    state_add_evt(st_comment, EVT_END_COMMENT, "", NULL, st_no_comment);

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 0);

    /* Add states */
    fsm_add_state(fsm, ST_NO_COMMENT, st_no_comment);
    fsm_add_state(fsm, ST_COMMENT, st_comment);

    /* Set initial state */
    fsm_set_state(fsm, ST_NO_COMMENT);

    /* Parse test string */
    p = str;
    depth = 0;
    while (*p != '\0')
        fsm_process_event(fsm, evt_get_key(fsm, &p, &depth), p);

    /* Free memory */
    state_free(st_no_comment);
    state_free(st_comment);

    fsm_free(fsm);

    return EXIT_SUCCESS;
}
