#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include <sys/types.h>
#include <stdint.h>

#include "notetaker_internal.h"

struct notetaker {
    int fds[NOTETAKER_IFACE_COUNT];
};

int notetaker_close(notetaker_t *notetaker)
{
    int i;
    int retval = 0;

    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        int fd = notetaker->fds[i];
        if (fd != -1) {
            retval = retval ? retval : close(fd);
        }
    }
    free(notetaker);
    return retval;
}

notetaker_t *notetaker_open(char **hidraw_paths, int *notetaker_errno) {
    int i;
    int original_errno;
    notetaker_t *notetaker;

    notetaker = (notetaker_t *) malloc(sizeof(notetaker_t));
    if (notetaker == NULL)
        return NULL;

    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        notetaker->fds[i] = -1;
    }

    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        int fd;
        const char *path = hidraw_paths[i];
        struct hidraw_devinfo devinfo;
        memset(&devinfo, 0, sizeof(struct hidraw_devinfo));

        if ((fd = open(path, O_RDWR)) == -1)
            goto err;

        if (ioctl(fd, HIDIOCGRAWINFO, &devinfo))
            goto err;

        if (memcmp(&devinfo, &DEVINFO_M210, sizeof(struct hidraw_devinfo)) != 0) {
            if (notetaker_errno != NULL)
                *notetaker_errno = -NOTETAKER_ERRNO_UNKNOWN_DEVICE;
            goto err;
        }

        notetaker->fds[i] = fd;
    }

    return notetaker;

  err:
    original_errno = errno;
    notetaker_close(notetaker);
    errno = original_errno;
    return NULL;
}

int notetaker_get_version_info(notetaker_t *notetaker,
                               struct notetaker_version_info *version_info)
{
    ssize_t wbytes, rbytes;
    struct version_response response;
    uint8_t request[] = {0x02, 0x01, 0x95};
    int fd = notetaker->fds[NOTETAKER_IFACE_PAD];
    memset(&response, 0, sizeof(struct version_response));

    wbytes = write(fd, request, sizeof(request));
    if (wbytes == -1)
        return -1;
    if (wbytes != sizeof(request))
        return -NOTETAKER_ERRNO_INCOMPLETE_WRITE;

    rbytes = read(fd, &response, sizeof(struct version_response));
    if (rbytes == -1)
        return -1;
    if (rbytes != IFACE_PACKET_SIZES[NOTETAKER_IFACE_PAD])
        return -NOTETAKER_ERRNO_INCOMPLETE_READ;

    version_info->firmware_version = be16toh(response.firmware_version);
    version_info->pen_version = be16toh(response.analog_version);
    version_info->pad_version = be16toh(response.pad_version);
    version_info->mode = response.tab_mode;

    return 0;
}
