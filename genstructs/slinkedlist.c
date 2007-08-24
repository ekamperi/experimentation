/* compile with:
   gcc slinkedlist.c -o slinkedlist -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

SLIST_HEAD(listhead, entry) head;
struct listhead *headp;

struct entry {
    SLIST_ENTRY(entry) entries;
    const char *str;
} *np, *n;

int main(void)
{
    const char *str[] = { "this", "is", "a", "single", "linked", "list" };
    unsigned int i;

    /* Initialize list */
    SLIST_INIT(&head);
    headp = &head;

    /* Populate list with str[] items */
    for (i = 0; i < sizeof str / sizeof *str; i++) {
        if ((n = malloc(sizeof(struct entry))) == NULL) {
            perror("malloc");
            goto CLEANUP_AND_EXIT;
        }
        n->str = str[i];

        if (i == 0)
            SLIST_INSERT_HEAD(&head, n, entries);
        else
            SLIST_INSERT_AFTER(np, n, entries);
        np = n;
    }

    /* Traverse list */
    SLIST_FOREACH(np, &head, entries)
        printf("%s\n", np->str);

 CLEANUP_AND_EXIT:;
    /* Delete all elements */
    while (SLIST_FIRST(&head) != NULL)
        SLIST_REMOVE_HEAD(&head, entries);

    return EXIT_SUCCESS;
}
