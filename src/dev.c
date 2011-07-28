/*
  libm210 - C API for Pegasus Tablet Mobile NoteTaker (M210)
  Copyright © 2011 Tuomas Jorma Juhani Räsänen <tuomas.rasanen@tjjr.fi>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/hidraw.h>
#include <linux/input.h>  /* BUS_USB */
#include <linux/limits.h> /* PATH_MAX */

#include <libudev.h>

#include "dev.h"

#define M210_DEV_READ_INTERVAL 1000000 /* Microseconds. */
#define M210_DEV_RESPONSE_SIZE 64

#define M210_DEV_PACKET_DATA_LEN 62

struct m210_dev_packet {
    uint16_t num;
    uint8_t data[M210_DEV_PACKET_DATA_LEN];
} __attribute__((packed));

static struct hidraw_devinfo const DEVINFO_M210 = {
    BUS_USB,
    0x0e20,
    0x0101,
};

static enum m210_dev_err m210_dev_write(struct m210_dev const *const m210,
                                        uint8_t const *const command,
                                        size_t const command_size)
{
    enum m210_dev_err err;
    uint8_t *request;
    size_t const request_size = command_size + 3;

    request = (uint8_t *) malloc(request_size);
    if (request == NULL) {
        return M210_DEV_ERR_SYS;
    }

    request[0] = 0x00;     /* Without this, response is not sent. Why?? */
    request[1] = 0x02;     /* report id */
    request[2] = command_size;

    /* Copy report paylod to the end of the request. */
    memcpy(request + 3, command, command_size);

    /* Send request to the interface 0. */
    if (write(m210->fds[0], request, request_size) == -1) {
        err = M210_DEV_ERR_SYS;
        goto err;
    }

    err = M210_DEV_ERR_OK;
  err:
    free(request);
    return err;
}

static enum m210_dev_err m210_dev_read(struct m210_dev const *const m210,
                                       int const interface,
                                       void *const response,
                                       size_t const response_size)
{
    fd_set readfds;
    int const fd = m210->fds[interface];
    static struct timeval select_interval;

    memset(&select_interval, 0, sizeof(struct timeval));
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    select_interval.tv_usec = M210_DEV_READ_INTERVAL;

    switch (select(fd + 1, &readfds, NULL, NULL, &select_interval)) {
    case 0:
        return M210_DEV_ERR_TIMEOUT;
    case -1:
        return M210_DEV_ERR_SYS;
    default:
        break;
    }

    if (read(fd, response, response_size) == -1) {
        return M210_DEV_ERR_SYS;
    }

    return M210_DEV_ERR_OK;
}

static enum m210_dev_err m210_dev_find_hidraw_devnode(int const iface,
                                                      char *const path,
                                                      size_t const path_size)
{
    int err = M210_DEV_ERR_SYS;
    struct udev_list_entry *list_entry = NULL;
    struct udev_enumerate *enumerate = NULL;
    struct udev *udev = NULL;

    udev = udev_new();
    if (udev == NULL) {
        goto out;
    }

    enumerate = udev_enumerate_new(udev);
    if (enumerate == NULL) {
        goto out;
    }

    if (udev_enumerate_add_match_subsystem(enumerate, "hidraw")) {
        goto out;
    }

    if (udev_enumerate_scan_devices(enumerate)) {
        goto out;
    }

    list_entry = udev_enumerate_get_list_entry(enumerate);

    while (list_entry != NULL) {
        int ifn;
        uint16_t vendor;
        uint16_t product;
        char const *const syspath = udev_list_entry_get_name(list_entry);
        struct udev_device *const dev = udev_device_new_from_syspath(udev,
                                                                     syspath);
        char const *const devnode = udev_device_get_devnode(dev);
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
            err = M210_DEV_ERR_OK;
            strncpy(path, devnode, path_size);
            udev_device_unref(dev);
            goto out;
        }
        list_entry = udev_list_entry_get_next(list_entry);
        udev_device_unref(dev);
    }
    err = M210_DEV_ERR_NODEV;
  out:
    if (enumerate) {
        udev_enumerate_unref(enumerate);
    }

    if (udev) {
        udev_unref(udev);
    }

    return err;
}

/* static enum m210_err m210_wait_ready(struct m210 const *const m210) */
/* { */
/*     return m210_get_info(m210, NULL); */
/* } */

static enum m210_dev_err m210_dev_accept(struct m210_dev const *const dev_ptr)
{
    uint8_t const command[] = {0xb6};
    return m210_dev_write(dev_ptr, command, sizeof(command));
}

/* static enum m210_err m210_write_and_wait(struct m210 const *const m210, */
/*                                          uint8_t const *const rpt, */
/*                                          size_t const rpt_size) */
/* { */
/*     enum m210_err const err = m210_write(m210, rpt, rpt_size); */
/*     if (err) { */
/*         return err; */
/*     } */

/*     return m210_wait_ready(m210); */
/* } */

static enum m210_dev_err m210_dev_reject(struct m210_dev const *const dev_ptr)
{
    uint8_t const command[] = {0xb7};
    return m210_dev_write(dev_ptr, command, sizeof(command));
}

static enum m210_dev_err m210_dev_open_from_hidraw_paths(struct m210_dev *const dev_ptr,
                                                         char *const *const hidraw_paths)
{
    int err = M210_DEV_ERR_SYS;
    int original_errno;

    for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
        dev_ptr->fds[i] = -1;
    }

    for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
        int fd;
        char const *const path = hidraw_paths[i];
        struct hidraw_devinfo devinfo;
        memset(&devinfo, 0, sizeof(struct hidraw_devinfo));

        if ((fd = open(path, O_RDWR)) == -1) {
            goto err;
        }

        if (ioctl(fd, HIDIOCGRAWINFO, &devinfo)) {
            goto err;
        }

        if (memcmp(&devinfo, &DEVINFO_M210,
                   sizeof(struct hidraw_devinfo)) != 0) {
            err = M210_DEV_ERR_BADDEV;
            goto err;
        }

        dev_ptr->fds[i] = fd;
    }

    return M210_DEV_ERR_OK;

  err:
    original_errno = errno;
    m210_dev_disconnect(dev_ptr);
    errno = original_errno;
    return err;
}

/*
  Download request can be used for two purposes:

  1: Request just packet count.

  HOST             DEVICE
  =============================
  GET_PACKET_COUNT >
  < PACKET_COUNT
  REJECT           >

  2: Download packets.

  HOST              DEVICE
  ==============================
  GET_PACKET_COUNT  >
  < PACKET_COUNT
  ACCEPT            >
  < PACKET #1
  < PACKET #2
  .
  .
  .
  < PACKET #N
  RESEND #X         >
  < PACKET #X
  RESEND #Y         >
  < PACKET #Y
  ACCEPT            >

*/
static enum m210_dev_err m210_dev_upload_begin(struct m210_dev const *const dev_ptr,
                                               uint16_t *const packet_count_ptr)
{
    static uint8_t const command[] = {0xb5};
    uint8_t response[9];
    enum m210_dev_err err;

    memset(response, 0, sizeof(response));

    err = m210_dev_write(dev_ptr, command, sizeof(command));
    if (err) {
        return err;
    }

    while (1) {
        err = m210_dev_read(dev_ptr, 0, response, sizeof(response));
        switch (err) {
        case M210_DEV_ERR_TIMEOUT:
            /*
              It seems, that a M210 device with zero notes does not send
              any response.
            */
            *packet_count_ptr = 0;
            return M210_DEV_ERR_OK;
        case M210_DEV_ERR_OK:
            break;
        default:
            goto err;
        }

        /* Check that the response is correct. */
        if (response[0] == 0xaa
            && response[1] == 0xaa
            && response[2] == 0xaa
            && response[3] == 0xaa
            && response[4] == 0xaa
            && response[7] == 0x55
            && response[8] == 0x55) {
            break;
        }

    }

    uint16_t packet_count;

    memcpy(&packet_count, response + 5, 2);
    *packet_count_ptr = be16toh(packet_count);

    return M210_DEV_ERR_OK;

  err:
    /* Try to leave the device as it was before the error. */
    m210_dev_reject(dev_ptr);
    return err;
}

static enum m210_dev_err m210_dev_read_note_data_packet(struct m210_dev const *const dev_ptr,
                                                        struct m210_dev_packet *const packet_ptr)
{
    enum m210_dev_err err;

    err = m210_dev_read(dev_ptr, 0, packet_ptr, sizeof(struct m210_dev_packet));
    if (err) {
        return err;
    }

    packet_ptr->num = be16toh(packet_ptr->num);

    return M210_DEV_ERR_OK;
}

enum m210_dev_err m210_dev_connect(struct m210_dev *const dev_ptr)
{
    char iface0_path[PATH_MAX];
    char iface1_path[PATH_MAX];
    char *paths[M210_DEV_USB_INTERFACE_COUNT] = {iface0_path, iface1_path};
    for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
        enum m210_dev_err err = M210_DEV_ERR_SYS;
        
        memset(paths[i], 0, PATH_MAX);
        
        err = m210_dev_find_hidraw_devnode(i, paths[i], PATH_MAX);
        switch (err) {
        case M210_DEV_ERR_OK:
            break;
        default:
            return err;
        }
    }
    return m210_dev_open_from_hidraw_paths(dev_ptr, paths);
}

enum m210_dev_err m210_dev_disconnect(struct m210_dev *const dev_ptr)
{
    for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
        if (dev_ptr->fds[i] != -1) {
            if (close(dev_ptr->fds[i]) == -1) {
                return M210_DEV_ERR_SYS;
            }
            dev_ptr->fds[i] = -1;
        }
    }
    return M210_DEV_ERR_OK;
}

enum m210_dev_err m210_dev_get_info(struct m210_dev const *const dev_ptr,
                                    struct m210_dev_info *const info_ptr)
{
    enum m210_dev_err err;
    static uint8_t const command[] = {0x95};
    uint8_t response[M210_DEV_RESPONSE_SIZE];
  
    err = m210_dev_write(dev_ptr, command, sizeof(command));
    if (err) {
        return err;
    }
    
    while (1) {
        memset(response, 0, sizeof(response));
        err = m210_dev_read(dev_ptr, 0, response, sizeof(response));
        if (err) {
            return err;
        }
        
        /* Check that the response is correct. */
        if (response[0] == 0x80
            && response[1] == 0xa9
            && response[2] == 0x28
            && response[9] == 0x0e) {
            break;
        }
    }

    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
   
    memcpy(&firmware_version, response + 3, 2);
    memcpy(&analog_version, response + 5, 2);
    memcpy(&pad_version, response + 7, 2);
    
    info_ptr->firmware_version = be16toh(firmware_version);
    info_ptr->analog_version = be16toh(analog_version);
    info_ptr->pad_version = be16toh(pad_version);
    info_ptr->mode = response[10];

    return M210_DEV_ERR_OK;
}

enum m210_dev_err m210_dev_get_notes_size(struct m210_dev const *const dev_ptr,
                                          uint32_t *const size_ptr)
{
    enum m210_dev_err err;
    uint16_t packet_count;

    err = m210_dev_upload_begin(dev_ptr, &packet_count);
    if (err) {
        return err;
    }

    *size_ptr = packet_count * M210_DEV_PACKET_DATA_LEN;

    return m210_dev_reject(dev_ptr);
}

/* enum m210_err m210_delete_notes(struct m210 const *const m210) */
/* { */
/*     uint8_t const rpt[] = {0xb0}; */
/*     return m210_write_and_wait(m210, rpt, sizeof(rpt)); */
/* } */

enum m210_dev_err m210_dev_download_notes(struct m210_dev const *const dev_ptr,
                                          FILE *const file_ptr)
{
    enum m210_dev_err err;
    uint16_t *lost_packet_nums;
    uint16_t lost_packet_num_count = 0;
    uint16_t packet_count;

    err = m210_dev_upload_begin(dev_ptr, &packet_count);
    if (err) {
        return err;
    }

    if (packet_count == 0) {
        return m210_dev_reject(dev_ptr);
    }

    lost_packet_nums = (uint16_t *)calloc(packet_count, sizeof(uint16_t));
    if (lost_packet_nums == NULL) {
        int const original_errno = errno;
        m210_dev_reject(dev_ptr);
        errno = original_errno;
        return M210_DEV_ERR_SYS;
    }

    err = m210_dev_accept(dev_ptr);
    if (err) {
        goto err;
    }

    for (int i = 0; i < packet_count; ++i) {
        struct m210_dev_packet packet;
        uint16_t const expected_packet_number = i + 1;

        err = m210_dev_read_note_data_packet(dev_ptr, &packet);
        if (err) {
            goto err;
        }

        if (packet.num != expected_packet_number) {
            lost_packet_nums[lost_packet_num_count++] = expected_packet_number;
        }

        if (!lost_packet_num_count) {
            if (fwrite(packet.data, sizeof(packet.data), 1, file_ptr) != 1) {
                err = M210_DEV_ERR_SYS;
                goto err;
            }
        }
    }

    while (lost_packet_num_count > 0) {
        struct m210_dev_packet packet;

        uint8_t resend_request[] = {0xb7, 0x00};
        resend_request[1] = htobe16(lost_packet_nums[0]);

        err = m210_dev_write(dev_ptr, resend_request, sizeof(resend_request));
        if (err) {
            goto err;
        }

        err = m210_dev_read_note_data_packet(dev_ptr, &packet);
        if (err) {
            if (err == M210_DEV_ERR_TIMEOUT) {
                /*
                  Request resend as many times as needed to get the
                  expected packet. The M210 device has made a promise
                  and we blindly believe that it's going to keep it.
                */
                continue;
            } else {
                goto err;
            }
        }

        if (packet.num == lost_packet_nums[0]) {
            lost_packet_nums[0] = lost_packet_nums[--lost_packet_num_count];
            if (fwrite(packet.data, sizeof(packet.data), 1, file_ptr) != 1) {
                err = M210_DEV_ERR_SYS;
                goto err;
            }
        }
    }

    /*
      All packets have been received, time to thank the device for
      cooperation.
    */
    err = m210_dev_accept(dev_ptr);

  err:
    free(lost_packet_nums);
    return err;
}

char const *m210_dev_strerror(enum m210_dev_err const err)
{
    static char const *const err_strs[] = {
        "",
        "system call failed",
        "unknown device",
        "device not found",
        "unexpected response",
        "response waiting timeouted"
    };
    return err_strs[err];
}

extern char *program_invocation_name;

int m210_dev_perror(enum m210_dev_err const err, char const *const s)
{
    int const original_errno = errno;
    char const *const m210_errstr = m210_dev_strerror(err);
    /* +2 == a colon and a blank */
    size_t const progname_len = strlen(program_invocation_name) + 2;
    /* +2 == a colon and a blank */
    size_t const s_len = s == NULL ? 0 : strlen(s) + 2;
    size_t const m210_errstr_len = strlen(m210_errstr);

    /* +1 == a terminating null byte. */
    size_t const total_len = progname_len + s_len + m210_errstr_len + 1;
    char *errstr;

    errstr = (char *)calloc(total_len, sizeof(char));
    if (errstr == NULL) {
        return -1;
    }

    if (s == NULL) {
        snprintf(errstr, total_len, "%s: %s",
                 program_invocation_name, m210_errstr);
    } else {
        snprintf(errstr, total_len, "%s: %s: %s",
                 program_invocation_name, s, m210_errstr);
    }

    if (err == M210_DEV_ERR_SYS) {
        errno = original_errno;
        perror(errstr);
    } else {
        fprintf(stderr, "%s\n", errstr);
    }

    free(errstr);
    return 0;
}

/* enum m210_err m210_set_mode(struct m210 const *const m210, */
/*                             enum m210_mode_indicator const mode_indicator, */
/*                             enum m210_mode const mode) */
/* { */
/*     uint8_t rpt[] = {0x80, 0xb5, 0x00, 0x00}; */
/*     rpt[2] = mode_indicator; */
/*     rpt[3] = mode; */
/*     return m210_write_and_wait(m210, rpt, sizeof(rpt)); */
/* } */

/* enum m210_err m210_config_tablet_mode(struct m210 const *const m210, */
/*                                       enum m210_area_size const area_size, */
/*                                       enum m210_orientation const orientation) */
/* { */
/*     /\* Area size byte in M210-protocol is counter-intuitive: 0x00 */
/*        means the largest, 0x09 the smallest. However, in our API, area */
/*        sizes are handled intuitively and therefore needs to be changed */
/*        here. */
/*      *\/ */

/*     uint8_t rpt[] = {0x80, 0xb6, 0x00, 0x00}; */
/*     rpt[2] = abs(area_size - 0x09); */
/*     rpt[3] = orientation; */
/*     return m210_write_and_wait(m210, rpt, sizeof(rpt)); */
/* } */

/* enum m210_err m210_fwrite_tablet_data(struct m210 const *const m210, FILE *const stream) */
/* { */
/*     enum m210_err err; */
/*     uint8_t rpt[M210_RESPONSE_SIZE_IFACE1]; */
/*     uint8_t const sig[] = {0x08}; */

/*     memset(&rpt, 0, sizeof(rpt)); */

/*     while (1) { */
/*         err = m210_read(m210, 1, &rpt, sizeof(rpt)); */
/*         if (err) { */
/*             return err; */
/*         } */
/*         if (memcmp(rpt, sig, sizeof(sig)) != 0) { */
/*             return M210_ERR_BADMSG; */
/*         } */
/*         switch (rpt[5]) { */
/*         case M210_TABLET_PEN_RELEASED: */
/*         case M210_TABLET_PEN_PRESSED: */
/*             break; */
/*         default: */
/*             return M210_ERR_BADMSG; */
/*         } */
/*         if (fwrite(rpt+sizeof(sig), sizeof(rpt)-sizeof(sig), 1, stream) != 1) { */
/*             return M210_ERR_SYS; */
/*         } */
/*         if (fflush(stream)) { */
/*             return M210_ERR_SYS; */
/*         } */
/*     } */
/*     return M210_ERR_OK; */
/* } */

/* enum m210_err m210_fwrite_mouse_data(struct m210 const *const m210, FILE *const stream) */
/* { */
/*     enum m210_err err; */
/*     uint8_t rpt[M210_RESPONSE_SIZE_IFACE0]; */

/*     memset(&rpt, 0, sizeof(rpt)); */

/*     while (1) { */
/*         err = m210_read(m210, 0, &rpt, sizeof(rpt)); */
/*         if (err) { */
/*             return err; */
/*         } */
/*         for (size_t i = 0; i < sizeof(rpt); i = i + 6) { */
/*             if (rpt[i] == 0x00) { */
/*                 break; /\* No more mouse data in this report. *\/ */
/*             } */
/*             switch (rpt[i]) { */
/*             case M210_MOUSE_BATTERY_UNKNOWN: */
/*             case M210_MOUSE_BATTERY_LOW: */
/*             case M210_MOUSE_BATTERY_HIGH: */
/*                 break; */
/*             default: */
/*                 /\* Unexpected battery state. *\/ */
/*                 return M210_ERR_BADMSG; */
/*             } */
/*             switch (rpt[i+1]) { */
/*             case M210_MOUSE_PEN_OUT_OF_RANGE: */
/*             case M210_MOUSE_PEN_HOVERING: */
/*             case M210_MOUSE_PEN_TIP_PRESSED: */
/*             case M210_MOUSE_PEN_SWITCH_PRESSED: */
/*             case M210_MOUSE_PEN_BOTH_PRESSED: */
/*                 break; */
/*             default: */
/*                 /\* Unexpected pen state. *\/ */
/*                 return M210_ERR_BADMSG; */
/*             } */
/*             if (fwrite(rpt+i, 6, 1, stream) != 1) { */
/*                 return M210_ERR_SYS; */
/*             } */
/*             if (fflush(stream)) { */
/*                 return M210_ERR_SYS; */
/*             } */
/*         } */
/*     } */
/*     return M210_ERR_OK; */
/* } */
