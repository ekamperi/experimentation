#include <stdio.h>

int main(void)
{
    int a, rv;

    do {
        printf("Input: ");
        rv = scanf("%d", &a);
        if (rv == 0)
            scanf("%[^\n]");
    } while (rv == 0 || a != -1);

    return 0;
}
