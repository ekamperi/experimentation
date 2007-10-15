#include <stdio.h>
#include <stdlib.h>

#include "mpool.h"

int main(void)
{
    mpool_t *mpool;
    char *buffer;

    /* Initialize memory pool */
    if (mpool_init(&mpool, 10, 5) == MPOOL_ENOMEM) {
        fprintf(stderr, "Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    /* Allocate 32 bytes for buffer */
    if ((buffer = mpool_alloc(mpool, 5)) == NULL) {
        fprintf(stderr, "No available block in pool\n");
        exit(EXIT_FAILURE);
    }

    /* Read input from standard input stream */
    fgets(buffer, 1<<5, stdin);

    /* Print buffer's contents */
    printf("Buffer: %s", buffer);

    /* Free buffer --
     * could be omitted, since we call mpool_destroy() afterwards
    */
    mpool_free(mpool, buffer);

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
