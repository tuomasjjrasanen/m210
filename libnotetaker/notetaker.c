#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <linux/hidraw.h>
#include <linux/input.h>  /* BUS_USB */
#include <linux/limits.h> /* PATH_MAX */

#include <libudev.h>

#include "notetaker.h"

/* #define NOTETAKER_MODE_NONE   0x00 */
/* #define NOTETAKER_MODE_MOBILE 0x03 */

/* #define NOTETAKER_LED_NONE  0x00 */
/* #define NOTETAKER_LED_PEN   0x01 */
/* #define NOTETAKER_LED_MOUSE 0x02 */

/* #define NOTETAKER_STATUS_NONE         0x00 */
/* #define NOTETAKER_STATUS_BATTERY_LOW  0x01 */
/* #define NOTETAKER_STATUS_BATTERY_GOOD 0x02 */

#define NT_RPT_INFO     0x95
#define NT_RPT_ERASE    0xB0
#define NT_RPT_UPLOAD   0xB5
#define NT_RPT_ACK      0xB6
#define NT_RPT_NACK     0xB7
#define NT_RPT_MODE1    0x80
#define NT_RPT_MODE2    0xB5
#define NT_RPT_SCALE1   0x80
#define NT_RPT_SCALE2   0xB6

#define NT_MAX_RESPONSE_SIZE 64

static const struct hidraw_devinfo DEVINFO_M210 = {
    BUS_USB,
    0x0e20,
    0x0101,
};

struct info_response {
    uint8_t special_command;
    uint8_t command_version;
    uint8_t product_id;
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t analog_product_id;
    uint8_t mode;
} __attribute__ ((packed));

struct notetaker {
    int fds[NOTETAKER_IFACE_COUNT];
};

static notetaker_err_t nt_write_rpt(struct notetaker *notetaker, uint8_t *rpt,
                                    size_t rpt_size)
{
    notetaker_err_t retval = err_sys;
    uint8_t *request;
    size_t request_size = rpt_size + 3;

    request = (uint8_t *) malloc(request_size);
    if (request == NULL)
        return retval;

    request[0] = 0x00;     /* Without this, response is not sent. Why?? */
    request[1] = 0x02;     /* report id */
    request[2] = rpt_size;

    /* Copy report paylod to the end of the request. */
    memcpy(request + 3, rpt, rpt_size);

    /* Send request to the interface 0. */
    if (write(notetaker->fds[0], request, request_size) == -1)
        goto err;

    retval = err_ok;
  err:
    free(request);
    return retval;
}

static notetaker_err_t nt_read_rpt(struct notetaker *notetaker, void *response,
                                   size_t response_size)
{
    uint8_t buf[NT_MAX_RESPONSE_SIZE];
    memset(buf, 0, NT_MAX_RESPONSE_SIZE);
    memset(response, 0, response_size);

    if (read(notetaker->fds[0], buf, NT_MAX_RESPONSE_SIZE) == -1)
        return err_sys;

    if (response_size > NT_MAX_RESPONSE_SIZE)
        memcpy(response, buf, NT_MAX_RESPONSE_SIZE);
    else
        memcpy(response, buf, response_size);

    return err_ok;
}

notetaker_err_t notetaker_free(struct notetaker *notetaker)
{
    int i;

    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        int fd = notetaker->fds[i];
        if (fd != -1) {
            if (close(fd) == -1)
                return err_sys;
        }
    }
    free(notetaker);
    return err_ok;
}

notetaker_err_t notetaker_open_from_hidraw_paths(struct notetaker **notetaker,
                                                 char **hidraw_paths)
{
    int retval = err_sys;
    int i;
    int original_errno;

    *notetaker = (struct notetaker *) malloc(sizeof(struct notetaker));
    if (*notetaker == NULL)
        return err_sys;

    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        (*notetaker)->fds[i] = -1;
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
            retval = err_baddev;
            goto err;
        }

        (*notetaker)->fds[i] = fd;
    }

    return err_ok;

  err:
    original_errno = errno;
    switch (notetaker_free(*notetaker)) {
    case err_sys:
        free(*notetaker);
        break;
    default:
        break;
    }
    errno = original_errno;
    return retval;
}

notetaker_err_t nt_find_hidraw_devnode(char *found, int iface, char *path,
                                       size_t path_size)
{
    int retval = err_sys;
    struct udev_list_entry *list_entry = NULL;
    struct udev_enumerate *enumerate = NULL;
    struct udev *udev = NULL;

    udev = udev_new();
    if (udev == NULL)
        goto out;

    enumerate = udev_enumerate_new(udev);
    if (enumerate == NULL)
        goto out;

    if (udev_enumerate_add_match_subsystem(enumerate, "hidraw"))
        goto out;

    if (udev_enumerate_scan_devices(enumerate))
        goto out;

    list_entry = udev_enumerate_get_list_entry(enumerate);

    while (list_entry != NULL) {
        int ifc;
        uint16_t vendor;
        uint16_t product;
        const char *syspath = udev_list_entry_get_name(list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, syspath);
        const char *devnode = udev_device_get_devnode(dev);
        struct udev_device *parent = udev_device_get_parent(dev);

        parent = udev_device_get_parent(parent); /* Second parent: usb */
        ifc = atoi(udev_device_get_sysattr_value(parent, "bInterfaceNumber"));
        parent = udev_device_get_parent(parent);
        vendor = strtol(udev_device_get_sysattr_value(parent, "idVendor"), NULL, 16);
        product = strtol(udev_device_get_sysattr_value(parent, "idProduct"), NULL, 16);
        if (vendor == DEVINFO_M210.vendor
            && product == DEVINFO_M210.product
            && iface == ifc) {
            retval = err_ok;
            *found = 1;
            strncpy(path, devnode, path_size);
            udev_device_unref(dev);
            goto out;
        }
        list_entry = udev_list_entry_get_next(list_entry);
        udev_device_unref(dev);
    }
    retval = err_ok;
    *found = 0;
  out:
    if (enumerate)
        udev_enumerate_unref(enumerate);
    if (udev)
        udev_unref(udev);
    return retval;
}

notetaker_err_t notetaker_open(struct notetaker **notetaker)
{
    int i;
    char iface0_path[PATH_MAX];
    char iface1_path[PATH_MAX];
    char *paths[NOTETAKER_IFACE_COUNT] = {iface0_path, iface1_path};
    for (i = 0; i < NOTETAKER_IFACE_COUNT; ++i) {
        notetaker_err_t err;
        char found;

        memset(paths[i], 0, PATH_MAX);

        err = nt_find_hidraw_devnode(&found, i, paths[i], PATH_MAX);
        switch (err) {
        case err_ok:
            break;
        default:
            return err;
        }

        if (!found)
            return err_nodev;
    }
    return notetaker_open_from_hidraw_paths(notetaker, paths);
}

notetaker_err_t notetaker_get_info(struct notetaker *notetaker,
                                   struct notetaker_info *info)
{
    notetaker_err_t err;
    uint8_t rpt[] = {NT_RPT_INFO};
    struct info_response response;

    err = nt_write_rpt(notetaker, rpt, sizeof(rpt));
    if (err)
        return err;

    err = nt_read_rpt(notetaker, &response, sizeof(struct info_response));
    if (err)
        return err;

    info->firmware_version = be16toh(response.firmware_version);
    info->analog_version = be16toh(response.analog_version);
    info->pad_version = be16toh(response.pad_version);
    info->mode = response.mode;

    return err_ok;
}

notetaker_err_t notetaker_delete_notes(struct notetaker *notetaker)
{
    notetaker_err_t err;
    uint8_t rpt[] = {NT_RPT_ERASE};

    err = nt_write_rpt(notetaker, rpt, sizeof(rpt));
    if (err)
        return err;
    return err_ok;
}

notetaker_err_t notetaker_wait_ready(struct notetaker *notetaker,
                                     const struct timeval *timeout,
                                     char *ready)
{
    uint8_t rpt[] = {NT_RPT_INFO};
    int fd = notetaker->fds[0];
    struct timeval now;
    struct timeval end_time;

    if (gettimeofday(&now, NULL) == -1)
        return err_sys;

    if (timeout != NULL)
        timeradd(&now, timeout, &end_time);

    while (timeout == NULL || timercmp(&now, &end_time, <=)) {
        fd_set readfds;
        struct timeval select_interval;
        int nfds;
        notetaker_err_t err;

        memset(&select_interval, 0, sizeof(struct timeval));
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        select_interval.tv_usec = 100000;

        err = nt_write_rpt(notetaker, rpt, sizeof(rpt));
        if (err)
            return err;

        nfds = select(fd + 1, &readfds, NULL, NULL, &select_interval);
        if (nfds == 0) {
            /* Not ready yet. */
            if (gettimeofday(&now, NULL) == -1)
                return err_sys;
            continue;
        } else if (nfds == -1) {
            return err_sys;
        } else {
            *ready = 1;
            return err_ok;
        }
    }
    *ready = 0;
    return err_ok;
}

/* int notetaker_upload_reject(struct notetaker *notetaker) */
/* { */
/*     uint8_t rpt[] = {NT_RPT_NACK}; */
/*     if (nt_write_rpt(notetaker, rpt, sizeof(rpt)) == -1) */
/*         return -1; */
/*     return 0; */
/* } */

/* int notetaker_upload_get_size(struct notetaker *notetaker) */
/* { */
/*     int i; */
/*     uint8_t rpt[] = {NT_RPT_UPLOAD}; */
/*     uint8_t response[NT_MAX_RESPONSE_SIZE]; */

/*     memset(response, 0, NT_MAX_RESPONSE_SIZE); */

/*     if (nt_chat(notetaker, rpt, sizeof(rpt), response, NT_MAX_RESPONSE_SIZE) == -1) */
/*         return -1; */

/*     for (i = 0; i < NT_MAX_RESPONSE_SIZE; ++i) { */
/*         printf("%d: %d\n", i, response[i]); */
/*     } */

/*     return notetaker_upload_reject(notetaker); */
/* } */
