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

#include <stdint.h>

struct m210_note_header {
    uint8_t next_header_position[3];
    uint8_t state;
    uint8_t note_number;
    uint8_t last_note_number;
    uint8_t reserved[8];
} __attribute__((packed));

struct m210_note_data {
    uint8_t x[2];
    uint8_t y[2];
} __attribute__((packed));

#endif /* NOTE_H */
