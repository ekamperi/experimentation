#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#define PASSLEN 64

/* function prototypes */
void diep(const char *s);

int main(int argc, char *argv[])
{
    struct termios oldt, newt;
    char password[PASSLEN];
    int fd;

    /* check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s tty\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open terminal device */
    if ((fd = open(argv[1], O_RDONLY | O_NOCTTY) == -1))
        diep("open");

    /* get current termios structure */
    if (tcgetattr(fd, &oldt) == -1)
        diep("tcgetattr");

    /* set new termios structure */
    newt = oldt;
    newt.c_lflag &= ~ECHO;     /* disable echoing */
    newt.c_lflag |= ECHONL;    /* echo NL even if ECHO is off */

    if (tcsetattr(fd, TCSANOW, &newt) == -1)
        diep("tcsetattr");

    /* prompt for password and get it*/
    printf("Password: ");
    fgets(password, PASSLEN, stdin);

    /* restole old termios structure */
    if (tcsetattr(fd, TCSANOW, &oldt) == -1)
        diep("tcsetattr");

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
