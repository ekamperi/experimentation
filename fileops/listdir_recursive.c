/* compile with:
   gcc listdir_recursive.c -o listdir -Wall -W -Wextra -ansi -pedantic */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int listdir(const char *path)
{
    struct dirent *pdent;
    DIR *pdir;
    char *newpath = NULL;
    static unsigned int dirdepth = 0;
    unsigned int i;

    /* open directory named by path, associate a directory stream
       with it and return a pointer to it
    */
    if ((pdir = opendir(path)) == NULL) {
        printf("%s\n", path);
        perror("opendir");
        return -1;
    }

    /* get all directory entries */
    while((pdent = readdir(pdir)) != NULL) {
        /* indent according to the depth we are */
        for (i = 0; i < dirdepth; i++)
            printf("  ");

        /* print current entry, or [entry] if it's a directory */
        if (pdent->d_type == DT_DIR)
            printf("[%s]\n", pdent->d_name);
        else
            printf("%s\n", pdent->d_name);

        /* Is it a directory ? If yes, list it */
        if (pdent->d_type == DT_DIR
            && strcmp(pdent->d_name, ".")
            && strcmp(pdent->d_name, "..")) {
            dirdepth++;

            /* allocate memory for new path */
            if ((newpath = malloc(strlen(path) + strlen(pdent->d_name) + 1)) == NULL) {
                perror("malloc");
                return -1;
            }

            /* construct new path */
            strcpy(newpath, path);
            strcat(newpath, "/");
            strcat(newpath, pdent->d_name);


            printf("Calling with [%s]\n", newpath);

            /* to iterate is human, to recurse, divine */
            if (listdir(newpath) == -1) {
                closedir(pdir);
                free(newpath);
                return -1;
            }
        }
    }

    closedir(pdir);
    if (newpath != NULL)
        free(newpath);

    dirdepth--;
    return 1;
}

int main(int argc, char *argv[])
{
    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    (void)listdir(argv[1]);

    return EXIT_SUCCESS;
}
