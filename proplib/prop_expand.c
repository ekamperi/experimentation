#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

#define INIT_CAPACITY 10
#define NUM_STRINGS 32
#define PRINT_STEP 5    /* Print stats every 5 steps */

int
main(int argc, char *argv[])
{
    prop_array_t pa;
    prop_string_t ps;
    unsigned int i;

    /* 
     * Create array object with initial capacity set to
     * ``INIT_CAPACITY''. The array will expand on demand
     * if its count exceeds its capacity.
     * This happens in prop_array_add() with ``EXPAND_STEP''
     * step, as defined in libprop/prop_array.c file.
     */
    pa = prop_array_create_with_capacity(INIT_CAPACITY);
    if (pa == NULL)
        errx(EXIT_FAILURE, "prop_array_create_with_capacity()");

    /* Create prop_string_t object */
    ps = prop_string_create_cstring("foo");
    if (ps == NULL) {
        prop_object_release(pa);
        errx(EXIT_FAILURE, "prop_string_create_cstring");
    }

    /*
     * Add up to ``NUM_STRINGS'' references to prop_string_t object
     * and watch if/how the array expands on demand.
     */
    for (i = 0; i < NUM_STRINGS; i++) {
        /* Print statistics every ``PRINT_STEP'' step */
        if (i % PRINT_STEP == 0) {
            printf("count = %u\tcapacity = %u\n",
                   prop_array_count(pa),
                   prop_array_capacity(pa));
        }

        /* Add object reference in array */
        if (prop_array_add(pa, ps) == FALSE) {
            prop_object_release(pa);
            prop_object_release(ps);
            errx(EXIT_FAILURE, "prop_array_add()");
        }
    }

    /* Release array object */
    prop_object_release(pa);
    prop_object_release(ps);

    return EXIT_SUCCESS;
}
