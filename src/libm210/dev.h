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

#ifndef DEV_H
#define DEV_H

#include <stdio.h>
#include <stdint.h>

#include "err.h"

#define M210_DEV_MODE_MOUSE  0x01
#define M210_DEV_MODE_TABLET 0x02

#define M210_DEV_MAX_MEMORY 4063232 /* Bytes. */

typedef struct m210_dev *m210_dev;

struct m210_dev_info {
	uint16_t firmware_version;
	uint16_t analog_version;
	uint16_t pad_version;
	uint8_t mode;
	uint32_t used_memory;
};

enum m210_err m210_dev_connect(m210_dev *devp);
enum m210_err m210_dev_disconnect(m210_dev *devp);
enum m210_err m210_dev_get_info(m210_dev dev, struct m210_dev_info *infop);
enum m210_err m210_dev_download_notes(m210_dev dev, FILE *file);
enum m210_err m210_dev_delete_notes(m210_dev dev);

#endif /* DEV_H */
