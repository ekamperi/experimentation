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

    /* Check argument count */
    if (argc != 2)
        errx(EXIT_FAILURE, "usage: %s <data.xml>\n", argv[0]);

    /* Read array contents from external XML file */
    pa = prop_array_internalize_from_file(argv[1]);
    if (pa == NULL)
        errx(EXIT_FAILURE, "prop_array_internalize_from_file() failed\n");

    /* Skim through every item of the array */
    for (i = 0; i < prop_array_count(pa); i++) {
        s = prop_string_cstring_nocopy(prop_array_get(pa, i));
        printf("%s\n", s);
    }

    /* We will now iterate through the items of the array,
     * but this time we will exploit a prop_array_iterator_t
     */
    pit = prop_array_iterator(pa);
    if (pit == NULL) {
        prop_object_release(pa);
        errx(EXIT_FAILURE, "prop_array_iterator() failed\n");
    }

    while((po = prop_object_iterator_next(pit)) != NULL)
        printf("%s\n", prop_string_cstring_nocopy(po));

    /* Release objects */
    prop_object_release(pa);
    prop_object_iterator_release(pit);

    return EXIT_SUCCESS;
}
