#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

#define NUM_STRINGS 50

int main(void)
{
    prop_array_t pa;
    prop_string_t ps;
    int i;

    /* Create array object with initial capacity 10
     * Note that the array will expand on demand
     * by the prop_array_add() with `EXPAND_STEP' step
     * as defined in libprop/prop_array.c
     */
    pa = prop_array_create_with_capacity(10);
    if (pa == NULL)
        errx(EXIT_FAILURE, "prop_array_create_with_capacity()");

    for (i = 0; i < NUM_STRINGS; i++) {
        printf("capacity = %u\n", prop_array_capacity(pa));
        ps = prop_string_create_cstring_nocopy("test");
        if (ps == NULL) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_string_create_cstring_nocopy()");
        }

        if (prop_array_add(pa, ps) == FALSE) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_array_add()");
        }
        
        prop_object_release(ps);
    }

    /* Release array object */
    prop_object_release(pa);

    return EXIT_SUCCESS;
}
