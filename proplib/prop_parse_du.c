/*
 * [root dictionary]
 *     [child dictionary]
 *         [path]
 *         [size]
 *         [type]
 *     [child dictionary]
 *         [path]
 *         [size]
 *         [type]
 *     ...
 *
 * Compile with:
 * gcc prop_parse_du.c -o prop_parse_du -lprop -Wall -W -Wextra -ansi -pedantic
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <prop/proplib.h>

#define INIT_ROOT_CAPACITY 100    /* root's dict initial capacity */
#define INIT_CHILD_CAPACITY 3     /* child's dict initial capacity */
#define MAX_STR 100
#define MAX_TOKENS 3

/* */
FILE *fp = NULL;
prop_dictionary_t prd = NULL;    /* root dictionary */
prop_dictionary_t pcd = NULL;    /* child dictionary */
prop_number_t ps = NULL;         /* path name */
prop_string_t pn = NULL;         /* size in bytes */

/* Function prototypes */
static void cleanup(void);

int main(void)
{
    char str[MAX_STR];
    char *tokens[MAX_TOKENS];    /* for du(1) output parse */
    char *last, *p;
    int i, ret;

    /* Register cleanup function */
    if (atexit(cleanup) == -1) {
        perror("atexit()");
        exit(EXIT_FAILURE);
    }

    /* Initiate pipe stream to du(1) */
    fp = popen("du", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    /* Create root dictionary */
    prd = prop_dictionary_create_with_capacity(INIT_ROOT_CAPACITY);
    if (prd == NULL)
        err(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");

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

        /* Create child dictionary */
        pcd = prop_dictionary_create_with_capacity(INIT_CHILD_CAPACITY);
        if (pcd == NULL)
            err(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");

        /*
         * tokens[0] holds the size in bytes
         *
         * We use a signed prop_number_t object, so that
         * when externalized it will be represented as decimal
         * (unsigned numbers are externalized in base-16).
         *
         * Note: atoi() does not detect errors, but we trust
         * du(1) to provide us with valid input. Otherwise,
         * we should use strtol(3) or sscanf(3).
         */
        pn = prop_number_create_integer(atoi(tokens[0]));

        /* tokens[1] holds the path */
        ps = prop_string_create_cstring(tokens[1]);

        /* Add path to child dictionary */
        if (prop_dictionary_set(pcd, "path", ps) == FALSE)
            err(EXIT_FAILURE, "prop_dictionary_set()");

        /* Add size to child dictionary */
        if (prop_dictionary_set(pcd, "size in bytes", pn) == FALSE)
            err(EXIT_FAILURE, "prop_dictionary_set()");

        /* Add child dictionary to root dictionary */
        if (prop_dictionary_set(prd, tokens[1], pcd) == FALSE)
            err(EXIT_FAILURE, "prop_dictionary_set()");

        /* Release `pn' and `ps' */
        prop_object_release(pn);
        prop_object_release(ps);

        /* Release child dictionary */
        prop_object_release(pcd);
    }

    /* Externalize root dictionary to file in XML representation */
    if (prop_dictionary_externalize_to_file(prd, "./data.xml") == FALSE)
        err(EXIT_FAILURE, "prop_dictionary_externalize_to_file()");

    /* Release root dictionary */
    prop_object_release(prd);

    /* Close pipe stream */
    if  (pclose(fp) == -1)
        err(EXIT_FAILURE, "pclose()");

    return EXIT_SUCCESS;
}

void cleanup(void)
{
    /* Close pipe */
    if (fp != NULL) {
        if (pclose(fp) == -1)
            warn("pclose()");
    }

    /* Release proplib objects */
    if (prd != NULL)
        prop_object_release(prd);
    if (prd != NULL)
        prop_object_release(pcd);
    if (ps != NULL)
        prop_object_release(ps);
    if (pn != NULL)
        prop_object_release(pn);
}
