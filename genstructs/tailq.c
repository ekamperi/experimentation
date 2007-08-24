/* compile with:
   gcc tailq.c -o tailq -Wall -W -Wextra -ansi -pedantic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

TAILQ_HEAD(tailhead, entry) head;

struct entry {
    TAILQ_ENTRY(entry) entries;
    const char *str;
} *np, *n;

int main(void)
{
    const char *str[] = { "this", "is", "a", "double", "linked", "tailq" };
    unsigned int i;

    /* Initialize list */
    TAILQ_INIT(&head);

    /* Populate list with str[] items */
    for (i = 0; i < sizeof str / sizeof *str; i++) {
        if ((n = malloc(sizeof(struct entry))) == NULL) {
            perror("malloc");
           goto CLEANUP_AND_EXIT;
        }
        n->str = str[i];

        if (i == 0)
            TAILQ_INSERT_HEAD(&head, n, entries);
        else
            TAILQ_INSERT_AFTER(&head, np, n, entries);
        np = n;
    }

    /* Traverse list forward */
    TAILQ_FOREACH(np, &head, entries)
        printf("%s %s", np->str,
               TAILQ_NEXT(np, entries) != NULL ?
               "-> " : "\n");

    TAILQ_FOREACH_REVERSE(np, &head, tailhead, entries)
        printf("%s %s", np->str,
               TAILQ_PREV(np, tailhead, entries) != NULL ?
               "-> " : "\n");

 CLEANUP_AND_EXIT:;
    /* Delete all elements */
    while (TAILQ_FIRST(&head) != NULL)
        TAILQ_REMOVE(&head, TAILQ_FIRST(&head), entries);

    return EXIT_SUCCESS;
}
