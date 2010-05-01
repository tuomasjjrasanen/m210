/*
  libm210 - API for Pegasus Mobile NoteTaker M210
  Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

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
#ifndef M210_H
#define M210_H

#define M210_IFACE_COUNT 2

struct m210_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t mode;
};

enum m210_err {
    err_ok,
    err_sys,
    err_baddev,
    err_nodev,
    err_badmsg,
    err_packets,
    err_timeout
};

struct m210_note_data {
    uint16_t x;
    uint16_t y;
} __attribute__((packed));

int m210_note_data_is_pen_up(const struct m210_note_data *data);

enum m210_note_state {
    empty = 0x9f,
    unfinished = 0x5f,
    finished_by_user = 0x3f,
    finished_by_software = 0x1f
};

struct m210_note {
    uint8_t number;
    uint8_t max_number;
    enum m210_note_state state;
    struct m210_note_data *datav;
    ssize_t datac;
};

struct m210_note_header {
    uint8_t next_note_addr[3];
    uint8_t state;
    uint8_t note_num;
    uint8_t max_note_num;
    uint8_t reserved[8];
} __attribute__((packed));

/**
   An opaque object representing a M210 device.
*/
typedef struct m210 m210_t;

typedef enum m210_err m210_err_t;

/**
   Open a hid connetion to a M210 device and return an object
   representing it.

   Succesfully opened device must be freed with m210_free().

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_nodev - a M210 device was not found

   - err_baddev - hidraw_paths did not represent interfaces 0 and 1 of
   a M210 device.

   @m210 an address of an object representing the M210 device.

   @hidraw_paths a list of two null-terminated paths of hidraw device
   nodes for interface 0 and 1 respectively or NULL in which case
   proper nodes are searched with udev.

*/
m210_err_t m210_open(m210_t **m210, char** hidraw_paths);

/**
   Close the M210 device connection and free all resources.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set
   appropriately. Memory is not freed!

   @m210 an object represeting the M210 device.

*/
m210_err_t m210_free(m210_t *m210);

/**
   Return information about the M210 device.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @m210 an object represeting the M210 device.

   @info an address where the info will be stored.

*/
m210_err_t m210_get_info(m210_t *m210, struct m210_info *info);

/**
   Requests the M210 device to delete all notes stored in it's memory.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @m210 an object represeting the M210 device.

*/
m210_err_t m210_delete_notes(m210_t *m210);

m210_err_t m210_get_packet_count(m210_t *m210, uint16_t *packet_count);

m210_err_t m210_fwrite_packets(m210_t *m210, FILE *stream);

/* m210_err_t m210_get_notes(m210_t *m210, struct m210_note **notev, uint8_t *notec); */

/* m210_err_t m210_fwrite_notes(m210_t *m210, FILE *stream); */

/* m210_err_t m210_get_packets(m210_t *m210, struct m210_packet *packetv, uint16_t packetc); */



#endif /* M210_H */
