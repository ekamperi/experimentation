#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

typedef struct hnode {
    const char *hn_str;
    TAILQ_ENTRY(hnode) hn_next;
} hnode_t;

typedef struct htable {
    size_t ht_size;
    u_int ht_used;
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

/* */
void htable_init(htable_t *htable, size_t size);
void htable_insert(htable_t *htable, const char *str);
u_int htable_mkhash(const char *str);

void htable_init(htable_t *htable, size_t size)
{
    u_int i;

    if ((htable->ht_table = malloc(1000)) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /*    for (i = 0; i < size; i++)
        TAILQ_INIT(&htable->ht_table[i]);
    */
    /*    htable->ht_size = size;
    htable->ht_used = 0;
    */
}

void htable_insert(htable_t *htable, const char *str)
{
    struct htablehead *phead;
    hnode_t *pnode;
    u_int hash;

    hash = htable_mkhash(str);

    phead = &htable->ht_table[hash & (htable->ht_size - 1)];
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return;
    pnode->hn_str = str;

    TAILQ_INSERT_TAIL(phead, pnode, hn_next);
}

void htable_print(htable_t *htable)
{
    hnode_t *pnode;
    u_int i;

    for (i = 0; i < htable->ht_size; i++) {
        TAILQ_FOREACH(pnode, &htable->ht_table[i], hn_next)
            printf("%s ", pnode->hn_str);
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
    htable_t *ptable;

    htable_init(ptable, 10);

    /*    htable_insert(ptable, "stathis");
          htable_insert(ptable, "maria");*/

    /*htable_print(ptable);*/

    return EXIT_SUCCESS;
}
