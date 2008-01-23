#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>    /* for waitpid() macros */
#include <prop/proplib.h>

#define INIT_CAPACITY 100
#define MAX_STR 100
#define MAX_TOKENS 3

int main(void)
{
    char str[MAX_STR];
    char *tokens[MAX_TOKENS];    /* for ``du'' output parse */
    char *last, *p, *s;
    int i, j, ret;
    FILE *fp;
    prop_dictionary_t pd;
    prop_string_t ps;

    /* Initiate pipe stream to ``du'' */
    fp = popen("du", "r");
    if (fp == NULL) {
        perror("popen()");
        exit(EXIT_FAILURE);
    }

    /* Create dictionary */
    pd = prop_dictionary_create_with_capacity(INIT_CAPACITY);
    if (pd == NULL) {
        pclose(fp);
        prop_object_release(pd);
        errx(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");
    }

    /* Read from stream */
    while (fgets(str, MAX_STR, fp) != NULL) {
        /* Parse output of ``du'' command */
        i = 0;
        for ((p = strtok_r(str, "\t", &last)); p;
             (p = strtok_r(NULL, "\t", &last)), i++) {
            if (i < MAX_TOKENS - 1)
                tokens[i] = p;
        }
        tokens[i] = NULL;

        /* Trim '\n' from tokens[1] */
        (tokens[1])[strlen(tokens[1]) - 1] = '\0';

        ps = prop_string_create_cstring(tokens[0]);
        prop_dictionary_set(pd, tokens[1], ps);
        prop_object_release(ps);

        printf("%s\n", s);
    }

    prop_dictionary_externalize_to_file(pd, "./data.xml");
    prop_object_release(pd);

    /* Close pipe stream */
    ret = pclose(fp);
    if  (ret == -1) {
        perror("pclose()");
        exit(EXIT_FAILURE);
    }
    else {
        /*
         * Use macros described under wait() to inspect the return value
         * of pclose() in order to determine success/failure of command
         * executed by popen().
         */

        if (WIFEXITED(ret)) {
            printf("Child exited, ret = %d\n", WEXITSTATUS(ret));
        } else if (WIFSIGNALED(ret)) {
            printf("Child killed, signal %d\n", WTERMSIG(ret));
        } else if (WIFSTOPPED(ret)) {
            printf("Child stopped, signal %d\n", WSTOPSIG(ret));
        }
        /* Not all implementations support this */
#ifdef WIFCONTINUED
        else if (WIFCONTINUED(ret)) {
            printf("Child continued\n");
        }
#endif
        /* Non standard case -- may never happen */
        else {
            printf("Unexpected return value, ret = 0x%x\n", ret);
        }
    }

    return EXIT_SUCCESS;
}
