#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpool.h"

mpret_t mpool_init(mpool_t **mpool, size_t logsize, size_t minsize)
{
    blknode_t *pblknode;
    unsigned int i;

    /* Allocate memory for memory pool data structure */
    if ((*mpool = malloc(sizeof **mpool)) == NULL)
        return MP_ENOMEM;

    /* Allocate the actual memory of the pool */
    printf("Allocating %u bytes for pool\n", 2 << logsize);
    if (((*mpool)->mem = malloc(2 << logsize)) == NULL) {
        free(*mpool);
        return MP_ENOMEM;
    }

    /* Allocate memory for block lists */
    if (((*mpool)->blktable = malloc(logsize * sizeof *(*mpool)->blktable)) == NULL) {
        free((*mpool)->blktable);
        free(*mpool);
        return MP_ENOMEM;
    }

    /* Initialize block lists */
    for (i = 0; i < logsize; i++)
        LIST_INIT(&(*mpool)->blktable[i]);

    (*mpool)->logsize = logsize;
    (*mpool)->minsize = minsize;

    /* Initially, before any storage has been requested,
       we have a single available block of length 2 << logsize
       in blktable[0].
     */
    if ((pblknode = malloc(sizeof *pblknode)) == NULL) {
        free((*mpool)->blktable);
        free(*mpool);
        return MP_ENOMEM;
    }
    pblknode->ptr = (*mpool)->mem;
    pblknode->avail = 1;
    pblknode->logsize = logsize;

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

    /* Find the most suitable 2^j bytes block for the requested size of k bytes.
       The block must satisfy the condition: 2^j >= size and must also be available.
     */
    pavailnode = NULL;
    for (i = 0; i < mpool->logsize; i++) {
        printf("Searcing block: %u\n", i);
        phead = &mpool->blktable[i];
        if ((pnode = LIST_FIRST(phead)) != NULL) {
            if ((unsigned)(2 << pnode->logsize) >= size) {
                LIST_FOREACH(pnode, &mpool->blktable[i], next_block) {
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
    printf("Found block of bytes %u\n", 2 << pavailnode->logsize);

    /* Split required ? */
AGAIN:;
    printf("size = %u\tp = %u\tp-1=%u\n",
           size,
           2 << pavailnode->logsize,
           2 << (pavailnode->logsize - 1));

    if ((size < mpool->minsize)
        || (size == (unsigned)(2 << pavailnode->logsize))
        || (size > (unsigned)(2 << (pavailnode->logsize - 1)))) {
        printf("No split required\n");
        pavailnode->avail = 0;    /* Mars as no longer available */
        mpool_printblks(mpool);
        return pavailnode->ptr;
    }

    printf("Splitting...\n");

    /* Removing old block */
    printf("Removing old chunk from list\n");
    LIST_REMOVE(pavailnode, next_block);
    mpool_printblks(mpool);
    pavailnode->logsize--;

    printf("New size is now: %u bytes\n", 2 << pavailnode->logsize);
    printf("Moving old chunk to new position\n");
    newpos = mpool->logsize - pavailnode->logsize;
    LIST_INSERT_HEAD(&mpool->blktable[newpos], pavailnode, next_block);
    mpool_printblks(mpool);

    /* Split */
    printf("Will add new item with bytes: %u\n", 2 << pavailnode->logsize);
    if ((pnewnode = malloc(sizeof *pnewnode)) == NULL)
        return NULL;    /* ? */
    pnewnode->ptr = pavailnode->ptr + (2 << pavailnode->logsize);
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
    for (i = 0; i < mpool->logsize; i++) {
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

    for (i = 0; i < mpool->logsize; i++) {
        printf("Block: %u\t", i);

        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_block) {
            printf("chunk(addr = %p, bytes = %u, av = %d)\t",
                   pnode->ptr,
                   (unsigned) (2 << pnode->logsize),
                   pnode->avail);
        }

        printf("\n");
    }
}

int main(void)
{
    mpool_t *mpool;
    /*char *p, *p2, *p3;*/
    char *p[100];
    unsigned int i;

    if (mpool_init(&mpool, 15, 2) == MP_ENOMEM) {
        fprintf(stderr, "Not enought memory\n");
        exit(EXIT_FAILURE);
    }

    /*
    p = mpool_alloc(mpool, 70);
    p2 = mpool_alloc(mpool, 35);
    p3 = mpool_alloc(mpool, 80);

    strcpy(p, "Hello flyfly");
    strcpy(p2, "H");
    strcpy(p3, "Hello all");

    printf("Address: %p\tContent: %s\n", p, p);
    printf("Address: %p\tContent: %s\n", p2, p2);
    printf("Address: %p\tContent: %s\n", p3, p3);
    */

    for (i = 0; i < 100; i++)
        if ((p[i] = mpool_alloc(mpool, 2 << ((rand() % 10)))) == NULL)
            break;

    mpool_destroy(mpool);
    return EXIT_SUCCESS;
}
