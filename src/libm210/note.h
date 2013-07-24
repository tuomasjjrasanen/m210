/* libm210
 * Copyright (C) 2011, 2013 Tuomas Räsänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NOTE_H
#define NOTE_H

#include <stdio.h>
#include <stdint.h>

#include "err.h"

struct m210_note_body {
	int16_t x;
	int16_t y;
	uint16_t pressure;
};

struct m210_note_head {
	uint8_t number;
	ssize_t bodyc;
};

enum m210_err m210_note_read_head(struct m210_note_head *headp, FILE *file);
enum m210_err m210_note_read_body(struct m210_note_body *bodyp, FILE *file);

#endif /* NOTE_H */
