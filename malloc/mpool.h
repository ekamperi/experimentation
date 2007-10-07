#include <sys/queue.h>

#define MP_DEBUG
#define MP_STATS

#ifdef MP_DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

#define MP_NODE_AVAIL  (1 << 0)    /* If not set, node is reserved, else available */
#define MP_NODE_LR     (1 << 1)    /* If not set, node is left buddy, else right buddy */
#define MP_NODE_PARENT (1 << 2)    /* If not set, parent is left buddy, else right buddy */

typedef struct blknode {
    unsigned char flags;    /* availability, left-right buddiness, inheritance */
    size_t logsize;         /* logarithm of size with base 2 */
    void *ptr;              /* pointer to beginning of free block (what mpool_alloc() returns) */
    LIST_ENTRY(blknode) next_block;
} blknode_t;

typedef struct mpool {
    void *mem;
    size_t nblocks;       /* nblocks = logsize + 1 */
    size_t maxlogsize;    /* logarithm of maximum size of chunk with base 2 */
    size_t minlogsize;    /* logarithm of minimum size of chunk with base 2 */
#ifdef MP_STATS
    size_t nsplits;       /* number of splits made */
    size_t nmerges;       /* number of merges made */
#endif
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
void mpool_free(mpool_t *mpool, void *ptr);
void mpool_destroy(mpool_t *mpool);

void mpool_printblks(const mpool_t *mpool);
void mpool_stat_get_nodes(const mpool_t *mpool, size_t *avail, size_t *used);
void mpool_stat_get_bytes(const mpool_t *mpool, size_t *avail, size_t *used);
#ifdef MP_STATS
size_t mpool_stat_get_splits(const mpool_t *mpool);
size_t mpool_stat_get_merges(const mpool_t *mpool);
#endif
