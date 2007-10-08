#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    /* for time() in srand() */

#include "mpool.h"

mpret_t mpool_init(mpool_t **mpool, size_t maxlogsize, size_t minlogsize)
{
    blknode_t *pblknode;
    unsigned int i;

    /* Validate input */
    if (maxlogsize < minlogsize)
        return MP_EBADVAL;

    /* Allocate memory for memory pool data structure */
    if ((*mpool = malloc(sizeof **mpool)) == NULL)
        return MP_ENOMEM;

    (*mpool)->maxlogsize = maxlogsize;
    (*mpool)->minlogsize = minlogsize;
    (*mpool)->nblocks = (*mpool)->maxlogsize - (*mpool)->minlogsize + 1;
#ifdef MP_STATS
    (*mpool)->nsplits = 0;
    (*mpool)->nmerges = 0;
#endif

    /* Allocate the actual memory of the pool */
    DPRINTF(("Allocating %u bytes for pool\n", 1 << maxlogsize));
    DPRINTF(("maxlogsize = %u\tminlogsize = %u\tnblocks = %u\n",
             (*mpool)->maxlogsize,
             (*mpool)->minlogsize,
             (*mpool)->nblocks));
    if (((*mpool)->mem = malloc(1 << maxlogsize)) == NULL) {
        free(*mpool);
        return MP_ENOMEM;
    }

    /* Allocate memory for block lists */
    if (((*mpool)->blktable = malloc((*mpool)->nblocks * sizeof *(*mpool)->blktable)) == NULL) {
        free((*mpool)->mem);
        free(*mpool);
        return MP_ENOMEM;
    }

    /* Initialize block lists */
    for (i = 0; i < (*mpool)->nblocks; i++)
        LIST_INIT(&(*mpool)->blktable[i]);

    /*
     * Initially, before any storage has been requested,
     * we have a single available block of length 2^maxlogsize
     * in blktable[0].
     */
    if ((pblknode = malloc(sizeof *pblknode)) == NULL) {
        free((*mpool)->blktable);
        free((*mpool)->mem);
        free(*mpool);
        return MP_ENOMEM;
    }
    pblknode->ptr = (*mpool)->mem;
    pblknode->flags |= MP_NODE_AVAIL;    /* Mark as available */
    pblknode->logsize = maxlogsize;

    LIST_INSERT_HEAD(&(*mpool)->blktable[0], pblknode, next_block);

    mpool_printblks(*mpool);

    return MP_OK;
}

void *mpool_alloc(mpool_t *mpool, size_t size)
{
    blkhead_t *phead;
    blknode_t *pnode;
    blknode_t *pavailnode;
    blknode_t *pnewnode;
    unsigned int i, newpos;
    unsigned char flag;

    DPRINTF(("\n\n=======================================================\n\n"));
    DPRINTF(("Searching for block of bytes: %u\n", size));

    /*
     * Find the most suitable 2^j bytes block for the requested size of bytes.
     * The condition 2^j >= size must be satisfied for the smallest possible value
     * of j and the block must be marked as available ofcourse.
    */
    pavailnode = NULL;
    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Searcing block: %u\n", i));
        phead = &mpool->blktable[i];
        if ((pnode = LIST_FIRST(phead)) != NULL) {
            if ((unsigned)(1 << pnode->logsize) >= size) {
                LIST_FOREACH(pnode, phead, next_block) {
                    if (pnode->flags & MP_NODE_AVAIL) {
                        pavailnode = pnode;
                        goto NEXT_BLOCK;
                    }
                }
            }
        }
    NEXT_BLOCK:;
    }

    /* Failure, no available block */
    if (pavailnode == NULL) {
        DPRINTF(("No available block found\n"));
        return NULL;
    }
    DPRINTF(("Found block of bytes %u\n", 1 << pavailnode->logsize));

    /* Is a split required ? */
AGAIN:;
    DPRINTF(("size = %u\tp = %u\tp-1 = %u\n",
             size,
             1 << pavailnode->logsize,
             1 << (pavailnode->logsize - 1)));

    /*
     * We don't need to split the chunk we just found,
     * if one the following statements is true:
     *
     * - `size' bytes fit exactly in the chunk
     * - `size' bytes won't fit in the divided chunk
     * - `minlogsize' constraint will be violated if we split
     *
     * NOTE: log2(size/2) = log2(size) - log2(2) = log2(size) - 1
     */
    if ((size == (unsigned)(1 << pavailnode->logsize))
        || (size > (unsigned)(1 << (pavailnode->logsize - 1)))
        || (mpool->minlogsize > (pavailnode->logsize - 1))) {
        DPRINTF(("No split required\n"));
        pavailnode->flags &= ~MP_NODE_AVAIL;    /* Mark as no longer available */
        mpool_printblks(mpool);
        return pavailnode->ptr;
    }

    DPRINTF(("Splitting...\n"));
#ifdef MP_STATS
    mpool->nsplits++;    /* FIXME: print a message if it overflows */
#endif

    /* Remove old block */
    DPRINTF(("Removing old chunk from list\n"));
    LIST_REMOVE(pavailnode, next_block);
    mpool_printblks(mpool);

    pavailnode->logsize--;
    flag = pavailnode->flags;
    if (pavailnode->flags & MP_NODE_LR)
        pavailnode->flags |= MP_NODE_PARENT;
    else
        pavailnode->flags &= ~MP_NODE_PARENT;
    pavailnode->flags &= ~MP_NODE_LR;    /* Mark as left buddy */

    DPRINTF(("New size is now: %u bytes\n", 1 << pavailnode->logsize));

    newpos = mpool->maxlogsize - pavailnode->logsize;
    DPRINTF(("Moving old chunk to new position: %u\n", newpos));

    LIST_INSERT_HEAD(&mpool->blktable[newpos], pavailnode, next_block);
    mpool_printblks(mpool);

    /* Split */
    DPRINTF(("Will add new item with bytes: %u\n", 1 << pavailnode->logsize));
    if ((pnewnode = malloc(sizeof *pnewnode)) == NULL)
        return NULL;    /* ? */
    pnewnode->ptr = (char *)pavailnode->ptr + (1 << pavailnode->logsize);
    pnewnode->flags |= MP_NODE_AVAIL;    /* Mark as available */
    pnewnode->flags |= MP_NODE_LR;       /* Mark as right buddy */

    if (flag & MP_NODE_PARENT)
        pnewnode->flags |= MP_NODE_PARENT;
    else
        pnewnode->flags &= ~MP_NODE_PARENT;

    pnewnode->logsize = pavailnode->logsize;
    LIST_INSERT_HEAD(&mpool->blktable[newpos], pnewnode, next_block);
    mpool_printblks(mpool);

    goto AGAIN;
    /* Never reached */
}

void mpool_free(mpool_t *mpool, void *ptr)
{
    blkhead_t *phead;
    blknode_t *pnode, *pbuddy;
    unsigned int i, newpos;
    void *buddyptr;

    DPRINTF(("Freeing ptr: %p\n", ptr));

    /* Search all nodes to find the one that points to ptr */
    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Searching for ptr %p in block: %u\n", ptr, i));
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            if (pnode->ptr == ptr) {
                DPRINTF(("Found chunk at block: %u\tBlock has chunks with bytes: %u\n",
                         i, 1 << pnode->logsize));
                goto CHUNK_FOUND;
            }
        }
    }

 CHUNK_FOUND:;
    if (pnode->logsize == mpool->maxlogsize) {
        pnode->flags |= MP_NODE_AVAIL;
        return;
    }

    /* Calculate possible buddy of chunk */
    if (pnode->flags & MP_NODE_LR)
        buddyptr = (char *)pnode->ptr - (1 << pnode->logsize);
    else
        buddyptr = (char *)pnode->ptr + (1 << pnode->logsize);

    /*
     * Search buddy node with address `buddyptr'.
     * If there is indeed a buddy of `pnode', it will be in
     * the same linked list with the latter.
     *
     * NOTE: nodes that belong to the same linked list,
     * point to memory chunks with the same size.
     */
    DPRINTF(("Chunk: %p\tPossible buddy at: %p\n", pnode->ptr, buddyptr));
    DPRINTF(("Searching for node with address: %p\n", buddyptr));
    pbuddy = NULL;
    LIST_FOREACH(pbuddy, phead, next_block) {
        if (pbuddy->ptr == buddyptr) {
            DPRINTF(("Buddy found\n"));
            break;
        }
    }

    /*
     * If there is no buddy of `pnode' or if there is, but it's unavailable,
     * just free `pnode' and we are done
     */
    if (pbuddy == NULL || (pbuddy != NULL && ((pbuddy->flags & MP_NODE_AVAIL) == 0))) {
        DPRINTF(("Not found or found but unavailable\n"));
        DPRINTF(("Freeing chunk (marking it as available)\n"));
        pnode->flags |= MP_NODE_AVAIL;
        mpool_printblks(mpool);
        return;
    }
    /*
     * There is a buddy, and it's available for sure. Coalesce.
     *
     * So now we have the chunk we were told to free (`pnode'), and
     * it's buddy (`pbuddy').
     *
     * pnode will become the parent, by updating its member structures,
     * such as logsize and flags (availability, LR buddiness, and inheritance)
     * and pbuddy will be free'd() for real.
     *
     * */
    else {
        DPRINTF(("Buddy exists and it's available. Coalesce\n"));
#ifdef MP_STATS
        mpool->nmerges++;
#endif
        DPRINTF(("Removing chunk from old position (so as to reposition it)\n"));
        LIST_REMOVE(pnode, next_block);
        mpool_printblks(mpool);
        pnode->logsize++;
        pnode->flags |= MP_NODE_AVAIL;    /* Mark as available */

        /* `pnode' is left buddy */
        if ((pnode->flags & MP_NODE_LR) == 0) {
            if (pnode->flags & MP_NODE_PARENT)
                pnode->flags |= MP_NODE_LR;
            else
                pnode->flags &= ~MP_NODE_LR;

            if (pbuddy->flags & MP_NODE_PARENT)
                pnode->flags |= MP_NODE_PARENT;
            else
                pnode->flags &= ~MP_NODE_PARENT;
        }

        /* `pbuddy' is left buddy */
        if ((pbuddy->flags & MP_NODE_LR) == 0) {
            if (pbuddy->flags & MP_NODE_PARENT)
                pnode->flags |= MP_NODE_LR;
            else
                pnode->flags &= ~MP_NODE_LR;

            DPRINTF(("Adjusting... pnode->ptr = pbuddy->ptr\n"));
            pnode->ptr = pbuddy->ptr;
        }

        newpos = mpool->nblocks - pnode->logsize;
        phead = &mpool->blktable[newpos];

        DPRINTF(("Inserting chunk to new position\n"));
        LIST_INSERT_HEAD(phead, pnode, next_block);
        mpool_printblks(mpool);

        DPRINTF(("Removing buddy\n"));
        LIST_REMOVE(pbuddy, next_block);
        free(pbuddy);
        mpool_printblks(mpool);
        goto CHUNK_FOUND;
    }
}

void mpool_destroy(mpool_t *mpool)
{
    const blkhead_t *phead;
    blknode_t *pnode;
    unsigned int i;

    /* Free all nodes in all block lists */
    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        while ((pnode = LIST_FIRST(phead)) != NULL) {
            LIST_REMOVE(pnode, next_block);
            free(pnode);
        }
    }

    free(mpool->blktable);
    free(mpool->mem);
    free(mpool);
}

void mpool_printblks(const mpool_t *mpool)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    unsigned int i;

    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Block: %u\t", i));

        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            DPRINTF(("ch(ad = %p, by = %u, av = %d, lr = %d, pa = %d)\t",
                     pnode->ptr,
                     (unsigned) (1 << pnode->logsize),
                     pnode->flags & MP_NODE_AVAIL ? 1 : 0,
                     pnode->flags & MP_NODE_LR ? 1 : 0,
                     pnode->flags & MP_NODE_PARENT ? 1 : 0));
        }

        DPRINTF(("\n"));
    }
}

void mpool_stat_get_nodes(const mpool_t *mpool, size_t *avail, size_t *used)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    unsigned int i;

    *avail = 0;
    *used = 0;
    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            if (pnode->flags & MP_NODE_AVAIL)
                (*avail)++;
            else
                (*used)++;
        }
    }
}

void mpool_stat_get_bytes(const mpool_t *mpool, size_t *avail, size_t *used)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    unsigned int i;

    *avail = 0;
    *used = 0;
    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            if (pnode->flags & MP_NODE_AVAIL)
                *avail += 1 << pnode->logsize;
            else
                *used += 1 << pnode->logsize;
        }
    }
}

size_t mpool_stat_get_block_length(const mpool_t *mpool, size_t pos)
{
    const blknode_t *pnode;
    size_t length;

    if (pos >= mpool->nblocks)
        return 0;    /* FIXME: Better error handling */

    length = 0;
    LIST_FOREACH(pnode, &mpool->blktable[pos], next_block)
        length++;

    return length;
}

#ifdef MP_STATS
size_t mpool_stat_get_splits(const mpool_t *mpool)
{
    return mpool->nsplits;
}

size_t mpool_stat_get_merges(const mpool_t *mpool)
{
    return mpool->nmerges;
}
#endif

int main(void)
{
    mpool_t *mpool;
    char *p[10000];
    size_t an = 0, un = 0, ab = 0, ub = 0, me = 0, sp = 0;
    unsigned int i, j, S;

    srand(time(NULL));

    if (mpool_init(&mpool, 23, 3) == MP_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 10000; i++) {
        if ((p[i] = mpool_alloc(mpool, S = (1 << ((rand() % 7))))) == NULL)
            break;
        else {
            /*memset(p[i], 0, S);*/;
            if  (rand() % 2 == 0)
                mpool_free(mpool, p[i]);
        }
    }

    for (j = 0; j < mpool->nblocks; j++)
        printf("Block %u has length: %u\n", j, mpool_stat_get_block_length(mpool, j));

    /*
    for (j = 0; j < i; j++)
        mpool_free(mpool, p[j]);
    */

    mpool_stat_get_nodes(mpool, &an, &un);
    mpool_stat_get_bytes(mpool, &ab, &ub);
    me = mpool_stat_get_merges(mpool);
    sp = mpool_stat_get_splits(mpool);
    DPRINTF(("avail nodes = %u\tused nodes = %u\tfree(%%) = %f\n", an, un, 100.0 * an / (an + un)));
    DPRINTF(("avail bytes  = %u\tused bytes = %u\tfree(%%) = %f\n", ab, ub, 100.0 * ab / (ab + ub)));
    DPRINTF(("splits = %u\tmerges = %u\n", sp, me));

    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
