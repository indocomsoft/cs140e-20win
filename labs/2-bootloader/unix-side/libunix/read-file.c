#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read entire file into buffer.  return it.   zero pads to a
// multiple of 4.
//
// make sure to cleanup!
uint8_t* read_file(unsigned* size, const char* name)
{
    struct stat file_stat;
    int fd;
    uint8_t* buf;

    if (stat(name, &file_stat) == -1) {
        sys_die(stat, "Unable to stat file <%s>\n", name);
    }
    *size = file_stat.st_size;

    if ((buf = calloc(1, roundup(file_stat.st_size, 4))) == NULL) {
        sys_die(malloc, "Failed to allocate memory");
    }

    if ((fd = open(name, O_RDONLY)) == -1) {
        sys_die(open, "Unable to open file <%s>\n", name);
    }

    if (read(fd, buf, file_stat.st_size) == -1) {
        sys_die(read, "Unable to read file <%s>\n", name);
    }

    return buf;
}
