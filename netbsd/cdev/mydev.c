#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/malloc.h>    /* for free(9) */
#include <sys/mydev.h>
#include <sys/kauth.h>
#include <sys/syslog.h>
#include <prop/proplib.h>

struct mydev_softc {
    struct device mydev_dev;
};

/* Autoconfiguration glue */
void mydevattach(struct device *parent, struct device *self, void *aux);
int mydevopen(dev_t dev, int flags, int fmt, struct lwp *proc);
int mydevclose(dev_t dev, int flags, int fmt, struct lwp *proc);
int mydevioctl(dev_t dev, u_long cmd, caddr_t data, int flags,
		      struct lwp *proc);

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

/* Count of number of times device is open */
static unsigned int mydev_usage = 0;

/*
 * Attach for autoconfig to find.
 */
void
mydevattach(struct device *parent, struct device *self, void *aux)
{
    /*
     * This is where resources that need to be allocated/initialised
     * before open is called can be set up.
    */
    mydev_usage = 0;
    log(LOG_DEBUG, "mydev: pseudo-device attached\n");
}

/*
 * Handle an open request on the dev.
 */
int
mydevopen(dev_t dev, int flags, int fmt, struct lwp *proc)
{
    log(LOG_DEBUG, "mydev: pseudo-device open attempt by "
        "uid=%u, pid=%u. (dev=%u)\n",
        kauth_cred_geteuid(proc->l_cred), proc->l_proc->p_pid,
        dev);

    if (mydev_usage > 0) {
        log(LOG_DEBUG, "mydev: pseudo-device already in use\n");
        return EBUSY;
    }

    return 0;    /* Success */
}

/*
 * Handle the close request for the dev.
 */
int
mydevclose(dev_t dev, int flags, int fmt, struct lwp *proc)
{
    if (mydev_usage > 0)
        mydev_usage--;

    log(LOG_DEBUG, "mydev: pseudo-device closed\n");

    return 0;
}

/*
 * Handle the ioctl for the dev.
 */
int
mydevioctl(dev_t dev, u_long cmd, caddr_t data, int flags,
           struct lwp *proc)
{
    prop_dictionary_t dict;
    prop_string_t ps;
    struct mydev_params *params;
    const struct plistref *pref;
    int error = 0;
    char *val;

    switch (cmd) {
    case MYDEVOLDIOCTL:
        /* Pass data from userspace to kernel in the conventional way */
        params = (struct mydev_params *)data;
        log(LOG_DEBUG, "Got number of %d and string of %s\n",
            params->number, params->string);
        break;

    case MYDEVSETPROPS:
        /* Use proplib(3) for userspace/kernel communication */
        pref = (const struct plistref *)data;
        error = prop_dictionary_copyin_ioctl(pref, cmd, &dict);
        if (error)
            return error;

        /* Print dict's count for debugging purposes */
        log(LOG_DEBUG, "mydev: dict count = %u\n",
            prop_dictionary_count(dict));

        /* Retrieve object associated with "key" key */
        ps = prop_dictionary_get(dict, "key");
        if (ps == NULL || prop_object_type(ps) != PROP_TYPE_STRING) {
            prop_object_release(dict);
            log(LOG_DEBUG, "mydev: prop_dictionary_get() failed\n");
            return -1;
        }

        /* Print data */
        val = prop_string_cstring(ps);
        prop_object_release(ps);
        log(LOG_DEBUG, "<x, y> = (%s, %s)\n", "key", val);
        free(val, M_TEMP);

        /* Done */
        prop_object_release(dict);
        break;

    default:
        /* Invalid operation */
        error = ENODEV;
    }

    return error;
}
