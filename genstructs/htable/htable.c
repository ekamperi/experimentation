#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "htable.h"

htret_t htable_init(htable_t *htable, size_t size, unsigned int factor,
                    unsigned int (*hashf)(const void *key),
                    int (*cmpf)(const void *arg1, const void *arg2),
                    void (*printf)(const void *key, const void *data))
{
    unsigned int i;

    /* Allocate memory for `size' tailq headers */
    if ((htable->ht_table = malloc(size * sizeof *htable->ht_table)) == NULL)
        return HT_NOMEM;

    /* Initialize tailqs */
    for (i = 0; i < size; i++)
        TAILQ_INIT(&htable->ht_table[i]);

    htable->ht_size = size;    /* size must be a power of 2 */
    htable->ht_used = 0;
    htable->ht_factor = factor;
    htable->ht_limit = factor * size;

    /* Setup callback functions */
    htable->ht_hashf = hashf;
    htable->ht_cmpf = cmpf;
    htable->ht_printf = printf;

    return HT_OK;
}

void htable_free(htable_t *htable)
{
    hhead_t *phead;
    hnode_t *pnode;
    unsigned int i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        while (TAILQ_FIRST(phead) != NULL) {
            pnode = TAILQ_FIRST(phead);
            TAILQ_REMOVE(phead, TAILQ_FIRST(phead), hn_next);
            free(pnode);
        }
    }

    free(htable->ht_table);
}

void htable_free_objects(htable_t *htable)
{
    hhead_t *phead;
    hnode_t *pnode;
    unsigned int i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        TAILQ_FOREACH(pnode, phead, hn_next) {
            free(pnode->hn_key);
            free(pnode->hn_data);
        }
    }
}

htret_t htable_grow(htable_t *htable)
{
    hhead_t *pcurhead, *pnewhead, *poldhead;
    hnode_t *pnode;
    unsigned int i, newhash, newsize;

    /* Allocate memory for new hash table */
    newsize = 2 * htable->ht_size;
    if ((pnewhead = malloc(newsize * sizeof *pnewhead)) == NULL)
        return HT_NOMEM;

    /* Initialize tailqs */
    for (i = 0; i < newsize; i++)
        TAILQ_INIT(&pnewhead[i]);

    poldhead = htable->ht_table;

    for (i = 0; i < htable->ht_size; i++) {
        pcurhead = &poldhead[i];
        while ((pnode = TAILQ_FIRST(pcurhead)) != NULL) {
            newhash = htable->ht_hashf(pnode->hn_key);
            TAILQ_REMOVE(pcurhead, pnode, hn_next);
            TAILQ_INSERT_TAIL(&pnewhead[newhash & (newsize - 1)], pnode, hn_next);
        }
    }

    /* Free old table */
    free(htable->ht_table);

    /* Set new table parameters */
    htable->ht_table = pnewhead;
    htable->ht_size = newsize;
    htable->ht_limit = htable->ht_factor * newsize;

    return HT_OK;
}

htret_t htable_insert(htable_t *htable, const void *key, void *data)
{
    hhead_t *phead;
    hnode_t *pnode;
    unsigned int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is already an entry
       with the same key. If there is, replace it.
       Don't forget to free the old data, before overwriting
       the pointer or else we will leak. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
            free(pnode->hn_key);
            free(pnode->hn_data);
            pnode->hn_key = key;
            pnode->hn_data = data;
            return HT_REPLACED;
        }

    /* Allocate memory for new entry */
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return HT_NOMEM;
    pnode->hn_key = key;
    pnode->hn_data = data;

    TAILQ_INSERT_TAIL(phead, pnode, hn_next);

    /* If used items exceeds limit, grow the table */
    if (++htable->ht_used > htable->ht_limit)
        htable_grow(htable);

    return HT_OK;
}

htret_t htable_remove(htable_t *htable, const void *key)
{
    hhead_t *phead;
    hnode_t *pnode, *tmp;
    unsigned int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is an entry with the
    key we are looking. If there is, delete it. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next) {
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
            tmp = pnode;
            TAILQ_REMOVE(phead, pnode, hn_next);
            free(pnode);
            htable->ht_used--;
            return HT_OK;
        }
    }

    return HT_NOTFOUND;
}

void *htable_search(const htable_t *htable, const void *key)
{
    const hhead_t *phead;
    const hnode_t *pnode;
    unsigned int hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0)
            return pnode->hn_data;

    return NULL;
}

void htable_print(const htable_t *htable)
{
    const hhead_t *phead;
    const hnode_t *pnode;
    unsigned int i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        TAILQ_FOREACH(pnode, phead, hn_next)
             htable->ht_printf(pnode->hn_key, pnode->hn_data);
        if (TAILQ_FIRST(&htable->ht_table[i]) != NULL)
            printf("\n");
    }
}
