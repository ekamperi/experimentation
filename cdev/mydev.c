#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/mydev.h>

struct mydev_softc {
	struct device	mydev_dev;
};

/* Autoconfiguration glue */
void	mydevattach(struct device *parent, struct device *self, void *aux);
int     mydevopen(dev_t device, int flags, int fmt, struct lwp *process);
int     mydevclose(dev_t device, int flags, int fmt, struct lwp *process);
int     mydevioctl(dev_t device, u_long command, caddr_t data,
		      int flags, struct lwp *process);

/* just define the character device handlers because that is all we need */
const struct cdevsw mydev_cdevsw = {
        mydevopen,
	mydevclose,
	noread,
	nowrite,
        mydevioctl,
	nostop,
	notty,
	nopoll,
	nommap,
	nokqfilter,
        0 /* int d_type; */
};

/*
 * Attach for autoconfig to find.
 */
void
mydevattach(struct device *parent, struct device *self, void *aux)
{
	  /* nothing to do for mydev, this is where resources that
	     need to be allocated/initialised before open is called
	     can be set up */
}

/*
 * Handle an open request on the device.
 */
int
mydevopen(dev_t device, int flags, int fmt, struct lwp *process)
{
	return 0; /* this always succeeds */
}

/*
 * Handle the close request for the device.
 */
int
mydevclose(dev_t device, int flags, int fmt, struct lwp *process)
{
	return 0; /* again this always succeeds */
}

/*
 * Handle the ioctl for the device
 */
int
mydevioctl(dev_t device, u_long command, caddr_t data, int flags,
	      struct lwp *process)
{
	int error;
	struct mydev_params *params = (struct mydev_params *)data;

	error = 0;
	switch (command) {
		case MYDEVTEST:
			printf("Got number of %d and string of %s\n",
			       params->number, params->string);
			break;

		default:
			error = ENODEV;
	}

	return (error);
}
