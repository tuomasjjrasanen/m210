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

#include <stdio.h>
#include <stdint.h>

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
    err_timeout
};

const char *m210_err_str(enum m210_err err);
int m210_err_printf(enum m210_err err, const char *s);

/*
  Position data:
  +-----+-----------+-+-+-+-+-+-+-+-+
  |Byte#|Description|7|6|5|4|3|2|1|0|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  1  |X LOW      |x|x|x|x|x|x|x|x|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  2  |X HIGH     |X|X|X|X|X|X|X|X|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  3  |Y LOW      |y|y|y|y|y|y|y|y|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  4  |Y HIGH     |Y|Y|Y|Y|Y|Y|Y|Y|
  +-----+-----------+-+-+-+-+-+-+-+-+

*/

/*
  Note stream:

  note1          ...          noteN
  +----------------------+    +----------------------+
  |                      |    |                      |
  header data1 ... dataN      header data1 ... dataN
*/

#define M210_NOTE_DATA_COMPONENT_LEN 2
#define M210_NOTE_DATA_HIGH 1
#define M210_NOTE_DATA_LOW 0
struct m210_note_data {
    uint8_t x[M210_NOTE_DATA_COMPONENT_LEN];
    uint8_t y[M210_NOTE_DATA_COMPONENT_LEN];
} __attribute__((packed));

int m210_note_data_is_pen_up(const struct m210_note_data *data);

#define M210_NOTE_HEADER_STATE_EMPTY 0x9f
#define M210_NOTE_HEADER_STATE_UNFINISHED 0x5f
#define M210_NOTE_HEADER_STATE_FINISHED_BY_USER 0x3f
#define M210_NOTE_HEADER_STATE_FINISHED_BY_SOFTWARE 0x1f

#define M210_NOTE_HEADER_NEXT_NOTE_ADDR_LEN 3
#define M210_NOTE_HEADER_RESERVED_LEN 8

struct m210_note_header {
    uint8_t next_note_addr[M210_NOTE_HEADER_NEXT_NOTE_ADDR_LEN];
    uint8_t state;
    uint8_t note_number;
    uint8_t max_note_number;
    uint8_t reserved[M210_NOTE_HEADER_RESERVED_LEN];
} __attribute__((packed));

uint32_t m210_note_data_len(struct m210_note_header *header);

/**
   An opaque object representing a M210 device.
*/
struct m210 {
    int fds[M210_IFACE_COUNT];
};

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
enum m210_err m210_open(struct m210 *m210, char** hidraw_paths);

/**
   Close the M210 device connection.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @m210 an object represeting the M210 device.

*/
enum m210_err m210_close(struct m210 *m210);

/**
   Return information about the M210 device.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_badmsg - device sent unexpected message

   @m210 an object represeting the M210 device.

   @info an address where the info will be stored.

*/
enum m210_err m210_get_info(const struct m210 *m210, struct m210_info *info);

/**
   Requests the M210 device to delete all notes stored in it's memory.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_badmsg - device sent unexpected message

   @m210 an object represeting the M210 device.

*/
enum m210_err m210_delete_notes(const struct m210 *m210);

/**
   Return the total size of notes in bytes.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_badmsg - device sent unexpected message

   @m210 an object represeting the M210 device.

   @size an address where the size will be stored.

*/
enum m210_err m210_get_notes_size(const struct m210 *m210, ssize_t *size);

/**
   Read notes from a M210 device and write them to a stream.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_badmsg - device sent unexpected message

   @m210 an object represeting the M210 device.

   @stream a writable destination stream.

 */
enum m210_err m210_fwrite_notes(const struct m210 *m210, FILE *stream);

#endif /* M210_H */
