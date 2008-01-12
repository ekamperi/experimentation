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

    /* Add up to `NUM_STRINGS' prop_string_t objects
     * and watch how the array expands on demand
     */
    for (i = 0; i < NUM_STRINGS; i++) {
        /* Print statistics */
        printf("count = %u\tcapacity = %u\n",
               prop_array_count(pa),
               prop_array_capacity(pa));
      
        /* Create prop_string_t object */
        ps = prop_string_create_cstring_nocopy("test");
        if (ps == NULL) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_string_create_cstring_nocopy()");
        }

        /* Add object in array */
        if (prop_array_add(pa, ps) == FALSE) {
            prop_object_release(ps);
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_array_add()");
        }
        
        prop_object_release(ps);
    }

    /* Release array object */
    prop_object_release(pa);

    return EXIT_SUCCESS;
}
