#include <stdio.h>
#include <stdlib.h>
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

    htable->ht_size = size;    /* size must be a power of 2 */
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

void htable_remove(htable_t *htable, const void *key)
{
    struct htablehead *phead;
    hnode_t *pnode;
    u_int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is an entry with the
    key we are looking. If there is, delete it. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
            TAILQ_REMOVE(phead, pnode, hn_next);
            return;
        }
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
    const hnode_t *pnode;
    u_int i;

    for (i = 0; i < htable->ht_size; i++) {
        TAILQ_FOREACH(pnode, &htable->ht_table[i], hn_next)
             htable->ht_printf(pnode->hn_key, pnode->hn_data);
        if (TAILQ_FIRST(&htable->ht_table[i]) != NULL)
            printf("\n");
    }
}
