// engler, cs140e.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char* ttyusb_prefixes[] = {
    "ttyUSB", // linux
    "cu.SLAB_USB", // mac os
    0
};

static int filter(const struct dirent* d)
{
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    for (int i = 0; ttyusb_prefixes[i] != 0; i++) {
        const char* prefix = ttyusb_prefixes[i];
        size_t n = strlen(prefix);
        if (strncmp(d->d_name, prefix, n) == 0) {
            return 1;
        }
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// panic's if 0 or more than 1.
//
char* find_ttyusb(void)
{
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent** namelist;
    int n = scandir("/dev/", &namelist, filter, alphasort);
    if (n == -1) {
        sys_die(scandir, "scandir() failed");
    }
    if (n == 0 || n > 1) {
        panic("expected to match 1 prefix, instead there are %d\n", n);
    }
    size_t len = strlen(namelist[0]->d_name);
    char* p = malloc(sizeof "/dev/" + len);
    strcpy(p, "/dev/");
    strcat(p, namelist[0]->d_name);

    free(namelist[0]);
    free(namelist);

    return p;
}
