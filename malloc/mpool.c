#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    /* Allocate the actual memory of the pool */
    printf("Allocating %u bytes for pool\n", 1 << maxlogsize);
    printf("maxlogsize = %u\tminlogsize = %u\tnblocks = %u\n",
           (*mpool)->maxlogsize,
           (*mpool)->minlogsize,
           (*mpool)->nblocks);
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
    pblknode->avail = 1;
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

    printf("\n\n=======================================================\n\n");
    printf("Searching for block of bytes: %u\n", size);

    /*
     * Find the most suitable 2^j bytes block for the requested size of bytes.
     * The condition  2^j >= size must be satisfied for the smallest possible value
     * of j and the block must be marked as available ofcourse.
    */
    pavailnode = NULL;
    for (i = 0; i < mpool->nblocks; i++) {
        printf("Searcing block: %u\n", i);
        phead = &mpool->blktable[i];
        if ((pnode = LIST_FIRST(phead)) != NULL) {
            if ((unsigned)(1 << pnode->logsize) >= size) {
                LIST_FOREACH(pnode, phead, next_block) {
                    if (pnode->avail != 0) {
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
        printf("No available block found\n");
        return NULL;
    }
    printf("Found block of bytes %u\n", 1 << pavailnode->logsize);

    /* Is a split required ? */
AGAIN:;
    printf("size = %u\tp = %u\tp-1=%u\n",
           size,
           1 << pavailnode->logsize,
           1 << (pavailnode->logsize - 1));

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
        printf("No split required\n");
        pavailnode->avail = 0;    /* Mark as no longer available */
        mpool_printblks(mpool);
        return pavailnode->ptr;
    }

    printf("Splitting...\n");

    /* Remove old block */
    printf("Removing old chunk from list\n");
    LIST_REMOVE(pavailnode, next_block);
    mpool_printblks(mpool);
    pavailnode->logsize--;

    printf("New size is now: %u bytes\n", 1 << pavailnode->logsize);
    printf("Moving old chunk to new position\n");
    newpos = mpool->nblocks - pavailnode->logsize - 1;
    LIST_INSERT_HEAD(&mpool->blktable[newpos], pavailnode, next_block);
    mpool_printblks(mpool);

    /* Split */
    printf("Will add new item with bytes: %u\n", 1 << pavailnode->logsize);
    if ((pnewnode = malloc(sizeof *pnewnode)) == NULL)
        return NULL;    /* ? */
    pnewnode->ptr = (char *)pavailnode->ptr + (1 << pavailnode->logsize);
    pnewnode->avail = 1;
    pnewnode->logsize = pavailnode->logsize;
    LIST_INSERT_HEAD(&mpool->blktable[newpos], pnewnode, next_block);
    mpool_printblks(mpool);

    goto AGAIN;
    /* Never reached */
}

void mpool_free(void *ptr, size_t size)
{
    size = 0;
    ptr = NULL;
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
        printf("Block: %u\t", i);

        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            printf("chunk(addr = %p, bytes = %u, av = %d)\t",
                   pnode->ptr,
                   (unsigned) (1 << pnode->logsize),
                   pnode->avail);
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
            if (pnode->avail == 1)
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

    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            if (pnode->avail == 1)
                *avail += 1 << pnode->logsize;
            else
                *used += 1 << pnode->logsize;
        }
    }
}

int main(void)
{
    mpool_t *mpool;
    char *p[1000];
    size_t an, un, ab, ub;
    unsigned int i;

    if (mpool_init(&mpool, 15, 1) == MP_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 10; i++)
        if ((p[i] = mpool_alloc(mpool, 1 << ((rand() % 10)))) == NULL)
            break;

    mpool_stat_get_nodes(mpool, &an, &un);
    mpool_stat_get_bytes(mpool, &ab, &ub);
    printf("avail nodes = %u\tused nodes = %u\tfree(%%) = %f\n", an, un, (float)an / (an + un));
    printf("avail bytes  = %u\tused bytes = %u\tfree(%%) = %f\n", ab, ub, (float)ab / (ab + ub));

    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
