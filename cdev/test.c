#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mydev.h>

int main()
{
    struct mydev_params params;
    int mydev_dev;

    params.number = 42;
    strcpy(params.string, "Hello World");

    if ((mydev_dev = open("/dev/mydev", O_RDONLY, 0)) < 0) {
        fprintf(stderr, "Failed to open /dev/skel\n");
        exit(2);
    }

    if (ioctl(mydev_dev, MYDEVTEST, &params) < 0) {
        perror("ioctl failed");
        exit(2);
    }
    
    exit(0);
}
