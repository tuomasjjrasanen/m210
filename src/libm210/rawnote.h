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

#ifndef RAWNOTE_H
#define RAWNOTE_H

#include <stdint.h>

#define M210_RAWNOTE_STATE_EMPTY		0x9f
#define M210_RAWNOTE_STATE_UNFINISHED		0x5f
#define M210_RAWNOTE_STATE_FINISHED_BY_USER	0x3f
#define M210_RAWNOTE_STATE_FINISHED_BY_SOFTWARE 0x1f

struct m210_rawnote_head {
	uint8_t next_pos[3]; /* Little-endian. */
	uint8_t state;
	uint8_t number;
	uint8_t last_number;
	uint8_t reserved[8];
} __attribute__((packed));

static struct m210_rawnote_head const M210_RAWNOTE_HEAD_LAST = {
	{0x00, 0x00, 0x00},
	0x00,
	0x00,
	0x00,
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

struct m210_rawnote_body {
	uint8_t x[2]; /* Little-endian. */
	uint8_t y[2]; /* Little-endian. */
} __attribute__((packed));

static struct m210_rawnote_body const M210_RAWNOTE_BODY_PENUP = {
	{0x00, 0x00},
	{0x00, 0x80}
};

#endif /* RAWNOTE_H */
