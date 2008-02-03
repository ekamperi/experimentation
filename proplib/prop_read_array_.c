/*
 * Compile with:
 * gcc prop_read_array.c -o prop_read_array -lprop -Wall -W -Wextra -ansi -pedantic
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

int main(int argc, char *argv[])
{
    prop_array_t pa;
    prop_object_t po;
    prop_object_iterator_t pit;
    unsigned int i;
    const char *s;

    /* No effect in NetBSD, but increases portability */
    setprogname(argv[0]);

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <data.xml>\n", getprogname());
        exit(EXIT_FAILURE);
    }

    /* Read array contents from external XML file */
    pa = prop_array_internalize_from_file(argv[1]);
    if (pa == NULL)
        err(EXIT_FAILURE, "prop_array_internalize_from_file()");

    /* Skim through every item of the array */
    for (i = 0; i < prop_array_count(pa); i++) {
        /* Get the object stored at the i-th array index */
        po = prop_array_get(pa, i);

        /* Just a sanity check */
        if (prop_object_type(po) != PROP_TYPE_STRING) {
            prop_object_release(pa);
            errx(EXIT_FAILURE, "invalid string type");
        }
        s = prop_string_cstring_nocopy(po);
        printf("%s\n", s);
    }

    /*
     * We will now iterate through the items of the array,
     * but this time we will exploit a prop_array_iterator_t
     */
    pit = prop_array_iterator(pa);
    if (pit == NULL) {
        prop_object_release(pa);
        err(EXIT_FAILURE, "prop_array_iterator()");
    }

    /* Traverse */
    while((po = prop_object_iterator_next(pit)) != NULL) {
        s = prop_string_cstring_nocopy(po);
        printf("%s\n", s);
    }

    /* Release objects */
    prop_object_release(pa);
    prop_object_iterator_release(pit);

    return EXIT_SUCCESS;
}
