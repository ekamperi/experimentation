#include <sys/queue.h>

typedef struct blknode {
    unsigned char avail;    /* 1 = available, 0 = reserved */
    size_t logsize;         /* logarithm of size with base 2 */
    void *ptr;
    LIST_ENTRY(blknode) next_block;
} blknode_t;

typedef struct mpool {
    void *mem;
    size_t nblocks;       /* nblocks = logsize + 1 */
    size_t maxlogsize;    /* logarithm of maximum size of chunk with base 2 */
    size_t minlogsize;    /* logarithm of minimum size of chunk with base 2 */
    LIST_HEAD(blkhead, blknode) *blktable;
} mpool_t;

typedef struct blkhead blkhead_t;

typedef enum {
    MP_OK,
    MP_EBADVAL,
    MP_ENOMEM
} mpret_t;

/* Function prototypes */
mpret_t mpool_init(mpool_t **mpool, size_t maxlogsize, size_t minlogsize);
void *mpool_alloc(mpool_t *mpool, size_t size);
void mpool_free(void *ptr, size_t size);
void mpool_destroy(mpool_t *mpool);

void mpool_printblks(const mpool_t *mpool);
void mpool_stat_get_nodes(const mpool_t *mpool, size_t *avail, size_t *used);
void mpool_stat_get_bytes(const mpool_t *mpool, size_t *avail, size_t *used);
