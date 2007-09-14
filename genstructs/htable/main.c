#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "htable.h"

unsigned int djb_hash(const void *key)
{
    unsigned int i, hash = 5381;
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
    char *p = malloc(100);
    char *s = malloc(100);
    char *t = malloc(100);
    char *q = malloc(100);

    strcpy(p, "stathis");
    strcpy(s, "maria");
    strcpy(t, "kostas");
    strcpy(q, "eleni");

    /* Initialize table */
    if (htable_init(&htable, 1, 1, djb_hash, mystrcmp, myprintf) == HT_NOMEM) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    htable_insert(&htable, "stathis", p);
    htable_insert(&htable, "maria", s);
    htable_insert(&htable, "kostas", t);
    htable_insert(&htable, "eleni", q);
    htable_print(&htable);

    /* Free memory */
    htable_free_all_obj(&htable, HT_FREEDATA);
    htable_free(&htable);

    return EXIT_SUCCESS;
}
