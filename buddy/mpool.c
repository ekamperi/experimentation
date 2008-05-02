/*
 * This is an implementation of a buddy memory allocator.
 * For a description of such a system, please refer to:
 *
 * The Art of Computer Programming Vol. I, by Donald E. Knuth
 */

#include <stdio.h>
#include <stdlib.h>

#include "mpool.h"

/* Function prototypes */
static void mpool_printblks(const mpool_t *mpool);

mpret_t mpool_init(mpool_t **mpool, size_t maxlogsize, size_t minlogsize)
{
    blknode_t *pblknode;
    size_t i;

    /* Validate input */
    if (maxlogsize < minlogsize || (size_t)(1 << minlogsize) <= sizeof *pblknode)
        return MPOOL_EBADVAL;

    /* Allocate memory for memory pool data structure */
    if ((*mpool = malloc(sizeof **mpool)) == NULL)
        return MPOOL_ENOMEM;

    (*mpool)->maxlogsize = maxlogsize;
    (*mpool)->minlogsize = minlogsize;
    (*mpool)->nblocks = (*mpool)->maxlogsize - (*mpool)->minlogsize + 1;
#ifdef MPOOL_STATS
    (*mpool)->nsplits = 0;
    (*mpool)->nmerges = 0;
#endif

    /* Allocate the actual memory of the pool */
    if (((*mpool)->mem = malloc((size_t)(1 << maxlogsize))) == NULL) {
        free(*mpool);
        return MPOOL_ENOMEM;
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
        return MPOOL_ENOMEM;
    }

    /* Initialize block lists */
    for (i = 0; i < (*mpool)->nblocks; i++)
        LIST_INIT(&(*mpool)->blktable[i]);

    /*
     * Initially, before any storage has been requested,
     * we have a single available block of length 2^maxlogsize
     * in blktable[0].
     */
    /*
      pblknode = (*mpool)->mem;
      pblknode->ptr = (char *)(*mpool)->mem + sizeof *pblknode;
      MPOOL_MARK_AVAIL(pblknode);
      pblknode->logsize = maxlogsize;
    */
    MPOOL_BLOCK_INIT(pblknode,
                     (*mpool)->mem,
                     (char *)(*mpool)->mem + sizeof(blknode_t),
                     MPOOL_BLOCK_AVAIL,
                     MPOOL_BLOCK_LEFT,      /* irrelevant */
                     MPOOL_BLOCK_PARENT,    /* irrelevant */
                     maxlogsize);

    /* Insert block to the first block list */
    LIST_INSERT_HEAD(&(*mpool)->blktable[0], pblknode, next_chunk);

    mpool_printblks(*mpool);

    return MPOOL_OK;
}

void *mpool_alloc(mpool_t *mpool, size_t blksize)
{
    blkhead_t *phead;
    blknode_t *pnode;
    blknode_t *pavailnode;
    blknode_t *pnewnode;
    size_t i, newpos, size;
    unsigned char flag;

    /*
     * Total size is the sum of the user's request
     * plus the overhead of a blknode_t data structure.
     *
     * Be aware for the particular scenario, when
     * reqsize is 2^j. The allocator will return
     * the next bigger memory chunk, leading to high
     * internal fragmentation.
    */
    size = blksize + sizeof *pnode;

    DPRINTF(("\n\n=======================================================\n\n"));
    DPRINTF(("Searching for block of bytes: %u + %u = %u\n",
             blksize, sizeof *pnode, size));

    /*
     * Find the most suitable 2^j bytes block for the requested size of bytes.
     * The condition 2^j >= size must be satisfied for the smallest possible
     * value of j and the block must be marked as available ofcourse.
    */
    pavailnode = NULL;
    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Searching block: %u\n", i));
        phead = &mpool->blktable[i];
        if ((pnode = LIST_FIRST(phead)) != NULL) {
            if ((size_t)(1 << pnode->logsize) >= size) {
                LIST_FOREACH(pnode, phead, next_chunk) {
                    if (MPOOL_IS_AVAIL(pnode)) {
                        pavailnode = pnode;
                        goto NEXT_BLOCK_LIST;
                    }
                }
            }
        }
    NEXT_BLOCK_LIST:;
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
     * if one at least of the following statements is true:
     *
     * - `size' bytes fit exactly in the chunk
     * - `size' bytes won't fit in the splitted chunk
     * - `minlogsize' constraint will be violated if we split
     *
     * NOTE: log2(size/2) = log2(size) - log2(2) = log2(size) - 1
     */
    if ((size == (size_t)(1 << pavailnode->logsize))
        || (size > (size_t)(1 << (pavailnode->logsize - 1)))
        || (mpool->minlogsize > (pavailnode->logsize - 1))) {
        DPRINTF(("No split required\n"));
        MPOOL_MARK_USED(pavailnode);
        mpool_printblks(mpool);
        return pavailnode->ptr;
    }

    DPRINTF(("Splitting...\n"));
#ifdef MPOOL_STATS
    mpool->nsplits++;
#endif

    /* Remove old chunk */
    DPRINTF(("Removing old chunk from list\n"));
    LIST_REMOVE(pavailnode, next_chunk);
    mpool_printblks(mpool);

    pavailnode->logsize--;
    flag = pavailnode->flags;
    if (MPOOL_IS_RIGHT(pavailnode))
        pavailnode->flags |= MPOOL_NODE_PARENT;
    else
        pavailnode->flags &= ~MPOOL_NODE_PARENT;
    MPOOL_MARK_LEFT(pavailnode);

    DPRINTF(("New size is now: %u bytes\n", 1 << pavailnode->logsize));

    newpos = mpool->maxlogsize - pavailnode->logsize;
    DPRINTF(("Moving old chunk to new position: %u\n", newpos));

    LIST_INSERT_HEAD(&mpool->blktable[newpos], pavailnode, next_chunk);
    mpool_printblks(mpool);

    /* Split */
    DPRINTF(("Will add new item with bytes: %u (0x%x)\n",
             1 << pavailnode->logsize,
             1 << pavailnode->logsize));
    if ((size_t)(1 << pavailnode->logsize) < sizeof *pnewnode)
        return NULL;
    /*
      pnewnode = (blknode_t *)((char *)pavailnode + (1 << pavailnode->logsize));
      pnewnode->ptr = (char *)pnewnode + sizeof *pnewnode;
      MPOOL_MARK_AVAIL(pnewnode);
      MPOOL_MARK_RIGHT(pnewnode);

      if (flag & MPOOL_NODE_PARENT)
      pnewnode->flags |= MPOOL_NODE_PARENT;
      else
      pnewnode->flags &= ~MPOOL_NODE_PARENT;
      pnewnode->logsize = pavailnode->logsize;
    */
    MPOOL_BLOCK_INIT(pnewnode,
                     (blknode_t *)((char *)pavailnode + (1 << pavailnode->logsize)),
                     (char *)pnewnode + sizeof *pnewnode,
                     MPOOL_BLOCK_AVAIL,
                     MPOOL_BLOCK_RIGHT,
                     (flag & MPOOL_NODE_PARENT) ? MPOOL_BLOCK_PARENT : -1,
                     pavailnode->logsize);

    LIST_INSERT_HEAD(&mpool->blktable[newpos], pnewnode, next_chunk);
    mpool_printblks(mpool);

    goto AGAIN;
    /* Never reached */
}

void mpool_free(mpool_t *mpool, void *ptr)
{
    blkhead_t *phead;
    blknode_t *pnode, *pbuddy;
    size_t i, newpos;

    DPRINTF(("[ Freeing ptr: %p ]\n", ptr));

    /* Search all nodes to find the one that points to ptr */
    pbuddy = NULL;
    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Searching for ptr %p in block: %u\n", ptr, i));
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_chunk) {
            if (pnode->ptr == ptr) {
                DPRINTF(("Found chunk at block: %u\t"
                         "Block has chunks with bytes: %u\n",
                         i, 1 << pnode->logsize));
                goto CHUNK_FOUND;
            }
        }
    }

    /*
     * Chunk isn't in our pool, this is probably bad.
     *
     * It means that either the user has provided an invalid
     * pointer to free or the allocator exhibited buggy
     * behaviour and corrupted itself.
     *
     * Either way, return immediately.
     */
    DPRINTF(("Chunk %p was not found in the pool\n", ptr));
    return;

 CHUNK_FOUND:;
    /* Are we top level ? */
    if (pnode->logsize == mpool->maxlogsize)
        return;

    /* Calculate possible buddy of chunk */
    DPRINTF(("Searching for buddy of %p...\n", pnode->ptr));

    /* `pnode' is a right buddy, so `pbuddy' is a left buddy */
    if (MPOOL_IS_RIGHT(pnode)) {
        pbuddy = (blknode_t *)((char *)pnode - (1 << pnode->logsize));
        if ((void *)pbuddy < (void *)mpool->mem) {
            DPRINTF(("buddy out of pool\n"));
            return;
        }
        if (pbuddy->logsize != pnode->logsize)
            pbuddy = NULL;
    }
    /* `pnode' is a left buddy, so `pbuddy' is a right buddy */
    else {
        pbuddy = (blknode_t *)((char *)pnode + (1 << pnode->logsize));
        if ((void *)pbuddy > (void *)((char *)mpool->mem + (1 << mpool->maxlogsize) - 1)) {
            DPRINTF(("buddy out of pool\n"));
            return;
        }
        if (pbuddy->logsize != pnode->logsize)
            pbuddy = NULL;
    }

    /*
     * If there is no buddy of `pnode' or if there is, but it's unavailable,
     * just free `pnode' and we are done.
     */
    /*if (pbuddy == NULL || (pbuddy != NULL && ((pbuddy->flags & MPOOL_NODE_AVAIL) == 0))) {*/
    if (pbuddy == NULL || (pbuddy != NULL && MPOOL_IS_USED(pbuddy))) {
        DPRINTF(("Not found or found but unavailable\n"));
        DPRINTF(("Freeing chunk %p (marking it as available)\n", pnode->ptr));
        MPOOL_MARK_AVAIL(pnode);
        mpool_printblks(mpool);
        return;
    }
    /*
     * There is a buddy, and it's available for sure. Coalesce.
     */
    else {
        DPRINTF(("Buddy %p exists and it's available. Coalesce.\n",
                 pbuddy->ptr));
#ifdef MPOOL_STATS
        mpool->nmerges++;
#endif
        DPRINTF(("Removing chunk %p from old position %u\n",
                 pnode->ptr, mpool->maxlogsize - pnode->logsize));
        LIST_REMOVE(pnode, next_chunk);
        mpool_printblks(mpool);

        /*
         * `pnode' is left buddy
         */
        if (MPOOL_IS_LEFT(pnode)) {
            if (pnode->flags & MPOOL_NODE_PARENT)
                MPOOL_MARK_RIGHT(pnode);
            else
                MPOOL_MARK_LEFT(pnode);

            if (pbuddy->flags & MPOOL_NODE_PARENT)
                pnode->flags |= MPOOL_NODE_PARENT;
            else
                pnode->flags &= ~MPOOL_NODE_PARENT;

            pnode->logsize++;
            MPOOL_MARK_AVAIL(pnode);

            /* Insert `pnode' to the appropriate position */
            newpos = mpool->maxlogsize - pnode->logsize;
            phead = &mpool->blktable[newpos];
            DPRINTF(("We will keep chunk %p, we will remove pbuddy %p\n",
                     pnode->ptr, pbuddy->ptr));
            DPRINTF(("Inserting chunk %p to new position = %u\n",
                     pnode->ptr, mpool->maxlogsize - pnode->logsize));
            LIST_INSERT_HEAD(phead, pnode, next_chunk);

            /* Remove `pbuddy' from the block lists */
            DPRINTF(("Removing buddy %p\n", pbuddy->ptr));
            LIST_REMOVE(pbuddy, next_chunk);
        }
        /*
         * `pbuddy' is left buddy
         */
        else if (MPOOL_IS_LEFT(pbuddy)) {
            LIST_REMOVE(pbuddy, next_chunk);
            if (pbuddy->flags & MPOOL_NODE_PARENT)
                MPOOL_MARK_RIGHT(pbuddy);
            else
                MPOOL_MARK_LEFT(pbuddy);

            if (pnode->flags & MPOOL_NODE_PARENT)
                pbuddy->flags |= MPOOL_NODE_PARENT;
            else
                pbuddy->flags &= ~MPOOL_NODE_PARENT;

            pbuddy->logsize++;
            MPOOL_MARK_AVAIL(pbuddy);

            /* Insert `pbuddy' to the appropriate position */
            newpos = mpool->maxlogsize - pbuddy->logsize;
            phead = &mpool->blktable[newpos];
            DPRINTF(("We will keep buddy %p, we will remove chunk %p\n",
                     pbuddy->ptr, pnode->ptr));
            DPRINTF(("Inserting buddy %p to new position = %u\n",
                     pbuddy->ptr, mpool->maxlogsize - pbuddy->logsize));
            LIST_INSERT_HEAD(phead, pbuddy, next_chunk);

            pnode = pbuddy;
        }
        /* Error */
        else {
            DPRINTF(("Chunk %p and buddy %p have wrong LR relation",
                     pnode->ptr, pbuddy->ptr));
            return;
        }
        mpool_printblks(mpool);

        goto CHUNK_FOUND;
        /* Never reached */
    }
}

void mpool_destroy(mpool_t *mpool)
{
    free(mpool->blktable);
    free(mpool->mem);
    free(mpool);
}

static void mpool_printblks(const mpool_t *mpool)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    size_t i;

    for (i = 0; i < mpool->nblocks; i++) {
        DPRINTF(("Block (%p): %u\t", mpool->blktable[i], i));
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_chunk) {
            DPRINTF(("ch(ad = %p, by = %u, av = %d, lr = %d, pa = %d)\t",
                     pnode->ptr,
                     (unsigned) (1 << pnode->logsize),
                     MPOOL_IS_AVAIL(pnode) ? 1 : 0,
                     MPOOL_IS_RIGHT(pnode) ? 1 : 0,
                     pnode->flags & MPOOL_NODE_PARENT ? 1 : 0));
        }
        DPRINTF(("\n"));
    }
}

