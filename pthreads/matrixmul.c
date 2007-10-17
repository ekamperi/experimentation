/* compile with:
   gcc matrixmul.c -o matrixmul -lpthread -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct matrix {
   unsigned int rows;
   unsigned int cols;
   int **data;
} *mat1, *mat2, *mat3;

struct matrix_index {
   unsigned int row;
   unsigned int col;
};

typedef enum {
    MM_OK,
    MM_ENOMEM,
    MM_EIO
} mmret_t;

/* function prototypes */
mmret_t matrix_alloc(struct matrix **mat, unsigned int rows, unsigned int cols);
void matrix_free(struct matrix **mat);
mmret_t matrix_read(const char *path, struct matrix **mat);
void matrix_print(const struct matrix *mat);
void *mulvect(void *arg);
void diep(const char *s);

int main(int argc, char *argv[])
{
    pthread_t *tid;
    struct matrix_index *v;
    unsigned int i, j, k, mdepth = 0, numthreads;

    /* check argument count */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s matfile1 matfile2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* read matrix data from files */
    if (matrix_read(argv[1], &mat1) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    if (matrix_read(argv[2], &mat2) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    /* is the multiplication feasible by definition? */
    if (mat1->cols != mat2->rows) {
        fprintf(stderr, "Matrices' dimensions size must satisfy (NxM)(MxK)=(NxK)\n");
        goto CLEANUP_AND_EXIT;
    }

    /* allocate memory for the result */
    if (matrix_alloc(&mat3, mat1->rows, mat2->cols) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    /* how many threads do we need ? */
    numthreads = mat1->rows * mat2->cols;

    /* v[k] holds the (i, j) pair in the k-th computation */
    if ((v = malloc(numthreads * sizeof *v)) == NULL) {
        perror("malloc");
        goto CLEANUP_AND_EXIT;
    }
    mdepth++;

    /* allocate memory for the threads' ids */
    if ((tid = malloc(numthreads * sizeof *tid)) == NULL) {
        perror("malloc");
        goto CLEANUP_AND_EXIT;
    }
    mdepth++;

    /* create the threads */
    for (i = 0; i < mat1->rows; i++) {
        for (j = 0; j < mat2->cols; j++) {
            k = i*mat1->rows + j;
            v[k].row = i;
            v[k].col = j;
            if (pthread_create(&tid[k], NULL, mulvect, (void *)&v[k])) {
                perror("pthread_create");
                goto CLEANUP_AND_EXIT;
            }
        }
    }

    /* make sure all threads are done  */
    for (i = 0; i < numthreads; i++)
        if (pthread_join(tid[i], NULL)) {
            perror("pthread_join");
            goto CLEANUP_AND_EXIT;
        }

    /* print the result */
    matrix_print(mat3);

 CLEANUP_AND_EXIT:;
    switch(mdepth) {
    case 5: free(tid);
    case 4: free(v);
    case 3: matrix_free(&mat3);
    case 2: matrix_free(&mat2);
    case 1: matrix_free(&mat1);
    case 0:  ;    /* free nothing */
    }

    return EXIT_SUCCESS;
}

mmret_t matrix_alloc(struct matrix **mat, unsigned int rows, unsigned int cols)
{
    unsigned int i, j, mdepth = 0;

    *mat = malloc(sizeof **mat);
    if (*mat == NULL)
        goto CLEANUP_AND_RETURN;
    mdepth++;

    (*mat)->rows = rows;
    (*mat)->cols = cols;

    (*mat)->data = malloc(rows * sizeof(int *));
    if ((*mat)->data == NULL)
        goto CLEANUP_AND_RETURN;
    mdepth++;

    for (i = 0; i < rows; i++) {
        (*mat)->data[i] = malloc(cols * sizeof(int));
        if ((*mat)->data[i] == NULL) {
            if (i != 0)
                mdepth++;
            goto CLEANUP_AND_RETURN;
        }
    }
    return MM_OK;

 CLEANUP_AND_RETURN:;
    perror("malloc");
    switch(mdepth) {
    case 3: for (j = 0; j < i; j++) free((*mat)->data[j]);
    case 2: free((*mat)->data);
    case 1: free(*mat);
    case 0:  ;    /* free nothing */
    }

    return MM_ENOMEM;
}

void matrix_free(struct matrix **mat)
{
    unsigned int i;

    for (i = 0; i < (*mat)->rows; i++)
        free((*mat)->data[i]);

    free((*mat)->data);
    free(*mat);
}

mmret_t matrix_read(const char *path, struct matrix **mat)
{
    FILE *fp;
    unsigned int i, j, rows, cols;

    /* open file */
    if ((fp = fopen(path, "r")) == NULL) {
        fprintf(stderr, "Error opening file: %s\n", path);
        return MM_EIO;
    }

    /* read matrix dimensions */
    fscanf(fp, "%u%u", &rows, &cols);

    /* allocate memory for matrix */
    if (matrix_alloc(mat, rows, cols) == MM_ENOMEM) {
        fclose(fp);
        return MM_ENOMEM;
    }

    /* read matrix elements */
    for (i = 0; i < (*mat)->rows; i++) {
        for (j = 0; j < (*mat)->cols; j++) {
            fscanf(fp, "%d", &(*mat)->data[i][j]);
        }
   }

    /* close file */
    fclose(fp);

    return MM_OK;
}

void matrix_print(const struct matrix *mat)
{
    unsigned int i, j;

    for (i = 0; i < mat->rows; i++) {
        for (j = 0; j < mat->cols; j++) {
            printf("%d ", mat->data[i][j]);
        }
        printf("\n");
    }
}


void *mulvect(void *arg)
{
   unsigned int i, row, col;

   row = *((int *)arg + 0);
   col = *((int *)arg + 1);

   mat3->data[row][col] = 0;
   for (i = 0; i < mat1->cols; i++)
      mat3->data[row][col] += mat1->data[row][i] * mat2->data[i][col];

   pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
