#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/disklabel.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
    struct disklabel dklbl;
    int fd;

    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "usage: %s /dev/file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open device file */
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* get disklabel by calling a disk-specific ioctl */
    if (ioctl(fd, DIOCGDINFO, &dklbl) == -1) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Disk: %s\n", dklbl.d_typename);

    return EXIT_SUCCESS;
}
