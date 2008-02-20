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
int mydevioctl(dev_t dev, u_long cmd, caddr_t data, int flags,
		      struct lwp *process);

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
    0    /* int d_type; */
};

/*
 * Attach for autoconfig to find.
 */
void
mydevattach(struct device *parent, struct device *self, void *aux)
{
    /*
     * Nothing to do for mydev.
     * This is where resources that need to be allocated/initialised
     * before open is called can be set up.
    */
}

/*
 * Handle an open request on the dev.
 */
int
mydevopen(dev_t dev, int flags, int fmt, struct lwp *process)
{
    return 0;    /* This always succeeds */
}

/*
 * Handle the close request for the dev.
 */
int
mydevclose(dev_t dev, int flags, int fmt, struct lwp *process)
{
    return 0;    /* Again this always succeeds */
}

/*
 * Handle the ioctl for the dev.
 */
int
mydevioctl(dev_t dev, u_long cmd, caddr_t data, int flags,
           struct lwp *process)
{
    prop_dictionary_t dict;
    prop_string_t ps;
    struct mydev_params *params;
    const struct plistref *pref;
    int error;
    char *val;

    error = 0;
    switch (cmd) {
    case MYDEVOLDIOCTL:
        /* Pass data from userspace to kernel in the conventional way */
        params = (struct mydev_params *)data;
        printf("Got number of %d and string of %s\n",
               params->number, params->string);
        break;

    case MYDEVSETPROPS:
        /* Use proplib(3) for userspace/kernel communication */
        pref = (const struct plistref *)data;
        error = prop_dictionary_copyin_ioctl(pref, cmd, &dict);
        if (error)
            return error;

        /* Print dict's count for debugging purposes */
        printf("count = %u\n", prop_dictionary_count(dict));

        /* Retrieve object associated with "key" key */
        ps = prop_dictionary_get(dict, "key");
        if (ps == NULL || prop_object_type(ps) != PROP_TYPE_STRING) {
            prop_object_release(dict);
            printf("prop_dictionary_get()\n");
            return -1;
        }

        /* Print data */
        val = prop_string_cstring(ps);
        prop_object_release(ps);
        printf("<key, val> = (%s, %s)\n", "key", val == NULL ? "null" : val);

        /* Done */
        prop_object_release(dict);
        break;

    default:
        /* Invalid operation */
        error = ENODEV;
    }

    return error;
}
