#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */
#include <sys/queue.h>

#include "mpool.h"

#define MAXLOGSIZE 18   /* Maximum logarithm of size */
#define MAXLIFETIME 100  /* Maximum lifetime of reserved blocks */
#define MAXNODES 20000     /* Maximum number of simulation nodes */
#define TI 10          /* Every `TI' steps dump statistics */

typedef struct simnode {
    void *ptr;
    unsigned int lifetime;
    LIST_ENTRY(simnode) next_block;
} simnode_t;

LIST_HEAD(simhead, simnode);
typedef struct simhead simhead_t;

/* Function prototypes */
void sim_add_to_list(simhead_t *simhead, simnode_t *simnode);
void sim_free_from_list(mpool_t *mpool, simhead_t *simhead, unsigned int t);
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
    if (mpool_init(&mpool, 25, 2) == MP_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize simlist */
    LIST_INIT(&simhead);

    /* Run simulation */
    for (t = 0; t < MAXNODES; t++) {
        /* Is it time to dump statistics ? */
        if (t % TI == 0)
            sim_print_stats(mpool, t, stdout);

        /* */
        sim_free_from_list(mpool, &simhead, t);

        /* Calculate a random size `sz' and a random lifetime `lt' */
        sz = (1 << rand() % (1 + MAXLOGSIZE));
        lt = 1 + rand() % MAXLIFETIME;
        /*printf("t = %u\tsz = %u\tlt = %u\n", t, sz, lt);*/

        /* Allocate a block of size `sz' and make it last `lt' time intervals */
        if ((simnode[t].ptr = mpool_alloc(mpool, sz)) == NULL) {
            fprintf(stderr, "mpool: no available block\n");
            mpool_destroy(mpool);
            exit(EXIT_FAILURE);
        }
        simnode[t].lifetime = t + lt;

        /* Add block to list, in the proper position */
        sim_add_to_list(&simhead, &simnode[t]);
    }

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}

void sim_add_to_list(simhead_t *simhead, simnode_t *simnode)
{
    simnode_t *pnode;

    /*
    LIST_FOREACH(pnode, simhead, next_block) {
        printf("%u -> ", pnode->lifetime);
    }
    printf("\n");
    */

    LIST_FOREACH(pnode, simhead, next_block) {
        if (simnode->lifetime < pnode->lifetime) {
            LIST_INSERT_BEFORE(pnode, simnode, next_block);
            return;
        }
        else if (LIST_NEXT(pnode, next_block) == NULL) {
            LIST_INSERT_AFTER(pnode, simnode, next_block);
            return;
        }
   }

    /* 1st element goes here */
    LIST_INSERT_HEAD(simhead, simnode, next_block);
}

void sim_free_from_list(mpool_t *mpool, simhead_t *simhead, unsigned int t)
{
    simnode_t *pnode;

    LIST_FOREACH(pnode, simhead, next_block) {
        if (t == pnode->lifetime) {
            /*printf("freeing %u\n", t);*/
            mpool_free(mpool, pnode->ptr);
            LIST_REMOVE(pnode, next_block);
        }
        else
            return;
    }
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

    fprintf(fp, "%u\t%u\t%u\t%f\t%u\t%u\t%f\t%u\t%u\t%f\n",
            t, an, un, 100.0 * an / (an + un), ab, ub, 100.0 * ab / (ab + ub), sp, me, 10.0*sp/(1+me));

    /*
    fprintf(fp, "avail nodes = %u\tused nodes = %u\tfree(%%) = %f\n", an, un, 100.0 * an / (an + un));
    fprintf(fp, "avail bytes  = %u\tused bytes = %u\tfree(%%) = %f\n", ab, ub, 100.0 * ab / (ab + ub));
    fprintf(fp, "splits = %u\tmerges = %u\n", sp, me);
    */
}
