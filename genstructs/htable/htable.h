#ifndef HTABLE_H
#define HTABLE_H

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
    void (*ht_printf)(const void *);
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

/* Function prototypes */
void htable_init(htable_t *htable, size_t size);
void htable_insert(htable_t *htable, const void *key, void *data);
void *htable_search(const htable_t *htable, const void *key);
void htable_print(const htable_t *htable);
#endif    /* HTABLE_H */
