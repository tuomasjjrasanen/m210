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

#define M210_DEV_PACKET_SIZE 62

#define M210_DEV_USB_INTERFACE_COUNT 2

#define M210_DEV_MODE_INDICATOR_TABLET 0x01
#define M210_DEV_MODE_INDICATOR_MOUSE  0x02

struct m210_dev {
        int fds[M210_DEV_USB_INTERFACE_COUNT];
};

struct m210_dev_packet {
        uint16_t num;
        uint8_t data[M210_DEV_PACKET_SIZE];
} __attribute__((packed));

static struct hidraw_devinfo const DEVINFO_M210 = {
        BUS_USB,
        0x0e20,
        0x0101,
};

static enum m210_err
m210_dev_write(struct m210_dev const *const dev_ptr,
               uint8_t const *const bytes,
               size_t const bytes_size)
{
        enum m210_err result;
        uint8_t *request;
        size_t const request_size = bytes_size + 3;

        request = malloc(request_size);
        if (request == NULL) {
                result = M210_ERR_SYS;
                goto err;
        }

        request[0] = 0x00;     /* Without this, response is not sent. Why?? */
        request[1] = 0x02;     /* report id */
        request[2] = bytes_size;

        /* Copy report paylod to the end of the request. */
        memcpy(request + 3, bytes, bytes_size);

        /* Send request to the interface 0. */
        if (write(dev_ptr->fds[0], request, request_size) == -1) {
                result = M210_ERR_SYS;
                goto err;
        }

        result = M210_ERR_OK;
err:
        free(request);
        return result;
}

static enum m210_err
m210_dev_read(struct m210_dev const *const dev_ptr,
              int const interface,
              void *const response,
              size_t const response_size)
{
        enum m210_err result;
        fd_set readfds;
        int const fd = dev_ptr->fds[interface];
        static struct timeval select_interval;

        memset(&select_interval, 0, sizeof(struct timeval));
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        select_interval.tv_usec = M210_DEV_READ_INTERVAL;

        switch (select(fd + 1, &readfds, NULL, NULL, &select_interval)) {
        case 0:
                result = M210_ERR_DEV_TIMEOUT;
                goto err;
        case -1:
                result = M210_ERR_SYS;
                goto err;
        default:
                break;
        }

        if (read(fd, response, response_size) == -1) {
                result = M210_ERR_SYS;
                goto err;
        }

        result = M210_ERR_OK;
err:
        return result;
}

static enum m210_err
m210_dev_find_hidraw_devnode(int const iface,
                             char *const path_ptr,
                             size_t const path_size)
{
        enum m210_err result;
        struct udev_list_entry *list_entry = NULL;
        struct udev_enumerate *enumerate = NULL;
        struct udev *udev = NULL;

        udev = udev_new();
        if (udev == NULL) {
                goto err;
        }

        enumerate = udev_enumerate_new(udev);
        if (enumerate == NULL) {
                goto err;
        }

        if (udev_enumerate_add_match_subsystem(enumerate, "hidraw")) {
                goto err;
        }

        if (udev_enumerate_scan_devices(enumerate)) {
                goto err;
        }

        list_entry = udev_enumerate_get_list_entry(enumerate);

        while (list_entry != NULL) {
                int ifn;
                uint16_t vendor;
                uint16_t product;
                char const *syspath;
                struct udev_device *device;
                char const *devnode;
                struct udev_device *parent;

                syspath =  udev_list_entry_get_name(list_entry);
                device = udev_device_new_from_syspath(udev, syspath);
                devnode = udev_device_get_devnode(device);

                /* Second parent: usb */
                parent = udev_device_get_parent(device);
                parent = udev_device_get_parent(parent);

                ifn = atoi(udev_device_get_sysattr_value(parent,
                                                         "bInterfaceNumber"));
                parent = udev_device_get_parent(parent);
                vendor = strtol(udev_device_get_sysattr_value(parent,
                                                              "idVendor"),
                                NULL, 16);
                product = strtol(udev_device_get_sysattr_value(parent,
                                                               "idProduct"),
                                 NULL, 16);
                if (vendor == DEVINFO_M210.vendor
                    && product == DEVINFO_M210.product
                    && iface == ifn) {
                        result = M210_ERR_OK;
                        strncpy(path_ptr, devnode, path_size);
                        udev_device_unref(device);
                        goto err;
                }
                list_entry = udev_list_entry_get_next(list_entry);
                udev_device_unref(device);
        }
        result = M210_ERR_NO_DEV;
err:
        if (enumerate) {
                udev_enumerate_unref(enumerate);
        }

        if (udev) {
                udev_unref(udev);
        }

        return result;
}

/* static enum m210_err m210_wait_ready(struct m210 const *const m210) */
/* { */
/*     return m210_get_info(m210, NULL); */
/* } */

static enum m210_err
m210_dev_accept_download(struct m210_dev const *const dev_ptr)
{
        uint8_t const bytes[] = {0xb6};
        return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
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

static enum m210_err
m210_dev_reject_download(struct m210_dev const *const dev_ptr)
{
        uint8_t const bytes[] = {0xb7};
        return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

static enum m210_err
m210_dev_connect_hidraw(struct m210_dev *const dev_ptr,
                        char *const *const hidraw_path_ptrs)
{
        enum m210_err err = M210_ERR_OK;
        int fds[M210_DEV_USB_INTERFACE_COUNT];

        for (size_t i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
                int fd;
                struct hidraw_devinfo devinfo;

                fd = open(hidraw_path_ptrs[i], O_RDWR);
                if (fd == -1) {
                        err = M210_ERR_SYS;
                        goto exit;
                }

                if (ioctl(fd, HIDIOCGRAWINFO, &devinfo)) {
                        err = M210_ERR_SYS;
                        goto exit;
                }

                if (memcmp(&devinfo, &DEVINFO_M210,
                           sizeof(struct hidraw_devinfo)) != 0) {
                        err = M210_ERR_BAD_DEV;
                        goto exit;
                }

                fds[i] = fd;
        }

exit:
        if (err) {
                return err;
        }
        memcpy(dev_ptr->fds, fds, sizeof(fds));
        return M210_ERR_OK;
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
static enum m210_err
m210_dev_begin_download(struct m210_dev const *const dev_ptr,
                        uint16_t *const packet_count_ptr)
{
        static uint8_t const bytes[] = {0xb5};
        uint8_t response[9];
        enum m210_err result;

        memset(response, 0, sizeof(response));

        result = m210_dev_write(dev_ptr, bytes, sizeof(bytes));
        if (result) {
                goto err;
        }

        while (1) {
                result = m210_dev_read(dev_ptr, 0, response, sizeof(response));
                switch (result) {
                case M210_ERR_DEV_TIMEOUT:
                        /*
                          It seems, that a M210 device with zero notes
                          does not send any response.
                        */
                        *packet_count_ptr = 0;
                        result = M210_ERR_OK;
                        goto out;
                case M210_ERR_OK:
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

        result = M210_ERR_OK;
        goto out;
err:
        /* Try to leave the device as it was before the error. */
        m210_dev_reject_download(dev_ptr);
out:
        return result;
}

static enum m210_err
m210_dev_read_packet(struct m210_dev const *const dev_ptr,
                     struct m210_dev_packet *const packet_ptr)
{
        enum m210_err err = m210_dev_read(dev_ptr, 0, packet_ptr,
                                          sizeof(struct m210_dev_packet));
        if (!err) {
                packet_ptr->num = be16toh(packet_ptr->num);
        }
        return err;
}

enum m210_err
m210_dev_connect(struct m210_dev **const dev_ptr_ptr)
{
        struct m210_dev *dev_ptr = NULL;
        enum m210_err err = M210_ERR_OK;
        char iface0_path[PATH_MAX];
        char iface1_path[PATH_MAX];
        char *paths[M210_DEV_USB_INTERFACE_COUNT] = {iface0_path, iface1_path};

        dev_ptr = malloc(sizeof(struct m210_dev));
        if (!dev_ptr) {
                err = M210_ERR_SYS;
                goto exit;
        }

        for (size_t i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
                memset(paths[i], 0, PATH_MAX);
        
                err = m210_dev_find_hidraw_devnode(i, paths[i], PATH_MAX);
                if (err) {
                        goto exit;
                }
        }
        err = m210_dev_connect_hidraw(dev_ptr, paths);
exit:
        if (err) {
                free(dev_ptr);
                dev_ptr = NULL;
        }
        *dev_ptr_ptr = dev_ptr;
        return err;
}

enum m210_err
m210_dev_disconnect(struct m210_dev **const dev_ptr_ptr)
{
        struct m210_dev *const dev_ptr = *dev_ptr_ptr;
        enum m210_err err = M210_ERR_OK;

        if (!dev_ptr) {
                goto exit;
        }

        for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
                if (close(dev_ptr->fds[i]) == -1) {
                        err = M210_ERR_SYS;
                }
        }
        free(dev_ptr);
        *dev_ptr_ptr = NULL;
exit:
        return err;
}

/*
  Return the total size of notes in bytes. Theoretical maximum size
  is 4063232:

  * Packets are numbered with 16 bit integers.
  => Maximum number of packets: 2**16 = 65536

  * Each packet is 64 bytes wide, last 62 bytes represent bytes in
  memory. The first two bytes represent the packet sequence number.
  => Maximum number of bytes in memory: 2**16 * 62 = 4063232

  * A 32bit integer can address 2**32 different bytes which is way
  more than the maximum number of bytes in devices memory.

*/
static enum m210_err
m210_dev_get_notes_size(struct m210_dev const *const dev_ptr,
                        uint32_t *const size_ptr)
{
        uint16_t packet_count;
        enum m210_err err;

        err = m210_dev_begin_download(dev_ptr, &packet_count);
        if (err) {
                goto exit;
        }

        err = m210_dev_reject_download(dev_ptr);
exit:
        if (!err) {
                *size_ptr = packet_count * M210_DEV_PACKET_SIZE;
        }
        return err;
}

enum m210_err
m210_dev_get_info(struct m210_dev *const dev_ptr,
                  struct m210_dev_info *const info_ptr)
{
        enum m210_err result;
        static uint8_t const bytes[] = {0x95};
        uint8_t response[M210_DEV_RESPONSE_SIZE];
        uint32_t used_memory = 0;
  
        result = m210_dev_write(dev_ptr, bytes, sizeof(bytes));
        if (result) {
                goto err;
        }
    
        while (1) {
                memset(response, 0, sizeof(response));
                result = m210_dev_read(dev_ptr, 0, response, sizeof(response));
                if (result) {
                        goto err;
                }
        
                /* Check that the response is correct. */
                if (response[0] == 0x80
                    && response[1] == 0xa9
                    && response[2] == 0x28
                    && response[9] == 0x0e) {
                        break;
                }
        }

        result = m210_dev_get_notes_size(dev_ptr, &used_memory);
        if (result) {
                goto err;
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
        info_ptr->used_memory = used_memory;
        
        result = M210_ERR_OK;
err:
        return result;
}

enum m210_err
m210_dev_delete_notes(struct m210_dev *const dev_ptr)
{
        uint8_t const bytes[] = {0xb0};
        return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

static enum m210_err
m210_dev_download(struct m210_dev const *const dev_ptr,
                  uint16_t packet_count,
                  uint16_t *const lost_nums,
                  FILE *const stream_ptr)
{
        enum m210_err result;
        uint16_t lost_count = 0;

        for (int i = 0; i < packet_count; ++i) {
                struct m210_dev_packet packet;
                uint16_t const expected_packet_number = i + 1;

                result = m210_dev_read_packet(dev_ptr, &packet);
                if (result) {
                        goto err;
                }

                if (packet.num != expected_packet_number) {
                        lost_nums[lost_count++] = expected_packet_number;
                }

                if (!lost_count) {
                        if (fwrite(packet.data, sizeof(packet.data), 1,
                                   stream_ptr) != 1) {
                                result = M210_ERR_SYS;
                                goto err;
                        }
                }
        }

        while (lost_count > 0) {
                struct m210_dev_packet packet;

                uint8_t resend_request[] = {0xb7, 0x00};
                resend_request[1] = htobe16(lost_nums[0]);

                result = m210_dev_write(dev_ptr, resend_request,
                                        sizeof(resend_request));
                if (result) {
                        goto err;
                }

                result = m210_dev_read_packet(dev_ptr, &packet);
                if (result) {
                        goto err;
                }

                if (packet.num == lost_nums[0]) {
                        lost_nums[0] = lost_nums[--lost_count];
                        if (fwrite(packet.data, sizeof(packet.data), 1,
                                   stream_ptr) != 1) {
                                result = M210_ERR_SYS;
                                goto err;
                        }
                }
        }
        result = M210_ERR_OK;
err:
        return result;
}

enum m210_err
m210_dev_download_notes(struct m210_dev *const dev_ptr,
                        FILE *const stream_ptr)
{
        enum m210_err result;
        uint16_t *lost_nums = NULL;
        uint16_t packet_count;

        result = m210_dev_begin_download(dev_ptr, &packet_count);
        if (result) {
                goto err;
        }

        if (packet_count == 0) {
                result = m210_dev_reject_download(dev_ptr);
                goto err;
        }

        lost_nums = calloc(packet_count, sizeof(uint16_t));
        if (lost_nums == NULL) {
                int const original_errno = errno;
                m210_dev_reject_download(dev_ptr);
                errno = original_errno;
                result = M210_ERR_SYS;
                goto err;
        }

        result = m210_dev_accept_download(dev_ptr);
        if (result) {
                goto err;
        }

        result = m210_dev_download(dev_ptr, packet_count, lost_nums, stream_ptr);
        if (result) {
                goto err;
        }

        /*
          All packets have been received, time to thank the device for
          cooperation.
        */
        result = m210_dev_accept_download(dev_ptr);
err:
        free(lost_nums);
        return result;
}

enum m210_err
m210_dev_set_mode(struct m210_dev *const dev_ptr, uint8_t const mode)
{        
        uint8_t bytes[] = {0x80, 0xb5, M210_DEV_MODE_INDICATOR_MOUSE, mode};
        if (mode == M210_DEV_MODE_TABLET) {
                bytes[2] = M210_DEV_MODE_INDICATOR_TABLET;
        }
        return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

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
