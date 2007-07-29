#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 1024

/* function prototypes */
void diep(const char *s);
int tcpopen(const char *host, int port);
void sendbuftosck(int sckfd, const char *buf, int len);

int main(int argc, char *argv[])
{
    struct kevent chlist[2];   /* events we want to monitor */
    struct kevent evlist[2];   /* events that were triggered */
    char buf[BUFSIZE];
    int sckfd, kq, nev, i;

    /* check argument count */
    if (argc != 3) {
        fprintf(stderr, "usage: %s host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open a connection to a host:port pair */
    sckfd = tcpopen(argv[1], atoi(argv[2]));

    /* create a new kernel event queue */
    if ((kq = kqueue()) == -1)
        diep("kqueue()");

    /* initialise kevent structures */
    EV_SET(&chlist[0], sckfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    EV_SET(&chlist[1], fileno(stdin), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    /* loop forever */
    for (;;) {
        nev = kevent(kq, chlist, 2, evlist, 2, NULL);

        if (nev < 0)
            diep("kevent()");

        else if (nev > 0) {
            if (evlist[0].flags & EV_EOF)                       /* read direction of socket has shutdown */
                exit(EXIT_FAILURE);

            for (i = 0; i < nev; i++) {
                if (evlist[i].flags & EV_ERROR) {                /* report errors */
                    fprintf(stderr, "EV_ERROR: %s\n", strerror(evlist[i].data));
                    exit(EXIT_FAILURE);
                }

                if (evlist[i].ident == sckfd) {                  /* we have data from the host */
                    memset(buf, 0, BUFSIZE);
                    if (read(sckfd, buf, BUFSIZE) < 0)
                        diep("read()");
                    fputs(buf, stdout);
                }

                else if (evlist[i].ident == fileno(stdin)) {     /* we have data from stdin */
                    memset(buf, 0, BUFSIZE);
                    fgets(buf, BUFSIZE, stdin);
                    sendbuftosck(sckfd, buf, strlen(buf));
                }
            }
        }
    }

    close(kq);
    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int tcpopen(const char *host, int port)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sckfd;

    if ((hp = gethostbyname(host)) == NULL)
        diep("gethostbyname()");

    if ((sckfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        diep("socket()");

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)hp->h_addr);
    memset(&(server.sin_zero), 0, 8);

    if (connect(sckfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
        diep("connect()");

    return sckfd;
}

void sendbuftosck(int sckfd, const char *buf, int len)
{
    int bytessent, pos;

    pos = 0;
    do {
        if ((bytessent = send(sckfd, buf + pos, len - pos, 0)) < 0)
            diep("send()");
        pos += bytessent;
    } while (bytessent > 0);
}
