#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htable.h"

u_int djb_hash(const void *key)
{
    u_int i, hash = 5381;
    char *str = (char *)key;

    for (i = 0; i < strlen(str); i++)
        hash = ((hash << 5) + hash) + str[i];

    return (hash & 0x7FFFFFFF);
}

int mystrcmp(const void *arg1, const void *arg2)
{
    return (strcmp((char *) arg1, (char *) arg2));
}

void myprintf(const void *key, const void *data)
{
    printf("%s(%s) ", (char *)key, (char *)data);
}

int main(void)
{
    htable_t htable;

    /* Initialize table */
    if (htable_init(&htable, 2<<5) == HT_NOMEM) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Setup callback functions */
    htable.ht_hashf = djb_hash;
    htable.ht_cmpf = mystrcmp;
    htable.ht_printf = myprintf;

    htable_insert(&htable, "stathis", "stathis");
    htable_insert(&htable, "maria", "maria");
    htable_insert(&htable, "kostas", "kostas");
    htable_insert(&htable, "panagiotopoulos", "panagiotopoulos");
    htable_insert(&htable, "eleni", "eleni");
    htable_print(&htable);

    htable_remove(&htable, "maria");
    htable_print(&htable);

    /* Free memory */
    htable_free(&htable);

    return EXIT_SUCCESS;
}
