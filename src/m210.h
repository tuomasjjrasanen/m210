/**
  \file
  \mainpage libm210 - API for Pegasus Mobile NoteTaker M210
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

/**
   Mode values.
 */
enum m210_mode {
    /**
       Mode value when M210 is connected to host via USB.
     */
    tablet = 0x02
};

/**
   An object representing miscellaneous information of a M210 device.
 */
struct m210_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    /**
       Current mode, possible values defined by #m210_mode.
     */
    uint8_t mode;
};

/**
   Possible error codes. Functions returning these values define their
   semantics.
 */
enum m210_err {
    err_ok,
    err_sys,
    err_baddev,
    err_nodev,
    err_badmsg,
    err_timeout
};

/**
   Return an error message describing the error code.

   \param err a m210 error code
 */
const char *m210_err_str(enum m210_err err);

/**
   Produce an error message on the standard error output describing
   the error code. First program_invocation_name is printed, followed
   by a colon and a blank. Then the message (if it is not NULL),
   followed by a colon and a blank. Then a string describing the error
   code as returned by m210_err_str() and if the error code was
   err_sys, a message as printed by perror(). A new line is printed at
   the end.

   \param err a m210 error code
   \param s error message
 */
int m210_err_printf(enum m210_err err, const char *s);

/**
   An object representing a note data block in a stream. A data block
   can represent a position or a pen up -event.

   Position:
   \verbatim
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
   \endverbatim

   Pen up:
   \verbatim
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
   \endverbatim
   X and y components are defined as byte arrays to emphasize the need
   to consider byte ordering when accessing these values.

   m210_note_data_get_x() and m210_note_data_get_y() take care of byte ordering.

 */
struct m210_note_data {
    /**
       Little endian uint16_t bytes representing x-coordinate.
     */
    uint8_t x[2];
    /**
       Little endian uint16_t bytes representing y-coordinate.
     */
    uint8_t y[2];
} __attribute__((packed));

/**
   Return the value of x in host byte order.

   \param data an address of data block object
 */
uint16_t m210_note_data_get_x(const struct m210_note_data *data);

/**
   Return the value of y in host byte order.

   \param data an address of data block object
 */
uint16_t m210_note_data_get_y(const struct m210_note_data *data);

/**
   Return 1 if the data block represents a pen up-event, 0 otherwise.

   \param data an address of data block object
 */
int m210_note_data_is_pen_up(const struct m210_note_data *data);

/**
   Note state values.
 */
enum m210_note_state {
    /**
       Note does not contain any data.
     */
    empty = 0x9f,
    /**
       Note is contains data but is not closed yet.
     */
    unfinished = 0x5f,
    /**
       Note contains data and is close by a user.
     */
    finished_by_user = 0x3f,
    /**
       Note contains data and is closed programmatically.
     */
    finished_by_software = 0x1f
};

/**
   \verbatim
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
   \endverbatim
 */
struct m210_note_header {
    /**
       Position of the first byte of the next header in the stream.
     */
    uint8_t next_header_pos[3];
    /**
       State of the note, possible values defined by #m210_note_state.
     */
    uint8_t state;
    /**
       Sequence number of the note.
     */
    uint8_t note_number;
    /**
       Total number of notes in the stream.
     */
    uint8_t max_note_number;
    /**
       Unused and undefined bytes.
     */
    uint8_t reserved[8];
} __attribute__((packed));

/**
   Return the position of a next note in a stream in host byte
   order. The position is defined as a byte offset from the beginning
   of a stream.

   \param header an address of a header block object
   \return value
 */
uint32_t m210_note_header_next_header_pos(const struct m210_note_header *header);

/**
   Number of M210 usb interfaces.
 */
#define M210_USB_INTERFACE_COUNT 2

/**
   An object representing a M210 device.
*/
struct m210 {
    int fds[M210_USB_INTERFACE_COUNT];
};

/**
   Open a hid connetion to a M210 device and return an object
   representing it.

   Succesfully opened device must be closed with m210_close().

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \retval err_nodev a M210 device was not found

   \retval err_baddev hidraw_paths did not represent interfaces 0 and 1 of
   a M210 device.

   \param m210 an address of an object representing the M210 device.

   \param hidraw_paths a list of two null-terminated paths of hidraw device
   nodes for interface 0 and 1 respectively or NULL in which case
   proper nodes are searched with udev.

*/
enum m210_err m210_open(struct m210 *m210, char** hidraw_paths);

/**
   Close the M210 device connection.

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \param m210 an object represeting the M210 device.

*/
enum m210_err m210_close(struct m210 *m210);

/**
   Return information about the M210 device.

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \retval err_badmsg device sent unexpected message

   \param m210 an object represeting the M210 device.

   \param info an address where the info will be stored.

*/
enum m210_err m210_get_info(const struct m210 *m210, struct m210_info *info);

/**
   Requests the M210 device to delete all notes stored in it's memory.

   Return values:

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \retval err_badmsg device sent unexpected message

   \param m210 an object represeting the M210 device.

*/
enum m210_err m210_delete_notes(const struct m210 *m210);

/**
   Return the total size of notes in bytes. Theoretical maximum size
   is 4063232:

      * Packets are numbered with 16 bit integers.
        => Maximum number of packets: 2**16 = 65536

      * Each packet is 64 bytes wide, last 62 bytes represent bytes in memory.
        The first two bytes represent the packet sequence number.
        => Maximum number of bytes in memory: 2**16 * 62 = 4063232

      * A 32bit integer can address 2**32 different bytes which is way more
        than maximum number of bytes in m210 memory.

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \retval err_badmsg device sent unexpected message

   \param m210 an object represeting the M210 device.

   \param size an address where the size will be stored.

*/
enum m210_err m210_get_notes_size(const struct m210 *m210, uint32_t *size);

/**
   Read notes from a M210 device and write them to a stream. Each note
   consist of a 14 byte wide header block and an arbitrary number of 4
   byte wide data blocks. Stream consist of an arbitrary number of
   notes.

   Note stream:
   \verbatim
            note1          ...          noteN
   +----------------------+    +----------------------+
   | 14      4         4  |    | 14      4         4  |
    header data1 ... dataN      header data1 ... dataN
   \endverbatim
   m210_note_header and m210_note_data are defined to
   represent header and data block respectively. Data block can
   represent a position or a pen up -event. m210_note_data_is_pen_up()
   can be used to determine if a data block represents the latter.

   \retval err_ok success

   \retval err_sys system call failed and errno is set appropriately

   \retval err_badmsg device sent unexpected message

   \param m210 an object represeting the M210 device.

   \param stream a writable destination stream.

 */
enum m210_err m210_fwrite_notes(const struct m210 *m210, FILE *stream);

enum m210_mode {
    basic=1,
    tablet=2
};

enum m210_led {
    pen=1,
    mouse=2
};

enum m210_err m210_set_mode(const struct m210 *m210, enum m210_led led,
                            enum m210_mode mode);

enum m210_orientation {
    top,
    left,
    right
};

#define M210_SCALE_MAX 9

enum m210_err m210_config_tablet(const struct m210 *m210, uint8_t scale,
                                 enum m210_orientation orientation);

/* struct m210_pen_data { */

/* }; */

enum m210_err m210_fwrite_tablet(const struct m210 *m210, FILE *stream);

#endif /* M210_H */
