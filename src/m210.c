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

#include "m210.h"

#define M210_READ_INTERVAL 1000000 /* Microseconds. */

#define M210_PACKET_DATA_LEN 62

struct m210_packet {
    uint16_t num;
    uint8_t data[M210_PACKET_DATA_LEN];
} __attribute__((packed));

static struct hidraw_devinfo const DEVINFO_M210 = {
    BUS_USB,
    0x0e20,
    0x0101,
};

/*
  Bytes:  0    1    2        3      4          rpt_size
  Values: 0x00 0x02 rpt_size rpt[0] rpt[1] ... rpt[rpt_size - 1]
*/
static enum m210_err m210_write(struct m210 const *const m210,
                                uint8_t const *const rpt, size_t const rpt_size)
{
    enum m210_err err;
    uint8_t *request;
    size_t const request_size = rpt_size + 3;

    request = (uint8_t *) malloc(request_size);
    if (request == NULL)
        return M210_ERR_SYS;

    request[0] = 0x00;     /* Without this, response is not sent. Why?? */
    request[1] = 0x02;     /* report id */
    request[2] = rpt_size;

    /* Copy report paylod to the end of the request. */
    memcpy(request + 3, rpt, rpt_size);

    /* Send request to the interface 0. */
    if (write(m210->fds[0], request, request_size) == -1) {
        err = M210_ERR_SYS;
        goto err;
    }

    err = M210_ERR_OK;
  err:
    free(request);
    return err;
}

#define M210_RESPONSE_SIZE_IFACE0 64
#define M210_RESPONSE_SIZE_IFACE1 9
static size_t const m210_interface_response_sizes[M210_USB_INTERFACE_COUNT] = {
    M210_RESPONSE_SIZE_IFACE0, M210_RESPONSE_SIZE_IFACE1};

static enum m210_err m210_read(struct m210 const *const m210, int const interface,
                               void *const response, size_t const response_size)
{
    fd_set readfds;
    int const fd = m210->fds[interface];
    static struct timeval select_interval;
    size_t const max_response_size = m210_interface_response_sizes[interface];
    uint8_t *buf;
    enum m210_err err;

    buf = (uint8_t *) malloc(max_response_size * sizeof(uint8_t));
    if (buf == NULL)
        return M210_ERR_SYS;

  again:
    memset(buf, 0, max_response_size);
    memset(&select_interval, 0, sizeof(struct timeval));
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    select_interval.tv_usec = M210_READ_INTERVAL;

    switch (select(fd + 1, &readfds, NULL, NULL, &select_interval)) {
    case 0:
        err = M210_ERR_TIMEOUT;
        goto err;
    case -1:
        err = M210_ERR_SYS;
        goto err;
    default:
        break;
    }

    if (read(fd, buf, max_response_size) == -1) {
        err = M210_ERR_SYS;
        goto err;
    }

    /**
       Ignore mode button data. Otherwise it might mess up
       then communication sequence.

       TODO: Better communication handling probably based on generic
       events and their handlers.

       Mode button:
       \verbatim
       +-----+-----------+-+-+-+-+-+-+-+-+
       |Byte#|Description|7|6|5|4|3|2|1|0|
       +-----+-----------+-+-+-+-+-+-+-+-+
       |  1  |0x80       |1|0|0|0|0|0|0|0|
       +-----+-----------+-+-+-+-+-+-+-+-+
       |  2  |0xB5       |1|0|1|1|0|1|0|1|
       +-----+-----------+-+-+-+-+-+-+-+-+
       \endverbatim
    */
    if (interface == 0) {
        uint8_t mode_button_rpt[M210_RESPONSE_SIZE_IFACE0];
        memset(mode_button_rpt, 0, M210_RESPONSE_SIZE_IFACE0);
        mode_button_rpt[0] = 0x80;
        mode_button_rpt[1] = 0xb5;
        if (memcmp(mode_button_rpt, buf, M210_RESPONSE_SIZE_IFACE0) == 0) {
            goto again;
        }
    }

    memset(response, 0, response_size);
    if (response_size > max_response_size)
        memcpy(response, buf, max_response_size);
    else
        memcpy(response, buf, response_size);

    err = M210_ERR_OK;
  err:
    free(buf);
    return err;
}

static enum m210_err m210_find_hidraw_devnode(uint8_t *const found, int const iface,
                                              char *const path, size_t const path_size)
{
    int err = M210_ERR_SYS;
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
        char const *const syspath = udev_list_entry_get_name(list_entry);
        struct udev_device *const dev = udev_device_new_from_syspath(udev, syspath);
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
            err = M210_ERR_OK;
            *found = 1;
            strncpy(path, devnode, path_size);
            udev_device_unref(dev);
            goto out;
        }
        list_entry = udev_list_entry_get_next(list_entry);
        udev_device_unref(dev);
    }
    err = M210_ERR_OK;
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
enum m210_err m210_get_info(struct m210 const *const m210, struct m210_info *const info)
{
    uint8_t const rpt[] = {0x95};
    uint8_t const resp[11];

    while (1) {
        enum m210_err err = M210_ERR_SYS;

        memset(&resp, 0, sizeof(resp));

        err = m210_write(m210, rpt, sizeof(rpt));
        if (err)
            return err;

        err = m210_read(m210, 0, &resp, sizeof(resp));
        if (err == M210_ERR_TIMEOUT)
            continue;
        else if (err == M210_ERR_OK)
            break;
        else
            return err;
    }

    /* Check that the received packet is correct. */
    if (resp[0] != 0x80
        || resp[1] != 0xa9
        || resp[2] != 0x28
        || resp[9] != 0x0e)
        return M210_ERR_BADMSG;

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

    return M210_ERR_OK;
}

static enum m210_err m210_wait_ready(struct m210 const *const m210)
{
    return m210_get_info(m210, NULL);
}

static enum m210_err m210_accept(struct m210 const *const m210)
{
    uint8_t const rpt[] = {0xb6};

    return m210_write(m210, rpt, sizeof(rpt));
}

static enum m210_err m210_write_and_wait(struct m210 const *const m210,
                                         uint8_t const *const rpt,
                                         size_t const rpt_size)
{
    enum m210_err const err = m210_write(m210, rpt, rpt_size);
    if (err)
        return err;

    return m210_wait_ready(m210);
}

static enum m210_err m210_reject_and_wait(struct m210 const *const m210)
{
    uint8_t const rpt[] = {0xb7};
    return m210_write_and_wait(m210, rpt, sizeof(rpt));
}

static enum m210_err m210_open_from_hidraw_paths(struct m210 *const m210, char *const *const hidraw_paths)
{
    int err = M210_ERR_SYS;
    int i;
    int original_errno;

    for (i = 0; i < M210_USB_INTERFACE_COUNT; ++i) {
        m210->fds[i] = -1;
    }

    for (i = 0; i < M210_USB_INTERFACE_COUNT; ++i) {
        int fd;
        char const *const path = hidraw_paths[i];
        struct hidraw_devinfo devinfo;
        memset(&devinfo, 0, sizeof(struct hidraw_devinfo));

        if ((fd = open(path, O_RDWR)) == -1)
            goto err;

        if (ioctl(fd, HIDIOCGRAWINFO, &devinfo))
            goto err;

        if (memcmp(&devinfo, &DEVINFO_M210,
                   sizeof(struct hidraw_devinfo)) != 0) {
            err = M210_ERR_BADDEV;
            goto err;
        }

        m210->fds[i] = fd;
    }

    return M210_ERR_OK;

  err:
    original_errno = errno;
    m210_close(m210);
    errno = original_errno;
    return err;
}

enum m210_err m210_open(struct m210 *const m210, char **const hidraw_paths)
{
    if (hidraw_paths == NULL) {
        int i;
        char iface0_path[PATH_MAX];
        char iface1_path[PATH_MAX];
        char *paths[M210_USB_INTERFACE_COUNT] = {NULL, NULL};
        paths[0] = iface0_path;
        paths[1] = iface1_path;
        for (i = 0; i < M210_USB_INTERFACE_COUNT; ++i) {
            enum m210_err err = M210_ERR_SYS;
            uint8_t found = 0;

            memset(paths[i], 0, PATH_MAX);

            err = m210_find_hidraw_devnode(&found, i, paths[i], PATH_MAX);
            switch (err) {
            case M210_ERR_OK:
                break;
            default:
                return err;
            }

            if (!found)
                return M210_ERR_NODEV;
        }
        return m210_open_from_hidraw_paths(m210, paths);
    }
    return m210_open_from_hidraw_paths(m210, hidraw_paths);
}

enum m210_err m210_close(struct m210 *const m210)
{
    int i;

    for (i = 0; i < M210_USB_INTERFACE_COUNT; ++i) {
        if (m210->fds[i] != -1) {
            if (close(m210->fds[i]) == -1)
                return M210_ERR_SYS;
            m210->fds[i] = -1;
        }
    }
    return M210_ERR_OK;
}

enum m210_err m210_delete_notes(struct m210 const *const m210)
{
    uint8_t const rpt[] = {0xb0};
    return m210_write_and_wait(m210, rpt, sizeof(rpt));
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

  Packet count request:
  Bytes:  0
  Values: 0xb5

  Packet count response:
  Bytes:  0    1    2    3    4    5          6         7    8
  Values: 0xaa 0xaa 0xaa 0xaa 0xaa count_high count_low 0x55 0x55
*/
static enum m210_err m210_upload_begin(struct m210 const *const m210,
                                       uint16_t *const packetc_ptr)
{
    static uint8_t const sig1[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    static uint8_t const sig2[] = {0x55, 0x55};
    static uint8_t const rpt[] = {0xb5};
    uint8_t resp[9];
    enum m210_err err;

    memset(resp, 0, sizeof(resp));

    err = m210_write(m210, rpt, sizeof(rpt));
    if (err)
        return err;

    err = m210_read(m210, 0, resp, sizeof(resp));
    switch (err) {
    case M210_ERR_TIMEOUT:
        /*
          It seems, that a M210 device with zero notes does not send
          any response.
        */
        *packetc_ptr = 0;
        return M210_ERR_OK;
    case M210_ERR_OK:
        break;
    default:
        goto err;
    }

    /* Check that the packet we received is correct. */
    if (memcmp(resp, sig1, sizeof(sig1))
        || memcmp(resp + sizeof(sig1) + 2, sig2, sizeof(sig2))) {
        err = M210_ERR_BADMSG;
        goto err;
    }

    /* Packet count is reported in big-endian format. */
    memcpy(packetc_ptr, resp + sizeof(sig1), 2);
    *packetc_ptr = be16toh(*packetc_ptr);

    return M210_ERR_OK;

  err:
    /* Try to leave the device as it was before the error. */
    m210_reject_and_wait(m210);
    return err;
}

/*
  Data packet:
  +-----+------------+-+-+-+-+-+-+-+-+
  |Byte#|Description |7|6|5|4|3|2|1|0|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  1  |Packet# HIGH|N|N|N|N|N|N|N|N|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  2  |Packet# LOW |n|n|n|n|n|n|n|n|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  3  |Data        |x|x|x|x|x|x|x|x|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  .  |Data        |x|x|x|x|x|x|x|x|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  .  |Data        |x|x|x|x|x|x|x|x|
  +-----+------------+-+-+-+-+-+-+-+-+
  |  64 |Data        |x|x|x|x|x|x|x|x|
  +-----+------------+-+-+-+-+-+-+-+-+

*/
static enum m210_err m210_read_note_data_packet(struct m210 const *const m210,
                                                struct m210_packet *const packet)
{
    enum m210_err err;

    err = m210_read(m210, 0, packet, sizeof(struct m210_packet));
    if (err)
        return err;

    packet->num = be16toh(packet->num);

    return M210_ERR_OK;
}

enum m210_err m210_get_notes_size(struct m210 const *const m210, uint32_t *const size)
{
    enum m210_err err;
    uint16_t packet_count;

    err = m210_upload_begin(m210, &packet_count);
    if (err)
        return err;

    *size = packet_count * M210_PACKET_DATA_LEN;

    return m210_reject_and_wait(m210);
}

enum m210_err m210_fwrite_note_data(struct m210 const *const m210, FILE *const f)
{
    int i;
    enum m210_err err;
    uint16_t *lost_packet_numv;
    uint16_t lost_packet_numc = 0;
    int original_errno;
    uint16_t packetc;

    err = m210_upload_begin(m210, &packetc);
    if (err)
        return err;

    if (packetc == 0)
        return m210_reject_and_wait(m210);

    lost_packet_numv = (uint16_t *)calloc(packetc, sizeof(uint16_t));
    if (lost_packet_numv == NULL) {
        original_errno = errno;
        m210_reject_and_wait(m210);
        errno = original_errno;
        return M210_ERR_SYS;
    }

    err = m210_accept(m210);
    if (err)
        goto err;

    for (i = 0; i < packetc; ++i) {
        struct m210_packet packet;
        uint16_t const expected_packet_number = i + 1;

        err = m210_read_note_data_packet(m210, &packet);
        if (err) {
            if (err == M210_ERR_TIMEOUT) {
                /*
                  Timeout because there is not any packets left to
                  read. However, the M210 has promised to send more,
                  so we mark all the rest packet numbers as lost and
                  proceed with resending.
                */
                int j;
                for (j = i; j < packetc; ++j) {
                    uint16_t const lost_packet_num = j + 1;
                    lost_packet_numv[lost_packet_numc++] = lost_packet_num;
                }
                err = M210_ERR_OK; /* This error has been handled. */
                goto resend;
            } else {
                goto err;
            }
        }

        if (packet.num != expected_packet_number)
            lost_packet_numv[lost_packet_numc++] = expected_packet_number;

        if (!lost_packet_numc) {
            if (fwrite(packet.data, sizeof(packet.data), 1, f) != 1) {
                err = M210_ERR_SYS;
                goto err;
            }
        }
    }

  resend:
    while (lost_packet_numc > 0) {
        struct m210_packet packet;

        /*
          Resend packet:
          +-----+------------+-+-+-+-+-+-+-+-+
          |Byte#|Description |7|6|5|4|3|2|1|0|
          +-----+------------+-+-+-+-+-+-+-+-+
          |  1  |NACK        |1|0|1|1|0|1|1|1|
          +-----+------------+-+-+-+-+-+-+-+-+
          |  2  |Packet# HIGH|N|N|N|N|N|N|N|N|
          +-----+------------+-+-+-+-+-+-+-+-+
          |  3  |Packet# LOW |n|n|n|n|n|n|n|n|
          +-----+------------+-+-+-+-+-+-+-+-+
        */
        uint8_t resend_request[] = {0xb7, 0x00};
        resend_request[1] = htobe16(lost_packet_numv[0]);

        err = m210_write(m210, resend_request, sizeof(resend_request));
        if (err)
            goto err;

        err = m210_read_note_data_packet(m210, &packet);
        if (err) {
            if (err == M210_ERR_TIMEOUT)
                /*
                  Request resend as many times as needed to get the
                  expected packet. The M210 device has made a promise
                  and we blindly believe that it's going to keep it.
                */
                continue;
            else
                goto err;
        }

        if (packet.num == lost_packet_numv[0]) {
            lost_packet_numv[0] = lost_packet_numv[--lost_packet_numc];
            if (fwrite(packet.data, sizeof(packet.data), 1, f) != 1) {
                err = M210_ERR_SYS;
                goto err;
            }
        }
    }

    /*
      All packets have been received, time to thank the device for
      cooperation.
    */
    err = m210_accept(m210);

  err:
    free(lost_packet_numv);
    return m210_wait_ready(m210);
}

inline uint32_t m210_note_header_next_header_pos(struct m210_note_header const *const header)
{
    return le32toh(header->next_header_pos[0] + header->next_header_pos[1] * 0x100
                   + header->next_header_pos[2] * 0x10000);
}

char const *m210_err_str(enum m210_err const err)
{
    static char const *const m210_errlist[] = {
        "no error",
        "syscall error",
        "device is not m210",
        "m210 not found",
        "unexpected response",
        "request timeouted"
    };
    return m210_errlist[err];
}

extern char *program_invocation_name;

int m210_err_printf(enum m210_err const err, char const *const s)
{
    int const original_errno = errno;
    char const *const m210_errstr = m210_err_str(err);
    /* +2 == a colon and a blank */
    size_t const progname_len = strlen(program_invocation_name) + 2;
    /* +2 == a colon and a blank */
    size_t const s_len = s == NULL ? 0 : strlen(s) + 2;
    size_t const m210_errstr_len = strlen(m210_errstr);

    /* +1 == a terminating null byte. */
    size_t const total_len = progname_len + s_len + m210_errstr_len + 1;
    char *errstr;

    errstr = (char *)calloc(total_len, sizeof(char));
    if (errstr == NULL)
        return -1;

    if (s == NULL)
        snprintf(errstr, total_len, "%s: %s", program_invocation_name, m210_errstr);
    else
        snprintf(errstr, total_len, "%s: %s: %s", program_invocation_name, s, m210_errstr);

    if (err == M210_ERR_SYS) {
        errno = original_errno;
        perror(errstr);
    } else {
        fprintf(stderr, "%s\n", errstr);
    }

    free(errstr);
    return 0;
}

inline int m210_note_data_is_pen_up(struct m210_note_data const *const data)
{
    static struct m210_note_data const penup = {0x0000, htole16(0x0080)};
    return memcmp(data, &penup, sizeof(struct m210_note_data)) == 0;
}

enum m210_err m210_set_mode(struct m210 const *const m210,
                            enum m210_mode_indicator const mode_indicator,
                            enum m210_mode const mode)
{
    uint8_t rpt[] = {0x80, 0xb5, 0x00, 0x00};
    rpt[2] = mode_indicator;
    rpt[3] = mode;
    return m210_write_and_wait(m210, rpt, sizeof(rpt));
}

enum m210_err m210_config_tablet_mode(struct m210 const *const m210,
                                      enum m210_area_size const area_size,
                                      enum m210_orientation const orientation)
{
    /* Area size byte in M210 is counter-intuitive: 0x00 means the largest,
     * 0x09 the smallest. However, in our API, area sizes are handled
     * intuitively and therefore needs to be changed here. */
    uint8_t rpt[] = {0x80, 0xb6, 0x00, 0x00};
    rpt[2] = abs(area_size - 0x09);
    rpt[3] = orientation;
    return m210_write_and_wait(m210, rpt, sizeof(rpt));
}

enum m210_err m210_fwrite_tablet_data(struct m210 const *const m210, FILE *const stream)
{
    enum m210_err err;
    uint8_t rpt[M210_RESPONSE_SIZE_IFACE1];
    uint8_t const sig[] = {0x08};

    memset(&rpt, 0, sizeof(rpt));

    while (1) {
        err = m210_read(m210, 1, &rpt, sizeof(rpt));
        if (err)
            return err;
        if (memcmp(rpt, sig, sizeof(sig)) != 0)
            return M210_ERR_BADMSG;
        switch (rpt[5]) {
        case M210_TABLET_PEN_RELEASED:
        case M210_TABLET_PEN_PRESSED:
            break;
        default:
            return M210_ERR_BADMSG;
        }
        if (fwrite(rpt+sizeof(sig), sizeof(rpt)-sizeof(sig), 1, stream) != 1)
            return M210_ERR_SYS;
        if (fflush(stream))
            return M210_ERR_SYS;
    }
    return M210_ERR_OK;
}

enum m210_err m210_fwrite_mouse_data(struct m210 const *const m210, FILE *const stream)
{
    enum m210_err err;
    uint8_t rpt[M210_RESPONSE_SIZE_IFACE0];
    size_t i;

    memset(&rpt, 0, sizeof(rpt));

    while (1) {
        err = m210_read(m210, 0, &rpt, sizeof(rpt));
        if (err)
            return err;
        for (i = 0; i < sizeof(rpt); i = i + 6) {
            if (rpt[i] == 0x00)
                break; /* No more mouse data in this report. */
            switch (rpt[i]) {
            case M210_MOUSE_BATTERY_UNKNOWN:
            case M210_MOUSE_BATTERY_LOW:
            case M210_MOUSE_BATTERY_HIGH:
                break;
            default:
                /* Unexpected battery state. */
                return M210_ERR_BADMSG;
            }
            switch (rpt[i+1]) {
            case M210_MOUSE_PEN_OUT_OF_RANGE:
            case M210_MOUSE_PEN_HOVERING:
            case M210_MOUSE_PEN_TIP_PRESSED:
            case M210_MOUSE_PEN_SWITCH_PRESSED:
            case M210_MOUSE_PEN_BOTH_PRESSED:
                break;
            default:
                /* Unexpected pen state. */
                return M210_ERR_BADMSG;
            }
            if (fwrite(rpt+i, 6, 1, stream) != 1)
                return M210_ERR_SYS;
            if (fflush(stream))
                return M210_ERR_SYS;
        }
    }
    return M210_ERR_OK;
}
