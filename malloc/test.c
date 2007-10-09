#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */
#include <sys/queue.h>

#include "mpool.h"

#define MAXLOGSIZE 10   /* Maximum logarithm of size */
#define MAXLIFETIME 10  /* Maximum lifetime of reserved blocks */
#define MAXNODES 10     /* Maximum number of simulation nodes */
#define TI 200          /* Every `TI' steps dump statistics */

typedef struct simnode {
    void *ptr;
    unsigned int lifetime;
    LIST_ENTRY(simnode) next_block;
} simnode_t;

LIST_HEAD(simhead, simnode);
typedef struct simhead simhead_t;

/* Function prototypes */
void sim_add_to_list(simhead_t *simhead, simnode_t *simnode);
void sim_print_stats(const mpool_t *mpool, unsigned int t, FILE *fp);

int main(void)
{
    mpool_t *mpool;
    simhead_t simhead;
    simnode_t simnode[MAXNODES];
    unsigned int t, sz, lt;

    /* Initialize random number generator */
    srand(time(NULL));

    /* Initialize memory pool */
    if (mpool_init(&mpool, 20, 3) == MP_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize simlist */
    LIST_INIT(&simhead);

    /* Run simulation */
    for (t = 0; ; t++) {
        printf("t = %u\n", t);
        /* Is it time to dump statistics ? */
        if (t % TI == 0)
            /*sim_print_stats(mpool, t, stdout);*/

        /* Calculate a random size `sz' and a random lifetime `lt' */
        sz = (1 << rand() % (1 + MAXLOGSIZE));
        lt = 1 + rand() % MAXLIFETIME;

        printf("sz = %u\tlt = %u\n", sz, lt);

        /* Allocate a block of size `sz' and make it last `lt' time intervals */
        printf("mpool = %p\n", mpool);
        if ((simnode[t].ptr = mpool_alloc(mpool, sz)) == NULL) {
            fprintf(stderr, "mpool: no available block found\n");
            mpool_destroy(mpool);
            exit(EXIT_FAILURE);
        }
        simnode[t].lifetime = lt;
    }

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}

void sim_add_to_list(simhead_t *simhead, simnode_t *simnode)
{
    simnode_t *pnode;

    LIST_FOREACH(pnode, simhead, next_block) {
        if (pnode->lifetime > simnode->lifetime) {
            LIST_INSERT_BEFORE(pnode, simnode, next_block);
            return;
        }
   }

    LIST_INSERT_HEAD(simhead, simnode, next_block);
}

void sim_print_stats(const mpool_t *mpool, unsigned int t, FILE *fp)
{
    size_t an, un;    /* nodes */
    size_t ab, ub;    /* blocks */
    size_t me, sp;    /* merges, splits */

    mpool_stat_get_nodes(mpool, &an, &un);
    mpool_stat_get_bytes(mpool, &ab, &ub);
    me = mpool_stat_get_merges(mpool);
    sp = mpool_stat_get_splits(mpool);

    fprintf(fp, "%u\t%u\t%u\t%f\t%u\t%u\t%f\t%u\t%u\n",
            t, an, un, 100.0 * an / (an + un), ab, ub, 100.0 * ab / (ab + ub), sp, me);

    /*
    fprintf(fp, "avail nodes = %u\tused nodes = %u\tfree(%%) = %f\n", an, un, 100.0 * an / (an + un));
    fprintf(fp, "avail bytes  = %u\tused bytes = %u\tfree(%%) = %f\n", ab, ub, 100.0 * ab / (ab + ub));
    fprintf(fp, "splits = %u\tmerges = %u\n", sp, me);
    */
}
