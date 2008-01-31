#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <prop/proplib.h>

#define INIT_CAPACITY 100
#define MAX_STR 100
#define MAX_TOKENS 3

int main(void)
{
    char str[MAX_STR];
    char *tokens[MAX_TOKENS];    /* for du(1) output parse */
    char *last, *p;
    int i, ret;
    FILE *fp;
    prop_dictionary_t pd;
    prop_number_t pn;

    /* Initiate pipe stream to du(1) */
    fp = popen("du", "r");
    if (fp == NULL) {
        perror("popen()");
        exit(EXIT_FAILURE);
    }

    /* Create dictionary */
    pd = prop_dictionary_create_with_capacity(INIT_CAPACITY);
    if (pd == NULL) {
        pclose(fp);
        errx(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");
    }

    /* Read from stream */
    while (fgets(str, MAX_STR, fp) != NULL) {
        /* Parse output of du(1) command */
        i = 0;
        p = strtok_r(str, "\t", &last);
        while (p && i < MAX_TOKENS - 1) {
            tokens[i++] = p;
            p = strtok_r(NULL, "\t", &last);
        }
        tokens[i] = NULL;

        /* Trim '\n' from tokens[1] */
        (tokens[1])[strlen(tokens[1]) - 1] = '\0';

        /*
         * We use a signed prop_number_t object, so that
         * when externalized it will be represented as decimal
         * (unsigned numbers are externalized in base-16).
         *
         * Note: atoi() does not detect errors, but we trust
         * du(1) to provide us with valid input. Otherwise,
         * we should use strtol(3) or sscanf(3).
         */
        pn = prop_number_create_integer(atoi(tokens[0]));

        /* Add a <path, size> pair in our dictionary */
        if (prop_dictionary_set(pd, tokens[1], pn) == FALSE) {
            prop_object_release(pn);
            prop_object_release(pd);
            errx(EXIT_FAILURE, "prop_dictionary_set()");
        }

        /* Release prop_number_t object */
        prop_object_release(pn);
    }

    /* Externalize dictionary to file in XML representation */
    if (prop_dictionary_externalize_to_file(pd, "./data.xml") == FALSE) {
        prop_object_release(pd);
        errx(EXIT_FAILURE, "prop_dictionary_externalize_to_file()");
    }

    /* Release dictionary */
    prop_object_release(pd);

    /* Close pipe stream */
    ret = pclose(fp);
    if  (ret == -1) {
        perror("pclose()");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
