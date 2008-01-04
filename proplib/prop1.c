#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

int main(int argc, char *argv[])
{
    /* Declare a pointer to a prop_array object
     * Note that prop_array_t is a pointer being
     * hidden inside a typedef, i.e.
     * typedef struct _prop_array *prop_array_t;
     */
    prop_array_t pa;
    prop_string_t ps;
    int i;

    /* Create array object with initial capacity 10
     * Note that the array will expand on demand
     * by the prop_array_add()
     */
    pa = prop_array_create_with_capacity(10);
    if (pa == NULL)
        errx(EXIT_FAILURE, "prop_array_create_with_capacity() failed\n");

    /* For every argument, create a reference to it
     * and store it in the array
     */
    for (i = 0; i < argc; i++) {
        ps = prop_string_create_cstring_nocopy(argv[i]);
        if (ps == NULL) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_string_create_cstring_nocopy() failed\n");
        }

        if (prop_array_add(pa, ps) == FALSE) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "prop_array_add() failed\n");
        }
    }

    /* Export array contents to file as XML */
    if (prop_array_externalize_to_file(pa, "./data.xml") == FALSE) {
        prop_object_release(pa);
        errx(EXIT_FAILURE, "prop_array_externalize_to_file() failed\n");
    }

    /* Release array object */
    prop_object_release(pa);

    return EXIT_SUCCESS;
}
