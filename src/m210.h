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

#ifndef M210_H
#define M210_H

#include <stdio.h>
#include <stdint.h>

enum m210_err {
    M210_ERR_OK,
    M210_ERR_SYS,
    M210_ERR_BADDEV,
    M210_ERR_NODEV,
    M210_ERR_BADMSG,
    M210_ERR_TIMEOUT
};

enum m210_mode_indicator {
    M210_MODE_INDICATOR_TABLET=0x01,
    M210_MODE_INDICATOR_MOUSE=0x02
};

enum m210_mode {
    M210_MODE_MOUSE=0x01,
    M210_MODE_TABLET=0x02
};

enum m210_note_state {
    M210_NOTE_STATE_EMPTY = 0x9f,               /* Note does not
                                                 * contain any
                                                 * data. */

    M210_NOTE_STATE_UNFINISHED = 0x5f,          /* Note contains data
                                                 * but is not closed
                                                 * yet. */

    M210_NOTE_STATE_FINISHED_BY_USER = 0x3f,    /* Note contains data
                                                 * and is closed by a
                                                 * user. */

    M210_NOTE_STATE_FINISHED_BY_SOFTWARE = 0x1f /* Note contains data
                                                 * and is closed
                                                 * programmatically. */
};

enum m210_mouse_battery {
    M210_MOUSE_BATTERY_UNKNOWN = 0x40,
    M210_MOUSE_BATTERY_LOW = 0x41,
    M210_MOUSE_BATTERY_HIGH = 0x42
};

enum m210_mouse_pen {
    M210_MOUSE_PEN_OUT_OF_RANGE = 0x00,

    M210_MOUSE_PEN_HOVERING = 0x08,       /* The pen is in the range,
                                           * hovering and neither the
                                           * tip nor the switch is
                                           * pressed. */

    M210_MOUSE_PEN_TIP_PRESSED = 0x01,

    M210_MOUSE_PEN_SWITCH_PRESSED = 0x02,

    M210_MOUSE_PEN_BOTH_PRESSED = 0x03
};

enum m210_orientation {
    M210_ORIENTATION_TOP = 0x00,
    M210_ORIENTATION_LEFT = 0x01,
    M210_ORIENTATION_RIGHT = 0x02
};

enum m210_area_size {
    M210_AREA_SIZE_MIN = 0x00,
    M210_AREA_SIZE0 = 0x00,
    M210_AREA_SIZE1 = 0x01,
    M210_AREA_SIZE2 = 0x02,
    M210_AREA_SIZE3 = 0x03,
    M210_AREA_SIZE4 = 0x04,
    M210_AREA_SIZE5 = 0x05,
    M210_AREA_SIZE6 = 0x06,
    M210_AREA_SIZE7 = 0x07,
    M210_AREA_SIZE8 = 0x08,
    M210_AREA_SIZE9 = 0x09,
    M210_AREA_SIZE_MAX = 0x09
};

enum m210_tablet_pen {
    M210_TABLET_PEN_RELEASED = 0x10, /* Neither the tip nor the switch
                                      * is pressed. */

    M210_TABLET_PEN_PRESSED = 0x11   /* Either the tip or the switch
                                      * is pressed or even both. */
};

enum m210_tablet_bound {
    M210_TABLET_BOUND_X_MIN = 0x03e8,
    M210_TABLET_BOUND_X_MAX = 0x2328,
    M210_TABLET_BOUND_Y_MIN = 0x0000,
    M210_TABLET_BOUND_Y_MAX = 0x1770
};
struct m210_tablet_data {
    uint16_t x;
    uint16_t y;
    uint8_t pen; /* enum m210_tablet_pen */
    uint16_t pressure;
} __attribute__((packed));

struct m210_mouse_data {
    uint8_t battery;
    uint8_t pen;
    uint16_t x;
    uint16_t y;
} __attribute__((packed));

struct m210_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t mode;
};

/*
  An object representing a note data block in a stream. A data block
  can represent a position or a pen up -event.

  Position:

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

  Pen up:

  +-----+-----------+-+-+-+-+-+-+-+-+
  |Byte#|Description|7|6|5|4|3|2|1|0|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  1  |Pen up     |0|0|0|0|0|0|0|0|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  2  |Pen up     |0|0|0|0|0|0|0|0|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  3  |Pen up     |0|0|0|0|0|0|0|0|
  +-----+-----------+-+-+-+-+-+-+-+-+
  |  4  |Pen up     |1|0|0|0|0|0|0|0|
  +-----+-----------+-+-+-+-+-+-+-+-+

  X and y components are defined as byte arrays to emphasize the need
  to consider byte ordering when accessing these values.

  m210_note_data_get_x() and m210_note_data_get_y() take care of byte ordering.

*/
struct m210_note_data {
    uint16_t x;
    uint16_t y;
} __attribute__((packed));

/*
  +-----+------------------+-+-+-+-+-+-+-+-+
  |Byte#|Description       |7|6|5|4|3|2|1|0|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  1  |Next note pos LOW |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  2  |Next note pos MID |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  3  |Next note pos HIGH|x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  4  |State             |x|x|x|1|1|1|1|1|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  5  |Note num          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  6  |Max note num      |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  7  |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  8  |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  9  |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  10 |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  11 |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  12 |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  13 |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+
  |  14 |Reserved          |x|x|x|x|x|x|x|x|
  +-----+------------------+-+-+-+-+-+-+-+-+

*/
struct m210_note_header {
    uint8_t next_header_pos[3];
    uint8_t state;
    uint8_t note_number;
    uint8_t max_note_number;
    uint8_t reserved[8];
} __attribute__((packed));

#define M210_USB_INTERFACE_COUNT 2

struct m210 {
    int fds[M210_USB_INTERFACE_COUNT];
};

char const *m210_strerror(enum m210_err err);

/*
  Produce an error message on the standard error output describing the
  error code. First program_invocation_name is printed, followed by a
  colon and a blank. Then the message (if it is not NULL), followed by
  a colon and a blank. Then a string describing the error code as
  returned by m210_err_str() and if the error code was err_sys, a
  message as printed by perror(). A new line is printed at the end.

*/
int m210_perror(enum m210_err err, char const *s);

int m210_note_data_is_pen_up(struct m210_note_data const *data);

/*
  Return the position of a next note in a stream in host byte
  order. The position is defined as a byte offset from the beginning
  of a stream.

*/
uint32_t m210_note_header_next_header_pos(struct m210_note_header const *header);

/* Open a hid connetion to a device and return an object representing
   it. Succesfully opened device must be closed with m210_close().

*/
enum m210_err m210_open(struct m210 *m210);

enum m210_err m210_close(struct m210 *m210);

enum m210_err m210_get_info(struct m210 const *m210, struct m210_info *info);

enum m210_err m210_delete_notes(struct m210 const  *m210);

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
enum m210_err m210_get_notes_size(struct m210 const *m210, uint32_t *size);

/*
  Read notes from a device and write them to a stream. Each note
  consist of a 14 byte wide header block and an arbitrary number of 4
  byte wide data blocks. Stream consist of an arbitrary number of
  notes.

  Note stream:

             note1           ...            noteN
  +------------------------+     +------------------------+
  |  14  |  4  |     |  4  |     |  14  |  4  |     |  4  |
  +------------------------+     +------------------------+
   header data1  ...  dataN       header data1  ...  dataN

  Structs m210_note_header and m210_note_data are defined to represent
  header and data block respectively. Data block can represent a
  position or a pen up -event. m210_note_data_is_pen_up() can be used
  to determine if a data block represents the latter.

*/
enum m210_err m210_fwrite_note_data(struct m210 const *m210, FILE *stream);

enum m210_err m210_set_mode(struct m210 const *m210,
                            enum m210_mode_indicator mode_indicator,
                            enum m210_mode mode);

enum m210_err m210_config_tablet_mode(struct m210 const *m210,
                                      enum m210_area_size area_size,
                                      enum m210_orientation orientation);

enum m210_err m210_fwrite_tablet_data(struct m210 const *m210, FILE *stream);

enum m210_err m210_fwrite_mouse_data(struct m210 const *m210, FILE *stream);

#endif /* M210_H */