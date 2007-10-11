#include <stdio.h>
#include <stdlib.h>

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
    if (((*mpool)->mem = malloc(1 << maxlogsize)) == NULL) {
        free(*mpool);
        return MP_ENOMEM;
    }
    DPRINTF(("maxlogsize = %u\tminlogsize = %u\tnblocks = %u\n" \
             "mpool->mem = %p\tsizeof(blknode) = 0x%x\n",
             (*mpool)->maxlogsize,
             (*mpool)->minlogsize,
             (*mpool)->nblocks,
             (*mpool)->mem,
             sizeof *pblknode));
    DPRINTF(("Allocated %u bytes for pool\n", 1 << maxlogsize));

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
    if ((unsigned) (1 << maxlogsize) > sizeof *pblknode)
        pblknode = (*mpool)->mem;
    else {
        free((*mpool)->blktable);
        free((*mpool)->mem);
        free(*mpool);
        return MP_ENOMEM;
    }
    pblknode->ptr = ((char *)(*mpool)->mem) + sizeof *pblknode;
    pblknode->flags |= MP_NODE_AVAIL;    /* Mark as available */
    pblknode->logsize = maxlogsize;

    /* Insert block to the appropriate list */
    LIST_INSERT_HEAD(&(*mpool)->blktable[0], pblknode, next_block);

    mpool_printblks(*mpool);

    return MP_OK;
}

void *mpool_alloc(mpool_t *mpool, size_t blksize)
{
    blkhead_t *phead;
    blknode_t *pnode;
    blknode_t *pavailnode;
    blknode_t *pnewnode;
    size_t size;
    unsigned int i, newpos;
    unsigned char flag;

    size = blksize + sizeof *pnode;

    DPRINTF(("\n\n=======================================================\n\n"));
    DPRINTF(("Searching for block of bytes: %u + %u = %u\n",
             blksize, sizeof *pnode, size));

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
        DPRINTF(("NEXT_BLOCK label\n"));
    }

    /* Failure, no available block */
    if (pavailnode == NULL) {
        DPRINTF(("No available block found\n"));
        return NULL;
    }
    DPRINTF(("Found block of bytes %u\n", 1 << pavailnode->logsize));

    /* Is a split required ? */
AGAIN:;
    DPRINTF(("AGAIN label\n"));
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
    DPRINTF(("Will add new item with bytes: %u (0x%x)\n", 1 << pavailnode->logsize, 1 << pavailnode->logsize));
    if ((unsigned) (1 << pavailnode->logsize) < sizeof *pnewnode)
        return NULL;
    pnewnode = (blknode_t *)((char *)pavailnode + (1 << pavailnode->logsize));
    pnewnode->ptr = (char *)pnewnode + sizeof(blknode_t);
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

    /*
     * Chunk isn't in our pool, this is bad.
     * Return immediately
     */
    DPRINTF(("Chunk point to %p was not found in the pool\n", ptr));
    return;

 CHUNK_FOUND:;
    DPRINTF(("CHUNK_FOUND label\n"));
    if (pnode->logsize == mpool->maxlogsize) {
        pnode->flags |= MP_NODE_AVAIL;
        return;
    }

    /* Calculate possible buddy of chunk */
    DPRINTF(("Searching for buddy...\n"));
    if (pnode->flags & MP_NODE_LR) {
        pbuddy = (blknode_t *)((char *)pnode - (1 << pnode->logsize));
        if ((void *)pbuddy < (void *)mpool->mem)
            pbuddy = NULL;
    }
    else {
        pbuddy = (blknode_t *)((char *)pnode + (1 << pnode->logsize));
        if ((void *)pbuddy > (void *)((char *)mpool->mem + (1 << mpool->maxlogsize) - 1))
            pbuddy = NULL;
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

        mpool_printblks(mpool);
        goto CHUNK_FOUND;
    }
}

void mpool_destroy(mpool_t *mpool)
{
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
        printf("Block: %u\t", i);

        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            printf("ch(ad = %p, by = %u, av = %d, lr = %d, pa = %d)\t",
                   pnode->ptr,
                   (unsigned) (1 << pnode->logsize),
                   pnode->flags & MP_NODE_AVAIL ? 1 : 0,
                   pnode->flags & MP_NODE_LR ? 1 : 0,
                   pnode->flags & MP_NODE_PARENT ? 1 : 0);
        }

        printf("\n");
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
                *avail += (1 << pnode->logsize) + sizeof *pnode;
            else
                *used += (1 << pnode->logsize) + sizeof *pnode;
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

