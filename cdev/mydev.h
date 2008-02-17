/*
 *
 * Definitions for the Mydev pseudo device.
 *
 */
#include <sys/param.h>
#include <sys/device.h>

#ifndef MYDEV_H
#define MYDEV_H 1

struct mydev_params
{
	int number;
	char string[80];
};

#define MYDEVTEST _IOW('S', 0x1, struct mydev_params)

#ifdef _KERNEL

/*
 * Put kernel inter-module interfaces here, this
 * pseudo device has none.
 */

#endif
#endif
