#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mydev.h>
#include <prop/proplib.h>

int main(void)
{
    struct mydev_params params;
    prop_dictionary_t pd;
    prop_string_t ps;
    int devfd;

    params.number = 42;
    strcpy(params.string, "Hello World");

    /* Open device */
    if ((devfd = open("/dev/mydev", O_RDONLY, 0)) < 0) {
        fprintf(stderr, "Failed to open /dev/mydev\n");
        exit(EXIT_FAILURE);
    }

    /* Send ioctl request in the traditional way */
    if (ioctl(devfd, MYDEVTEST, &params) < 0) {
        close(devfd);
        err("ioctl()");
    }

    /* Create dictionary and add a <key, value> pair in it */
    pd = prop_dictionary_create();
    if (pd == NULL) {
        close(devfd);
        err("prop_dictionary_create()");
    }

    ps = prop_string_create_cstring("value");
    if (ps == NULL) {
        close(devfd);
        prop_object_release(pd);
        err("prop_string_create_cstring()");
    }

    if (prop_dictionary_set(pd, "key", ps) == false) {
        close(devfd);
        prop_object_release(ps);
        prop_object_release(pd);
        err("prop_dictionary_set()");
    }

    prop_object_release(ps);

    /* Send dictionary to kernel space */
    prop_dictionary_send_ioctl(pd, devfd, MYDEVSETPROPS);

    prop_object_release(pd);

    /* Close device */
    if (close(devfd) == -1) {
        err("close(");
        err(EXIT_FAILURE, "close()");
    }
    
    return EXIT_SUCCESS;
}
