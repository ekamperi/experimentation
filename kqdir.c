#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>    /* for strerror() */

#define MAX_ENTRIES 256

void diep(const char *s);

int main(int argc, char *argv[])
{
    struct kevent evlist[MAX_ENTRIES];    /* events we want to monitor */
    struct kevent chlist[MAX_ENTRIES];    /* events that were triggered */
    struct dirent *pdent;
    DIR *pdir;
    char fullpath[256];
    int fdlist[MAX_ENTRIES], cnt, kq, nev, i;

    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* create a new kernel event queue */
    if ((kq = kqueue()) == -1)
        diep("kqueue");

    /* */
    if ((pdir = opendir(argv[1])) == NULL)
        perror("opendir");

    /* Skip . and .. */
    cnt = 0;
    while((pdent = readdir(pdir)) != NULL && cnt++ < 2)
        ;

    /* */
    cnt = 0;
    while((pdent = readdir(pdir)) != NULL) {
        if (cnt > MAX_ENTRIES - 1) {
            fprintf(stderr, "Max number of entries exceeded\n");
            for (i = 0; i < cnt; i++)
                close(fdlist[i]);
            closedir(pdir);
            close(kq);
            exit(EXIT_FAILURE);
        }

        if (strlen(argv[1] + strlen(pdent->d_name) + 1) > 256) {
            fprintf(stderr,"Max path length exceeded\n");
            exit(EXIT_FAILURE);
        }
        strcpy(fullpath, argv[1]);
        strcat(fullpath, "/");
        strcat(fullpath, pdent->d_name);

        if ((fdlist[cnt] = open(fullpath, O_RDONLY)) == -1)
            perror("open");

        EV_SET(&chlist[cnt], fdlist[cnt], EVFILT_VNODE,
               EV_ADD | EV_ENABLE | EV_ONESHOT,
               NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
               0, 0);

        cnt++;
    }

    /* loop forever */
    for (;;) {
        nev = kevent(kq, chlist, cnt, evlist, cnt, NULL);
        if (nev == -1)
            perror("kevent");

        else if (nev > 0) {
            for (i = 0; i < nev; i++) {
                if (evlist[i].flags & EV_ERROR) {
                    fprintf(stderr, "EV_ERROR: %s\n", strerror(evlist[i].data));
                    exit(EXIT_FAILURE);
                }

                if (evlist[i].fflags & NOTE_DELETE) {
                    printf("fd: %d Deleted\n", evlist[i].ident);
                }
                else if (evlist[i].fflags & NOTE_EXTEND ||
                    evlist[i].fflags & NOTE_WRITE) {
                    printf("fd: %d Modified\n", evlist[i].ident);
                }
                else if (evlist[i].fflags & NOTE_ATTRIB) {
                    printf("fd: %d Attributes modified\n", evlist[i].ident);
                }
            }
        }
    }

    /* */
    for (i = 0; i < cnt; i++)
        close(fdlist[i]);
    closedir(pdir);
    close(kq);

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
