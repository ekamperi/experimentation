#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTABLE_SIZE 100

typedef struct hnode {
    char *str;
    struct hnode *next;
} hnode_t;

/* Function prototypes */
hnode_t *htable_alloc(unsigned int size);
void htable_free(hnode_t *htable, unsigned int size);
void htable_insert(hnode_t *htable, char *str, unsigned int pos);
unsigned int htable_search(hnode_t *htable, char *str);
unsigned int htable_mkhash(char *str);

int main(void)
{
    hnode_t *myhtable;

    myhtable = htable_alloc(HTABLE_SIZE);
    htable_insert(myhtable, "stathis", htable_mkhash("stathis"));

    printf("%d\n", htable_search(myhtable, "stathis"));
    htable_free(myhtable, HTABLE_SIZE);

    return EXIT_SUCCESS;
}

hnode_t *htable_alloc(unsigned int size)
{
    hnode_t *pnode;
    unsigned int i;

    if ((pnode = malloc(size * sizeof *pnode)) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
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
        pnode = &htable[i];
        while (pnode->next != NULL) {
            tmp = pnode->next->next;
            free(pnode->next->str);
            free(pnode->next);
            pnode = tmp;
        }
    }

    free(htable);
}

void htable_insert(hnode_t *htable, char *str, unsigned int pos)
{
    hnode_t *pnode;
    unsigned int i;

    if (htable[pos].str == NULL) {
        if ((htable[pos].str = malloc(sizeof(strlen(str)))) == NULL)
            goto OUT;
        strcpy(htable[pos].str, str);
    }
    else {
        /* Collision resolution */
        for (pnode = htable[pos].next; pnode != NULL; pnode = pnode->next) {
            if (pnode->next == NULL) {
                pnode->next = htable_alloc(1);
                if ((pnode->next->str = malloc(sizeof(strlen(str)))) == NULL)
                    goto OUT;
                strcpy(pnode->next->str, str);
            }
        }
    }
    OUT:;
}

unsigned int htable_search(hnode_t *htable, char *str)
{
    hnode_t *pnode;
    unsigned int pos;

    pos = htable_mkhash(str);

    if (strcmp(htable[pos].str, str) == 0)
        return pos;
    else {
        /* Search across the chain */
        for (pnode = htable[pos].next; pnode != NULL; pnode = pnode->next)
            if (strcmp(pnode->str, str))
                return pos;
    }
    
    return -1;
}

unsigned int htable_mkhash(char *str)
{
    /* DJB hashing */
    unsigned int i, hash = 5381;

    for (i = 0; i < strlen(str); i++)
        hash = ((hash << 5) + hash) + str[i];

    return (hash & 0x7FFFFFFF) % HTABLE_SIZE;
}
