#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned int i, j, maxf, freq[26] = { 0 };    /* A - Z */
    int c;

    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "usage: %s path\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open file for reading */
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    /* count frequencies */
    while ((c = fgetc(fp)) != EOF) {
        c = toupper(c);
        if (c >= 'A' && c <= 'Z')
            freq[c - 'A']++;
    }

    /* get max frequency */
    maxf = freq[0];
    for (i = 1; i < sizeof freq / sizeof *freq; i++)
        if (freq[i] > maxf)
            maxf = i;

    /* print frequencies */
    i = maxf;
    for (i = freq[maxf]; i > 0; i--) {
        printf("%3u| ", i);
        for (j = 0; j < sizeof freq / sizeof *freq; j++)
            if (freq[j] >= i)
                printf("*");
            else
                printf(" ");
        printf("\n");
    }

    /* print letters */
    printf("     ");
    for (i = 0; i < sizeof freq / sizeof *freq; i++)
        printf("%c", (char)('A' + i));
    printf("\n");


    /*
     * close file
     * (since we opened the file only for read,
     * we assume that it is safe to not check against
     * the return value of fclose())
    */
    (void)fclose(fp);

    return EXIT_SUCCESS;
}
