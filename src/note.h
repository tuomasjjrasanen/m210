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

#ifndef NOTE_H
#define NOTE_H

#include <stdio.h>
#include <stdint.h>

enum m210_note_err {
        M210_NOTE_ERR_OK,
        M210_NOTE_ERR_SYS,
        M210_NOTE_ERR_BAD_HEAD,
        M210_NOTE_ERR_BAD_BODY,
        M210_NOTE_ERR_EOF
};

struct m210_note_coord {
        uint16_t x;
        uint16_t y;
} __attribute__((packed));

struct m210_note_path {
        struct m210_note_coord *coords;
        size_t coord_count;
};

struct m210_note {
        uint8_t number;
        struct m210_note_path *paths;
        size_t path_count;
};

enum m210_note_err m210_note_create_next(struct m210_note **note_ptr_ptr,
                                         FILE *stream_ptr);

void m210_note_destroy(struct m210_note **note_ptr_ptr);

#endif /* NOTE_H */
