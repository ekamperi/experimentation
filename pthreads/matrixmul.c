#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct matrix {
   unsigned int rows;
   unsigned int cols;
   int **data;
};

struct matrix_index {
   unsigned int row;
   unsigned int col;
};

struct matrix *mat1 = NULL;
struct matrix *mat2 = NULL;
struct matrix *mat3 = NULL;

typedef enum {
    mm_error_none,
    mm_error_no_memory,
    mm_error_io
} mm_error;

/* function prototypes */
mm_error matrix_alloc(struct matrix **mat, unsigned int rows, unsigned int cols);
void matrix_free(struct matrix **mat);
mm_error matrix_read(const char *path, struct matrix **mat);
void matrix_print(const struct matrix *mat);
void *mulvect(void *arg);
void diep(const char *s);

int main(int argc, char *argv[])
{
    pthread_t *tid;
    struct matrix_index *v;
    unsigned int i, j, k, numthreads;

    /* check argument count */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s matfile1 matfile2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* read matrix data from files */
    matrix_read(argv[1], &mat1);
    matrix_read(argv[2], &mat2);

    /* is the multiplication feasible by definition? */
    if (mat1->cols != mat2->rows) {
        fprintf(stderr, "Matrices' dimensions size must satisfy (NxM)(MxK)=(NxK)\n");
        exit(EXIT_FAILURE);
    }

    /* allocate memory for the result */
    matrix_alloc(&mat3, mat1->rows, mat2->cols);

    /* how many threads do we need ?*/
    numthreads = mat1->rows * mat2->cols;

    /* v[k] holds the (i, j) pair in the k-th computation */
    if ((v = malloc(numthreads * sizeof(struct matrix_index))) == NULL)
        diep("malloc");

    /* allocate memory for the threads' ids */
    tid = malloc(numthreads * sizeof(pthread_t));
    if (tid == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }

    /* create the threads */
    for (i = 0; i < mat1->rows; i++) {
        for (j = 0; j < mat2->cols; j++) {
            k = i*mat1->rows + j;
            v[k].row = i;
            v[k].col = j;
            if (pthread_create(&tid[k], NULL, mulvect, (void *)&v[k])) {
                fprintf(stderr, "pthtrad_create() error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    /* make sure all threads are done  */
    for (i = 0; i < numthreads; i++)
        if (pthread_join(tid[i], NULL)) {
            fprintf(stderr, "pthread_join() error\n");
            exit(EXIT_FAILURE);
        }

    /* print the result */
    printmat(mat3);

    /* free matrices */
    matrix_free(&mat1);
    matrix_free(&mat2);
    matrix_free(&mat3);

    return EXIT_SUCCESS;
}

mm_error matrix_alloc(struct matrix **mat, unsigned int rows, unsigned int cols)
{
    unsigned int i, j, mdepth = 0;

    *mat = malloc(sizeof **mat);
    if (*mat == NULL) {
        perror("malloc");
        goto CLEANUP_AND_RETURN;
    }
    mdepth++;

    (*mat)->rows = rows;
    (*mat)->cols = cols;

    (*mat)->data = malloc(rows * sizeof(int *));
    if ((*mat)->data == NULL) {
        perror("malloc");
        goto CLEANUP_AND_RETURN;
    }
    mdepth++;

    for (i = 0; i < rows; i++) {
        (*mat)->data[i] = malloc(cols * sizeof(int));
        if ((*mat)->data[i] == NULL) {
            perror("malloc");
            if (i != 0)
                mdepth++;
            goto CLEANUP_AND_RETURN;
        }
    }
    return mm_error_none;

 CLEANUP_AND_RETURN:;
    switch(mdepth) {
    case 3:
        for (j = 0; j < i; j++)
            free((*mat)->data[j]);
    case 2:
        free((*mat)->data);
    case 1:
        free(*mat);
    case 0:
        ;    /* free nothing */
    }

    return mm_error_no_memory;
}

void matrix_free(struct matrix **mat)
{
    unsigned int i;

    for (i = 0; i < (*mat)->rows; i++) {
        if ((*mat)->data[i] != NULL)
            free((*mat)->data[i]);
    }

    if (*mat != NULL)
        free(*mat);

    *mat = NULL;
}

mm_error matrix_read(const char *path, struct matrix **mat)
{
    FILE *fp;
    unsigned int i, j, rows, cols;

    /* open file */
    if ((fp = fopen(path, "r")) == NULL) {
        fprintf(stderr, "Error opening file: %s\n", path);
        return mm_error_io;
    }

    /* read matrix dimensions */
    fscanf(fp, "%u%u", &rows, &cols);

    /* allocate memory for matrix */
    matrix_alloc(mat, rows, cols);

    /* read matrix elements */
    for (i = 0; i < (*mat)->rows; i++) {
        for (j = 0; j < (*mat)->cols; j++) {
            fscanf(fp, "%d", &(*mat)->data[i][j]);
        }
   }

    /* close file */
    fclose(fp);

    return mm_error_none;
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


void *mulvect(void *arg) {
   unsigned int i, row, col;

   row = *((int *)arg + 0);
   col = *((int *)arg + 1);

   mat3->data[row][col] = 0;
   for (i=0; i<mat1->cols; i++)
      mat3->data[row][col] += mat1->data[row][i] * mat2->data[i][col];

   pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
