/* compile with:
   gcc listdir_recursive.c -o listdir -Wall -W -Wextra -ansi -pedantic */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH 256

void listdir(const char *path)
{
    struct dirent *pdent;
    DIR *pdir;
    char newpath[MAX_PATH];
    static unsigned int dirdepth = 0;
    unsigned int i;

    /* open directory named by path, associate a directory stream
       with it and return a pointer to it
    */
    if ((pdir = opendir(path)) == NULL) {
        perror("opendir");
       exit(EXIT_FAILURE);
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

            strcpy(newpath, path);
            strcat(newpath, "/");
            strcat(newpath, pdent->d_name);

            listdir(newpath);
        }
    }

    closedir(pdir);
    dirdepth--;
}

int main(int argc, char *argv[])
{
    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listdir(argv[1]);

    return EXIT_SUCCESS;
}
