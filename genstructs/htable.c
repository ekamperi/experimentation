#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

typedef struct hnode {
    const char *hn_str;
    TAILQ_ENTRY(hnode) hn_next;
} hentry_t;

typedef struct htable {
    size_t ht_size;
    u_int ht_used;
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

void htable_init(htable_t *htable, size_t size)
{
    u_int i;

    if ((htable->ht_table = malloc(size * sizeof *htable)) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++)
        TAILQ_INIT(&htable->ht_table[i]);

    htable->ht_size = size;
    htable->ht_used = 0;
}

int htable_insert(htable_t *htable, const char *str)
{
    u_int hash;

    hash = mkhash(str);
}

u_int mkhash(const char *str)
{
    
    return hash;
}


int main(void)
{

    return EXIT_SUCCESS;
}
