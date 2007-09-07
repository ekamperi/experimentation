#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "htable.h"

void htable_init(htable_t *htable, size_t size)
{
    u_int i;

    /* Allocate memory for `size' tailq headers */
    if ((htable->ht_table = malloc(size * sizeof *htable->ht_table)) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Initialize tailqs */
    for (i = 0; i < size; i++)
        TAILQ_INIT(&htable->ht_table[i]);

    htable->ht_size = size;
    htable->ht_used = 0;
}

void htable_insert(htable_t *htable, const void *key, void *data)
{
    struct htablehead *phead;
    hnode_t *pnode;
    u_int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is already an entry
       with the same key. If there is, replace it. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
            pnode->hn_data = data;
            return;
        }

    /* Allocate memory for new entry */
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return;
    pnode->hn_key = key;
    pnode->hn_data = data;

    TAILQ_INSERT_TAIL(phead, pnode, hn_next);
    htable->ht_used++;
}

void *htable_search(const htable_t *htable, const void *key)
{
    hnode_t *pnode;
    u_int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    TAILQ_FOREACH(pnode, &htable->ht_table[hash & (htable->ht_size - 1)], hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0)
            return pnode->hn_data;

    return NULL;
}

void htable_print(const htable_t *htable)
{
    hnode_t *pnode;
    u_int i;

    for (i = 0; i < htable->ht_size; i++) {
        TAILQ_FOREACH(pnode, &htable->ht_table[i], hn_next)
             htable->ht_printf(pnode->hn_data);
        if (TAILQ_FIRST(&htable->ht_table[i]) != NULL)
            printf("\n");
    }
}

u_int djb_hash(const void *str)
{
    /* DJB hashing */
    u_int i, hash = 5381;

    for (i = 0; i < strlen((char*)str); i++)
        hash = ((hash << 5) + hash) + ((char*)str)[i];

    return (hash & 0x7FFFFFFF);
}

int mycmp(const void *arg1, const void *arg2)
{
    return (strcmp((char *) arg1, (char *) arg2));
}

void myprintf(const void *data)
{
    printf("%s ", (char *)data);
}

int main(void)
{
    htable_t ptable;

    /* Initialize table */
    htable_init(&ptable, 10);

    /* Setup callback functions */
    ptable.ht_hashf = djb_hash;
    ptable.ht_cmpf = mycmp;
    ptable.ht_printf = myprintf;

    htable_insert(&ptable, "stathis", "stathis");
    htable_insert(&ptable, "maria", "maria");
    htable_insert(&ptable, "kostas", "kostas");
    htable_insert(&ptable, "panagiotopoulos", "panagiotopoulos");
    htable_insert(&ptable, "eleni", "eleni");

    htable_print(&ptable);

    return EXIT_SUCCESS;
}
