#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* for memset() */

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
unsigned int get_evt_key(const fsm_t *fsm, char **p);
void print_char(void *data);

/*
 * get_evt_key() works like an "event generator".
 * It examines the current state of FSM along with
 * the input we feed it, and then generates an
 * appropriate event.
 *
 * fsm_process_event() acts as an "event consumer",
 * handling each one of the previously generated events.
 */
unsigned int get_evt_key(const fsm_t *fsm, char **p)
{
    unsigned int stkey;

    /* Get current state of FSM */
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
            return EVT_END_COMMENT;
        }
        else {
            *p += 1;
            return EVT_NO_END_COMMENT;
        }
    }

    /* Normally, this would never be reached */
    *p += 1;
    return -1;
}

void print_char(void *data)
{
    printf("%c", *(char *)data);
}

int main(int argc, char *argv[])
{
    char buf[256];    /* must be big enough, or else beginning of comment might split */
    state_t *st_no_comment;
    state_t *st_comment;
    fsm_t *fsm;
    FILE *fp;
    char *p;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file.c\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open file to parse */
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    /*
     * In the following code, normally, we would have to check the
     * return values of the API calls, e.g state_init(),
     * state_add_evt(), fsm_init(), etc.
     *
     * Since the purpose of this test case is to demonstrate the
     * integration of the FSM API with an everyday program, we will
     * skip the checks, gaining in clarity and losing somewhat in
     * technical correctness.
     *
     * Initialize states
     */
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

    /* Parse file */
    while (!feof(fp)) {
        memset(buf, 0, sizeof buf);
        if (fgets(buf, sizeof buf, fp) == NULL && !feof(fp)) {
            perror("fgets");
            break;
        }
        p = buf;
        while (*p != '\0')
            fsm_process_event(fsm, get_evt_key(fsm, &p), p);
    }

    /* Free memory */
    state_free(st_no_comment);
    state_free(st_comment);

    fsm_free(fsm);

    /* Close file */
    (void)fclose(fp);

    return EXIT_SUCCESS;
}
