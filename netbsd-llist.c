#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <assert.h>

LIST_HEAD(listhead, entry) head;
struct listhead *headp;

struct entry {
    LIST_ENTRY(entry) entries;
    const char *str;
} *np, *n;

void diep(const char *s);

int main(void)
{
    const char *str[] = { "this", "is", "a", "linked", "list" };
    unsigned int i;

    /* Initialize list */
    LIST_INIT(&head);
    headp = &head;

    /* Populate list with str[] items */
    for (i = 0; i < sizeof str / sizeof *str; i++) {
        n = malloc(sizeof(struct entry));
        if (n == NULL)
            diep("malloc");
        n->str = str[i];

        if (i == 0)
            LIST_INSERT_HEAD(&head, n, entries);
        else
            LIST_INSERT_AFTER(np, n, entries);
        np = n;
    }

    /* Traverse list */
    LIST_FOREACH(np, &head, entries)
        printf("%s\n", np->str);

    /* Delete all elements */
    while (LIST_FIRST(&head) != NULL)
        LIST_REMOVE(LIST_FIRST(&head), entries);

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
