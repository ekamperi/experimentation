#include <stdio.h>
#include <stdlib.h>

#include "states.h"
#include "types.h"

void foo1(void *data) { printf("foo1()\n"); }
void foo2(void *data) { printf("foo2()\n"); }
void foo3(void *data) { printf("foo3()\n"); }
void foo4(void *data) { printf("foo4()\n"); }
void foo5(void *data) { printf("foo5()\n"); }


int main(void)
{
    state_t mystate, mystate2;
    unsigned int x = 1, y = 2, z = 3, a = 4, b = 5;

    state_init(&mystate, 2, 1);

    state_add_evt(&mystate, x, "event1", foo1, &mystate2);
    state_add_evt(&mystate, y, "event2", foo2, &mystate2);
    state_add_evt(&mystate, z, "event3", foo3, &mystate2);
    state_add_evt(&mystate, a, "event4", foo4, &mystate2);
    state_add_evt(&mystate, b, "event5", foo5, &mystate2);
    state_add_evt(&mystate, b, "event5", foo5, &mystate2);

    state_print_evts(&mystate);

    state_rem_evt(&mystate, b);

    printf("________________________\n");

    state_print_evts(&mystate);
    state_free(&mystate);

    return EXIT_SUCCESS;
}
