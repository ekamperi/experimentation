/* compile with:
   gcc readwd.c -o readwd -Wall -W -Wextra -ansi -pedantic */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dev/ata/atareg.h>
#include <sys/ataio.h>
#include <sys/ioctl.h>
#include <sys/param.h>

int main(int argc, char *argv[])
{
    int fd;
    unsigned int i;
    struct atareq req;
    static union {
        unsigned char inbuf[DEV_BSIZE];
        struct ataparams inqbuf;
    } inbuf;
    u_int16_t *p;

    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "usage: %s /dev/file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open device file descriptor */
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* construct an ata request */
    memset(&req, 0, sizeof req);
    req.flags = ATACMD_READ;
    req.command = WDCC_IDENTIFY;
    req.databuf = (caddr_t) &inbuf;
    req.datalen = sizeof(inbuf);
    req.timeout = 1000;    /* 1 sec */

    if (ioctl(fd, ATAIOCCOMMAND, &req) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    /* print the model */
    for (i = 0; i < sizeof(inbuf.inqbuf.atap_model); i+=2) {
        p = (u_int16_t *) (inbuf.inqbuf.atap_model + i);
        *p = ntohs(*p);
    }

    for (i = 0; i < sizeof(inbuf.inqbuf.atap_model); i++)
        printf("%c", inbuf.inqbuf.atap_model[i]);

    return EXIT_SUCCESS;
}
