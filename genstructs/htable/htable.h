#ifndef HTABLE_H
#define HTABLE_H

#include <sys/queue.h>
#include <stddef.h>    /* for size_t type */

typedef struct hnode {
    void *hn_key;
    void *hn_data;
    TAILQ_ENTRY(hnode) hn_next;
} hnode_t;

typedef struct htable {
    size_t ht_size;    /* size must be a power of 2 */
    unsigned int ht_used;
    unsigned int ht_factor;
    unsigned int ht_limit;
    unsigned int (*ht_hashf)(const void *key);
    int (*ht_cmpf)(const void *arg1, const void *arg2);
    void (*ht_printf)(const void *key, const void *data);
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

typedef struct htablehead hhead_t;

typedef enum {
    HT_OK,
    HT_EXISTS,
    HT_NOMEM,
    HT_NOTFOUND,
} htret_t;

/* Function prototypes */
htret_t htable_init(htable_t *htable, size_t size, unsigned int factor,
                    unsigned int (*hashf)(const void *key),
                    int (*cmpf)(const void *arg1, const void *arg2),
                    void (*printf)(const void *key, const void *data));
void htable_free(htable_t *htable);
htret_t htable_free_obj(htable_t *htable, void *key);
void htable_free_all_obj(htable_t *htable);
htret_t htable_grow(htable_t *htable);
htret_t htable_insert(htable_t *htable, void *key, void *data);
htret_t htable_remove(htable_t *htable, const void *key);
void *htable_search(const htable_t *htable, const void *key);
void htable_print(const htable_t *htable);

#endif    /* HTABLE_H */
