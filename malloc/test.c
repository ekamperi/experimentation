#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */

#include "mpool.h"

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

