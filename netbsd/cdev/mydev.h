/*
 *
 * Definitions for the Mydev pseudo device.
 *
 */
#include <sys/param.h>
#include <sys/device.h>

#ifndef MYDEV_H
#define MYDEV_H

struct mydev_params
{
    int number;
    char string[80];
};

#define MYDEVOLDIOCTL _IOW('M', 0x1, struct mydev_params)
#define MYDEVSETPROPS _IOW('M', 0x2, struct plistref)

#ifdef _KERNEL

/*
 * Put kernel inter-module interfaces here, this
 * pseudo device has none.
 */

#endif    /* _KERNEL */
#endif    /* MYDEV_H */
