#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#include "htable.h"

htret_t htable_init(htable_t *htable, size_t size, size_t factor,
                    hashf_t *myhashf,
                    cmpf_t *mycmpf,
                    printf_t *myprintf)
{
    size_t i;

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
    htable->ht_hashf = myhashf;
    htable->ht_cmpf = mycmpf;
    htable->ht_printf = myprintf;

    return HT_OK;
}

void htable_free(htable_t *htable)
{
    hhead_t *phead;
    hnode_t *pnode;
    size_t i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        while ((pnode = TAILQ_FIRST(phead)) != NULL) {
            TAILQ_REMOVE(phead, pnode, hn_next);
            free(pnode);
        }
    }

    free(htable->ht_table);
}

htret_t htable_free_obj(htable_t *htable, void *key, htfree_t htfree)
{
    hhead_t *phead;
    hnode_t *pnode;
    size_t hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /*
     * Search across chain if there is an entry with the
     * key we are looking for. If there is, free its contents.
     */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next) {
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
            TAILQ_REMOVE(phead, pnode, hn_next);
            if (htfree & HT_FREEKEY)
                free(pnode->hn_key);
            if (htfree & HT_FREEDATA)
                free(pnode->hn_data);
            free(pnode);
            htable->ht_used--;
            return HT_OK;
        }
    }

    return HT_NOTFOUND;
}

void htable_free_all_obj(htable_t *htable, htfree_t htfree)
{
    hhead_t *phead;
    hnode_t *pnode;
    size_t i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        TAILQ_FOREACH(pnode, phead, hn_next) {
            if (htfree & HT_FREEKEY)
                free(pnode->hn_key);
            if (htfree & HT_FREEDATA)
                free(pnode->hn_data);
        }
    }
}

htret_t htable_grow(htable_t *htable)
{
    hhead_t *pcurhead, *pnewhead, *poldhead;
    hnode_t *pnode;
    size_t i, newhash, newsize;

    /*
     * Allocate memory for new hash table
     * The new hash table is 2 times bigger than the old one
     */
    newsize = htable->ht_size << 1;
    if ((pnewhead = malloc(newsize * sizeof *pnewhead)) == NULL)
        return HT_NOMEM;

    /* Initialize tailqs */
    for (i = 0; i < newsize; i++)
        TAILQ_INIT(&pnewhead[i]);

    poldhead = htable->ht_table;

    /*
     * Remove the entries from the old hash table,
     * rehash them in respect to the new hash table,
     * and add them to the new one
     */
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

htret_t htable_insert(htable_t *htable, void *key, void *data)
{
    hhead_t *phead;
    hnode_t *pnode;
    size_t hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is already an entry with the same key. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0)
            return HT_EXISTS;

    /* Allocate memory for new entry */
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return HT_NOMEM;
    pnode->hn_key = key;
    pnode->hn_data = data;

    TAILQ_INSERT_TAIL(phead, pnode, hn_next);

    /* If used items exceed limit, grow the table */
    if (++htable->ht_used > htable->ht_limit)
        htable_grow(htable);

    return HT_OK;
}

htret_t htable_remove(htable_t *htable, const void *key)
{
    hhead_t *phead;
    hnode_t *pnode;
    size_t hash;

    /* Calculate hash */
    hash = htable->ht_hashf(key);

    /* Search across chain if there is an entry with the
    key we are looking. If there is, delete it. */
    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    TAILQ_FOREACH(pnode, phead, hn_next) {
        if (htable->ht_cmpf(pnode->hn_key, key) == 0) {
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
    size_t hash;

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
    size_t i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        TAILQ_FOREACH(pnode, phead, hn_next)
             htable->ht_printf(pnode->hn_key, pnode->hn_data);
        if (TAILQ_FIRST(&htable->ht_table[i]) != NULL)
            printf("\n");
    }
}

size_t htable_get_size(const htable_t *htable)
{
    return htable->ht_size;
}

size_t htable_get_used(const htable_t *htable)
{
    return htable->ht_used;
}

void htable_traverse(const htable_t *htable, void (*pfunc)(void *data))
{
    const hhead_t *phead;
    const hnode_t *pnode;
    size_t i;

    for (i = 0; i < htable->ht_size; i++) {
        phead = &htable->ht_table[i];
        TAILQ_FOREACH(pnode, phead, hn_next)
            pfunc(pnode->hn_data);
    }
}

const hnode_t *htable_get_next_elm(const htable_t *htable, size_t *pos, const hnode_t *pnode)
{
    const hhead_t *phead;
    size_t i;

    /* Is pos out of bound ? If yes, return immediately */
    if (*pos > (htable->ht_size - 1))
        return NULL;

    /* Find first usable element, if we were not supplied with one */
    if (pos == 0 || pnode == NULL) {
        for (i = *pos; i < htable->ht_size; i++) {
            ++*pos;
            phead = &htable->ht_table[i];
            if (TAILQ_FIRST(phead) != NULL)
                return TAILQ_FIRST(phead);
        }
    }

    /* Are we on a chain ? */
    if (TAILQ_NEXT(pnode, hn_next) != NULL) {
        /* Don't increase pos since we are stack in an horizontal chain,
           being still at the same 'height' which is what pos represents anyway  */
        return TAILQ_NEXT(pnode, hn_next);
    }
    else {
        for (i = *pos; i < htable->ht_size; i++) {
            ++*pos;
            phead = &htable->ht_table[i];
            if (TAILQ_FIRST(phead) != NULL)
                return TAILQ_FIRST(phead);
        }
        /* We have traversed all elements. Nothing left. */
        return NULL;
    }
}
