#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

typedef struct hnode {
    const void *hn_key;
    void *hn_data;
    TAILQ_ENTRY(hnode) hn_next;
} hnode_t;

typedef struct htable {
    size_t ht_size;
    u_int ht_used;
    u_int (*ht_hashf)(const void *);
    int (*ht_cmpf)(const void *, const void *);
    void (*ht_printf)(void *);
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

/* */
void htable_init(htable_t *htable, size_t size);
void htable_insert(htable_t *htable, const void *key);
u_int htable_mkhash(const char *str);

void htable_init(htable_t *htable, size_t size)
{
    u_int i;

    if ((htable->ht_table = malloc(size * sizeof(htable->ht_table))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++)
        TAILQ_INIT(&htable->ht_table[i]);

    htable->ht_size = size;
    htable->ht_used = 0;
}

void htable_insert(htable_t *htable, const void *key)
{
    struct htablehead *phead;
    hnode_t *pnode;
    u_int hash;

    hash = htable->ht_hashf(key);

    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return;
    pnode->hn_key = key;

    TAILQ_INSERT_TAIL(phead, pnode, hn_next);
}

void *htable_search(htable_t *htable, const void *key)
{
    hnode_t *pnode;
    u_int hash;

    hash = htable->ht_hashf(key);

    TAILQ_FOREACH(pnode, &htable->ht_table[hash & (htable->ht_size - 1)], hn_next)
        if (htable->ht_cmpf(pnode->hn_key, key) == 0)
            return pnode->hn_data;

    return NULL;
}

void htable_print(htable_t *htable)
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

u_int htable_mkhash(const char *str)
{
    /* DJB hashing */
    u_int i, hash = 5381;

    for (i = 0; i < strlen(str); i++)
        hash = ((hash << 5) + hash) + str[i];

    return (hash & 0x7FFFFFFF);
}

int main(void)
{
    htable_t ptable;

    htable_init(&ptable, 10);

    htable_insert(&ptable, "stathis");
    htable_insert(&ptable, "maria");
    htable_insert(&ptable, "kostas");
    htable_insert(&ptable, "panagiotopoulos");
    htable_insert(&ptable, "eleni");

    htable_print(&ptable);

    return EXIT_SUCCESS;
}
