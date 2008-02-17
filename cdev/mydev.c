#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/mydev.h>
#include <prop/proplib.h>

struct mydev_softc {
    struct device mydev_dev;
};

/* Autoconfiguration glue */
void mydevattach(struct device *parent, struct device *self, void *aux);
int mydevopen(dev_t dev, int flags, int fmt, struct lwp *process);
int mydevclose(dev_t dev, int flags, int fmt, struct lwp *process);
int mydevioctl(dev_t dev, u_long cmd, caddr_t data,
		      int flags, struct lwp *process);

/* Just define the character dev handlers because that is all we need */
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
    /*
     * Nothing to do for mydev, this is where resources that
     * need to be allocated/initialised before open is called
     * can be set up.
    */
}

/*
 * Handle an open request on the dev.
 */
int
mydevopen(dev_t dev, int flags, int fmt, struct lwp *process)
{
    return 0; /* This always succeeds */
}

/*
 * Handle the close request for the dev.
 */
int
mydevclose(dev_t dev, int flags, int fmt, struct lwp *process)
{
    return 0; /* Again this always succeeds */
}

/*
 * Handle the ioctl for the dev.
 */
int
mydevioctl(dev_t dev, u_long cmd, caddr_t data, int flags,
           struct lwp *process)
{
    prop_dictionary_t dict, odict;
    prop_object_t po;
    struct mydev_params *params = (struct mydev_params *)data;
    const struct plistref *pref;
    int error;
    char *val;

    error = 0;
    switch (cmd) {

    case MYDEVTEST:
        printf("Got number of %d and string of %s\n",
               params->number, params->string);
        break;

    case MYDEVSETPROPS:
        pref = (const struct plistref *)data;
        error = prop_dictionary_copyin_ioctl(pref, cmd, &dict);
        if (error)
            return error;
        odict = mydevprops;
        mydevprops = dict;
        prop_object_release(odict);

        po = prop_dictionary_get(mydevprops, "key");

        val = prop_string_cstring(po);
        printf("<key, val> = (%s, %s)\n", "key", val);

        break;

    default:
        error = ENODEV;
    }

    return error;
}
