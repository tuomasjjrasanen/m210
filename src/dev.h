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

#ifndef DEV_H
#define DEV_H

#include <stdio.h>
#include <stdint.h>

#include "err.h"

#define M210_DEV_MAX_MEMORY 4063232

typedef struct m210_dev *m210_dev_t;

enum m210_dev_mode {
        M210_DEV_MODE_MOUSE,
        M210_DEV_MODE_TABLET
};

struct m210_dev_info {
        uint16_t firmware_version;
        uint16_t analog_version;
        uint16_t pad_version;
        enum m210_dev_mode mode;
        uint32_t used_memory;
};

enum m210_err m210_dev_connect(m210_dev_t *dev_ptr);

enum m210_err m210_dev_disconnect(m210_dev_t *dev_ptr);

enum m210_err m210_dev_get_info(m210_dev_t dev, struct m210_dev_info *info_ptr);

enum m210_err m210_dev_download_notes(m210_dev_t dev, FILE *stream_ptr);

enum m210_err m210_dev_delete_notes(m210_dev_t dev);

enum m210_err m210_dev_set_mode(m210_dev_t dev, enum m210_dev_mode mode);

/* enum m210_mode_indicator { */
/*     M210_MODE_INDICATOR_TABLET=0x01, */
/*     M210_MODE_INDICATOR_MOUSE=0x02 */
/* }; */

/* enum m210_mouse_battery { */
/*     M210_MOUSE_BATTERY_UNKNOWN = 0x40, */
/*     M210_MOUSE_BATTERY_LOW = 0x41, */
/*     M210_MOUSE_BATTERY_HIGH = 0x42 */
/* }; */

/* enum m210_mouse_pen { */
/*     M210_MOUSE_PEN_OUT_OF_RANGE = 0x00, */

/*     M210_MOUSE_PEN_HOVERING = 0x08,       /\* The pen is in the range, */
/*                                            * hovering and neither the */
/*                                            * tip nor the switch is */
/*                                            * pressed. *\/ */

/*     M210_MOUSE_PEN_TIP_PRESSED = 0x01, */

/*     M210_MOUSE_PEN_SWITCH_PRESSED = 0x02, */

/*     M210_MOUSE_PEN_BOTH_PRESSED = 0x03 */
/* }; */

/* enum m210_orientation { */
/*     M210_ORIENTATION_TOP = 0x00, */
/*     M210_ORIENTATION_LEFT = 0x01, */
/*     M210_ORIENTATION_RIGHT = 0x02 */
/* }; */

/* enum m210_area_size { */
/*     M210_AREA_SIZE_MIN = 0x00, */
/*     M210_AREA_SIZE0 = 0x00, */
/*     M210_AREA_SIZE1 = 0x01, */
/*     M210_AREA_SIZE2 = 0x02, */
/*     M210_AREA_SIZE3 = 0x03, */
/*     M210_AREA_SIZE4 = 0x04, */
/*     M210_AREA_SIZE5 = 0x05, */
/*     M210_AREA_SIZE6 = 0x06, */
/*     M210_AREA_SIZE7 = 0x07, */
/*     M210_AREA_SIZE8 = 0x08, */
/*     M210_AREA_SIZE9 = 0x09, */
/*     M210_AREA_SIZE_MAX = 0x09 */
/* }; */

/* enum m210_tablet_pen { */
/*     M210_TABLET_PEN_RELEASED = 0x10, /\* Neither the tip nor the switch */
/*                                       * is pressed. *\/ */

/*     M210_TABLET_PEN_PRESSED = 0x11   /\* Either the tip or the switch */
/*                                       * is pressed or even both. *\/ */
/* }; */

/* enum m210_tablet_bound { */
/*     M210_TABLET_BOUND_X_MIN = 0x03e8, */
/*     M210_TABLET_BOUND_X_MAX = 0x2328, */
/*     M210_TABLET_BOUND_Y_MIN = 0x0000, */
/*     M210_TABLET_BOUND_Y_MAX = 0x1770 */
/* }; */
/* struct m210_tablet_data { */
/*     uint16_t x; */
/*     uint16_t y; */
/*     uint8_t pen; /\* enum m210_tablet_pen *\/ */
/*     uint16_t pressure; */
/* } __attribute__((packed)); */

/* struct m210_mouse_data { */
/*     uint8_t battery; */
/*     uint8_t pen; */
/*     uint16_t x; */
/*     uint16_t y; */
/* } __attribute__((packed)); */

/* enum m210_err m210_config_tablet_mode(struct m210 const *m210, */
/*                                       enum m210_area_size area_size, */
/*                                       enum m210_orientation orientation); */

/* enum m210_err m210_fwrite_tablet_data(struct m210 const *m210, FILE *stream); */

/* enum m210_err m210_fwrite_mouse_data(struct m210 const *m210, FILE *stream); */

#endif /* DEV_H */
