/*
 * Compile with:
 * gcc scanfprob.c -o scanfprob -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int a, rv;

    /*
     * When scanf is attempting to convert numbers, any non-numeric characters
     * it encounters terminate the conversion and are left in the input stream.
     * Therefore, unexpected non-numeric input ``jams'' scanf again and again.
     * E.g.
     * 
     * do {
     *     printf("Input: ");
     *     scanf("%d", &a);
     * } while (a != -1);
     * 
     * See also: http://c-faq.com/stdio/scanfjam.html
     */

    /* The correct way to do it */
    do {
        printf("Input: ");
        rv = scanf("%d", &a);
        if (rv == 0)
            scanf("%[^\n]");
    } while (rv == 0 || a != -1);

    return EXIT_SUCCESS;
}
