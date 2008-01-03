#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

int main(int argc, char *argv[])
{
	prop_array_t pa;
	int i;

        /* Create array object with initial capacity 10
         * Note that the array will expand on demand
         * by the prop_array_add()
        */
	pa = prop_array_create_with_capacity(10);

	/* For every argument, create a reference to it
	 * and store it in the array
	 */
	for (i = 0; i < argc; i++)
		prop_array_add(pa, prop_string_create_cstring_nocopy(argv[i]));

	/* Export array contents to file as XML */
	prop_array_externalize_to_file(pa, "./data.xml");

	/* Release array object */
	prop_object_release(pa);

	return EXIT_SUCCESS;
}
