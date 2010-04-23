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

#include "m210.h"

#define WAIT_INTERVAL 100000 /* Microseconds. */

static const struct hidraw_devinfo DEVINFO_M210 = {
    BUS_USB,
    0x0e20,
    0x0101,
};

struct m210 {
    int fds[M210_IFACE_COUNT];
};

/*
  Bytes:  0    1    2        3      4          rpt_size
  Values: 0x00 0x02 rpt_size rpt[0] rpt[1] ... rpt[rpt_size - 1]
*/
static m210_err_t m210_write_rpt(struct m210 *m210,
                                 const uint8_t *rpt, size_t rpt_size)
{
    m210_err_t err = err_sys;
    uint8_t *request;
    size_t request_size = rpt_size + 3;

    request = (uint8_t *) malloc(request_size);
    if (request == NULL)
        return err_sys;

    request[0] = 0x00;     /* Without this, response is not sent. Why?? */
    request[1] = 0x02;     /* report id */
    request[2] = rpt_size;

    /* Copy report paylod to the end of the request. */
    memcpy(request + 3, rpt, rpt_size);

    /* Send request to the interface 0. */
    if (write(m210->fds[0], request, request_size) == -1)
        goto err;

    err = err_ok;
  err:
    free(request);
    return err;
}

#define M210_RESP_SIZE 64
static m210_err_t m210_read_rpt(struct m210 *m210, void *response,
                                size_t response_size)
{
    uint8_t buf[M210_RESP_SIZE];
    memset(buf, 0, M210_RESP_SIZE);

    if (read(m210->fds[0], buf, M210_RESP_SIZE) == -1)
        return err_sys;

    memset(response, 0, response_size);
    if (response_size > M210_RESP_SIZE)
        memcpy(response, buf, M210_RESP_SIZE);
    else
        memcpy(response, buf, response_size);

    return err_ok;
}

static m210_err_t m210_find_hidraw_devnode(char *found, int iface,
                                           char *path, size_t path_size)
{
    int err = err_sys;
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
        int ifn;
        uint16_t vendor;
        uint16_t product;
        const char *syspath = udev_list_entry_get_name(list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, syspath);
        const char *devnode = udev_device_get_devnode(dev);
        struct udev_device *parent = udev_device_get_parent(dev);

        parent = udev_device_get_parent(parent); /* Second parent: usb */
        ifn = atoi(udev_device_get_sysattr_value(parent, "bInterfaceNumber"));
        parent = udev_device_get_parent(parent);
        vendor = strtol(udev_device_get_sysattr_value(parent, "idVendor"),
                        NULL, 16);
        product = strtol(udev_device_get_sysattr_value(parent, "idProduct"),
                         NULL, 16);
        if (vendor == DEVINFO_M210.vendor
            && product == DEVINFO_M210.product
            && iface == ifn) {
            err = err_ok;
            *found = 1;
            strncpy(path, devnode, path_size);
            udev_device_unref(dev);
            goto out;
        }
        list_entry = udev_list_entry_get_next(list_entry);
        udev_device_unref(dev);
    }
    err = err_ok;
    *found = 0;
  out:
    if (enumerate)
        udev_enumerate_unref(enumerate);
    if (udev)
        udev_unref(udev);
    return err;
}

/*
  Get info request:
  Bytes:  0
  Values: 0x95

  Info response:
  Bytes:  0    1    2    3   4   5   6   7   8   9    10
  Values: 0x80 0xa9 0x28 fvh fvl avh avl pvh pvl 0x0e mode

  fvh = Firmware version high
  fvl = Firmware version low
  avh = Analog version high
  avl = Analog version low
  pvh = Pad version high
  pvl = Pad version low
*/
m210_err_t m210_get_info(struct m210 *m210, struct m210_info *info)
{
    uint8_t rpt[] = {0x95};
    int fd = m210->fds[0];

    while (1) {
        fd_set readfds;
        struct timeval select_interval;
        int nfds;
        m210_err_t err;

        memset(&select_interval, 0, sizeof(struct timeval));
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        select_interval.tv_usec = WAIT_INTERVAL;

        err = m210_write_rpt(m210, rpt, sizeof(rpt));
        if (err)
            return err;

        nfds = select(fd + 1, &readfds, NULL, NULL, &select_interval);
        if (nfds == 0) {
            /* Not ready yet. */
            continue;
        } else if (nfds == -1) {
            return err_sys;
        } else {
            uint8_t resp[11];
            memset(&resp, 0, sizeof(resp));
            err = m210_read_rpt(m210, &resp, sizeof(resp));
            if (err)
                return err;

            /* Check that the received packet is correct. */
            if (resp[0] != 0x80
                || resp[1] != 0xa9
                || resp[2] != 0x28
                || resp[9] != 0x0e)
                return err_badmsg;

            if (info != NULL) {
                /* Fill in only if caller is interested in the info. */
                uint16_t firmware_version;
                uint16_t analog_version;
                uint16_t pad_version;

                memcpy(&firmware_version, resp + 3, 2);
                memcpy(&analog_version, resp + 5, 2);
                memcpy(&pad_version, resp + 7, 2);

                info->firmware_version = be16toh(firmware_version);
                info->analog_version = be16toh(analog_version);
                info->pad_version = be16toh(pad_version);
                info->mode = resp[10];
            }
            break;
        }
    }
    return err_ok;
}

static m210_err_t m210_reject(m210_t m210)
{
    uint8_t rpt[] = {0xb7};

    return m210_write_rpt(m210, rpt, sizeof(rpt));
}

#define M210_PACKET_SIZE M210_RESP_SIZE
/*
  Download request can be used for two purposes:

  1: Request just packet count.

  HOST       DEVICE
  =======================
  DOWNLOAD >
           < PACKET_COUNT
  REJECT   >

  2: Download packets.

  HOST        DEVICE
  ========================
  DOWNLOAD  >
            < PACKET_COUNT
  ACCEPT    >
            < PACKET #1
            < PACKET #2
            .
            .
            .
            < PACKET #N
  RESEND #X >
            < PACKET #X
  RESEND #Y >
            < PACKET #Y
  ACCEPT    >

  Begin upload request:
  Bytes:  0
  Values: 0xb5

  Packet count response:
  Bytes:  0    1    2    3    4    5          6         7    8
  Values: 0xaa 0xaa 0xaa 0xaa 0xaa count_high count_low 0x55 0x55
*/
static m210_err_t m210_download(m210_t m210, size_t *size)
{
    static const uint8_t sig1[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    static const uint8_t sig2[] = {0x55, 0x55};
    static const uint8_t rpt[] = {0xb5};
    uint8_t resp[9];
    m210_err_t err;
    uint16_t packet_count;

    memset(resp, 0, sizeof(resp));

    err = m210_write_rpt(m210, rpt, sizeof(rpt));
    if (err)
        return err;

    err = m210_read_rpt(m210, resp, sizeof(resp));
    if (err)
        return err;

    /* Check that the packet we received is correct. */
    if (memcmp(resp, sig1, sizeof(sig1))
        || memcmp(resp + sizeof(sig1) + 2, sig2, sizeof(sig2))) {
        return err_badmsg;
    }

    /* Packet count is reported in big-endian format. */
    memcpy(&packet_count, resp + sizeof(sig1), 2);
    packet_count = be16toh(packet_count);
    *size = packet_count * M210_PACKET_SIZE;

    return err_ok;
}

m210_err_t m210_open_from_hidraw_paths(struct m210 **m210, char **hidraw_paths)
{
    int err = err_sys;
    int i;
    int original_errno;

    *m210 = (struct m210 *) malloc(sizeof(struct m210));
    if (*m210 == NULL)
        return err_sys;

    for (i = 0; i < M210_IFACE_COUNT; ++i) {
        (*m210)->fds[i] = -1;
    }

    for (i = 0; i < M210_IFACE_COUNT; ++i) {
        int fd;
        const char *path = hidraw_paths[i];
        struct hidraw_devinfo devinfo;
        memset(&devinfo, 0, sizeof(struct hidraw_devinfo));

        if ((fd = open(path, O_RDWR)) == -1)
            goto err;

        if (ioctl(fd, HIDIOCGRAWINFO, &devinfo))
            goto err;

        if (memcmp(&devinfo, &DEVINFO_M210,
                   sizeof(struct hidraw_devinfo)) != 0) {
            err = err_baddev;
            goto err;
        }

        (*m210)->fds[i] = fd;
    }

    return err_ok;

  err:
    original_errno = errno;
    switch (m210_free(*m210)) {
    case err_sys:
        free(*m210);
        break;
    default:
        break;
    }
    errno = original_errno;
    return err;
}

m210_err_t m210_open(struct m210 **m210, char** hidraw_paths)
{
    if (hidraw_paths == NULL) {
        int i;
        char iface0_path[PATH_MAX];
        char iface1_path[PATH_MAX];
        char *paths[M210_IFACE_COUNT] = {iface0_path, iface1_path};
        for (i = 0; i < M210_IFACE_COUNT; ++i) {
            m210_err_t err;
            char found;

            memset(paths[i], 0, PATH_MAX);

            err = m210_find_hidraw_devnode(&found, i, paths[i], PATH_MAX);
            switch (err) {
            case err_ok:
                break;
            default:
                return err;
            }

            if (!found)
                return err_nodev;
        }
        return m210_open_from_hidraw_paths(m210, paths);
    }
    return m210_open_from_hidraw_paths(m210, hidraw_paths);
}

m210_err_t m210_free(struct m210 *m210)
{
    int i;

    for (i = 0; i < M210_IFACE_COUNT; ++i) {
        int fd = m210->fds[i];
        if (fd != -1) {
            if (close(fd) == -1)
                return err_sys;
        }
    }
    free(m210);
    return err_ok;
}

m210_err_t m210_delete_notes(struct m210 *m210)
{
    m210_err_t err;
    uint8_t rpt[] = {0xb0};

    err = m210_write_rpt(m210, rpt, sizeof(rpt));
    if (err)
        return err;

    return m210_get_info(m210, NULL);
}

m210_err_t m210_get_data_size(m210_t m210, size_t *size)
{
    m210_err_t err;

    err = m210_download(m210, size);
    if (err)
        return err;

    /* Reject download, we are only interested in data size. */
    err = m210_reject(m210);
    if (err)
        return err;

    return m210_get_info(m210, NULL);
}
