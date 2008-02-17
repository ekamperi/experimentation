#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mydev.h>
#include <prop/proplib.h>

int main()
{
    struct mydev_params params;
    prop_dictionary_t pd;
    prop_string_t ps;
    int mydev_dev;

    params.number = 42;
    strcpy(params.string, "Hello World");

    if ((mydev_dev = open("/dev/mydev", O_RDONLY, 0)) < 0) {
        fprintf(stderr, "Failed to open /dev/mydev\n");
        exit(2);
    }

    if (ioctl(mydev_dev, MYDEVTEST, &params) < 0) {
        perror("ioctl failed");
        exit(2);
    }

    pd = prop_dictionary_create();

    ps = prop_string_create_cstring("key");
    prop_dictionary_set(pd, "value", ps);
    prop_object_release(ps);

    prop_dictionary_send_ioctl(pd, mydev_dev, MYDEVSETPROPS);

    prop_object_release(pd);
    close(mydev_dev);
    
    exit(0);
}
