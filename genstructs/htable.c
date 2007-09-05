#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTABLE_SIZE 10

typedef struct hnode {
    char *str;
    struct hnode *next;
} hnode_t;

/* Function prototypes */
hnode_t *htable_alloc(unsigned int size);
void htable_free(hnode_t *htable, unsigned int size);
void htable_insert(hnode_t *htable, char *str, unsigned int pos);
unsigned int htable_search(const hnode_t *htable, unsigned int size, const char *str);
void htable_print(const hnode_t *htable, unsigned int size);
unsigned int htable_mkhash(const char *str, unsigned int size);

int main(void)
{
    hnode_t *myhtable;

    myhtable = htable_alloc(HTABLE_SIZE);

    htable_insert(myhtable, "stathis", htable_mkhash("stathis", HTABLE_SIZE));
    htable_insert(myhtable, "kostas", htable_mkhash("kostas", HTABLE_SIZE));
    htable_insert(myhtable, "petros", htable_mkhash("petros", HTABLE_SIZE));
    htable_insert(myhtable, "maria", htable_mkhash("maria", HTABLE_SIZE));

    printf("%d\n", htable_search(myhtable, HTABLE_SIZE, "stathis"));
    printf("%d\n", htable_search(myhtable, HTABLE_SIZE, "kostas"));
    printf("%d\n", htable_search(myhtable, HTABLE_SIZE, "petros"));
    printf("%d\n", htable_search(myhtable, HTABLE_SIZE, "maria"));

    htable_print(myhtable, HTABLE_SIZE);

    htable_free(myhtable, HTABLE_SIZE);

    return EXIT_SUCCESS;
}

hnode_t *htable_alloc(unsigned int size)
{
    hnode_t *pnode;
    unsigned int i;

    /* Allocate memory for `size' hnode_t structures */
    if ((pnode = malloc(size * sizeof *pnode)) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Initialize hnode_t fields */
    for (i = 0; i < size; i++) {
        pnode[i].str = NULL;
        pnode[i].next = NULL;
    }

    return pnode;
}

void htable_free(hnode_t *htable, unsigned int size)
{
    hnode_t *pnode, *tmp;
    unsigned int i;

    for (i = 0; i < size; i++) {
        pnode = htable[i].next;
        /* Does this pnode have a chain ? If yes, free it */
        while (pnode != NULL) {
            tmp = pnode->next;
            free(pnode->str);
            free(pnode);
            pnode = tmp;
        }
        if (htable[i].str != NULL)
            free(htable[i].str);
    }

    free(htable);
}

void htable_insert(hnode_t *htable, char *str, unsigned int pos)
{
    hnode_t *pnode;

    /* Is `pos' available in the hash table ? */
    if (htable[pos].str == NULL) {
        if ((htable[pos].str = malloc(sizeof(strlen(str)))) == NULL)
            goto OUT;
        strcpy(htable[pos].str, str);
    }
    else {
        /* Collision resolution */
        for (pnode = &htable[pos]; pnode->next != NULL; pnode = pnode->next)
            ;
        pnode->next = htable_alloc(1);
        if ((pnode->next->str = malloc(sizeof(strlen(str)))) == NULL)
            goto OUT;
        strcpy(pnode->next->str, str);
    }
 OUT:;
}

unsigned int htable_search(const hnode_t *htable, unsigned int size, const char *str)
{
    hnode_t *pnode;
    unsigned int pos;

    /* Get the hash */
    pos = htable_mkhash(str, size);

    /* Is the `str' in the hash table ? */
    if (strcmp(htable[pos].str, str) == 0)
        return pos;
    else {
        /* Search across the chain */
        for (pnode = htable[pos].next; pnode != NULL; pnode = pnode->next)
            if (strcmp(pnode->str, str) == 0)
                return pos;
    }

    return -1;    /* Not found */
}

void htable_print(const hnode_t *htable, unsigned int size)
{
    const hnode_t *pnode;
    unsigned int i;

    for (i = 0; i < size; i++) {
        pnode = &htable[i];
        if (pnode->str != NULL) {
            printf("%s ", pnode->str);
            /* Does this pnode have a chain ? If yes, print it */
            while (pnode->next != NULL) {
                printf("%s ", pnode->next->str);
                pnode = pnode->next;
            }
            printf("\n");
        }
    }
}

unsigned int htable_mkhash(const char *str, unsigned int size)
{
    /* DJB hashing */
    unsigned int i, hash = 5381;

    for (i = 0; i < strlen(str); i++)
        hash = ((hash << 5) + hash) + str[i];

    return (hash & 0x7FFFFFFF) % size;
}
