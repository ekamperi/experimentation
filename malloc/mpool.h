#include <sys/queue.h>

typedef struct blknode {
    unsigned char avail;    /* 1 = available, 0 = reserved */
    size_t logsize;         /* logarithm of size with base 2 */
    void *ptr;
    LIST_ENTRY(blknode) next_block;
} blknode_t;

typedef struct mpool {
    void *mem;
    size_t logsize;    /* logarithm of size with base 2  */
    size_t minsize;    /* minimum size of chunk in bytes */
    LIST_HEAD(blkhead, blknode) *blktable;
} mpool_t;

typedef struct blkhead blkhead_t;

typedef enum {
    MP_OK,
    MP_ENOMEM
} mpret_t;

/* Function prototypes */
mpret_t mpool_init(mpool_t **mpool, size_t logsize, size_t minsize);
void *mpool_alloc(mpool_t *mpool, size_t size);
void mpool_free(void *ptr, size_t size);
void mpool_destroy(mpool_t *mpool);

void mpool_printblks(const mpool_t *mpool);

