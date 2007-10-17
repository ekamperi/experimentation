#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */

#include "mpool.h"

int main(int argc, char *argv[])
{
    mpool_t *mpool;
    int i, j, **array;

    /* Check argument count */
    if (argc != 3) {
        fprintf(stderr, "usage: %s nrows ncols\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Initialize memory pool with 1024 bytes */
    if (mpool_init(&mpool, 10, 5) == MPOOL_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize random number generator */
    srand(time(NULL));

    /* Allocate memory for rows */
    if ((array = mpool_alloc(mpool, atoi(argv[1]) * sizeof *array)) == NULL) {
        fprintf(stderr, "No available block in pool\n");
        mpool_destroy(mpool);
        exit(EXIT_FAILURE);
    }

    /* Allocate memory for columns */
    for (i = 0; i < atoi(argv[1]); i++) {
        if ((array[i] = mpool_alloc(mpool, atoi(argv[2]) * sizeof **array)) == NULL) {
            fprintf(stderr, "No available block in pool\n");
            mpool_destroy(mpool);
            exit(EXIT_FAILURE);
        }
    }

    /* Fill in matrix with random values and print it */
    for (i = 0; i < atoi(argv[1]); i++) {
        for (j = 0; j < atoi(argv[2]); j++) {
            array[i][j] = rand() % 100);
            printf("%2d\t", array[i][j]);
        }
        printf("\n");
    }


    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
