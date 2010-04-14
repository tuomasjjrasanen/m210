#ifndef NOTETAKER_INTERNAL_H
#define NOTETAKER_INTERNAL_H

#include <linux/input.h>

#include "notetaker.h"

#define NOTETAKER_MODE_NONE   0x00
#define NOTETAKER_MODE_MOBILE 0x03

#define NOTETAKER_LED_NONE  0x00
#define NOTETAKER_LED_PEN   0x01
#define NOTETAKER_LED_MOUSE 0x02

#define NOTETAKER_STATUS_NONE         0x00
#define NOTETAKER_STATUS_BATTERY_LOW  0x01
#define NOTETAKER_STATUS_BATTERY_GOOD 0x02

#define NOTETAKER_IFACE_PAD   0
#define NOTETAKER_IFACE_PEN   1
#define NOTETAKER_IFACE_COUNT 2

static const uint8_t IFACE_PACKET_SIZES[] = {64, 8};

const struct hidraw_devinfo DEVINFO_M210 = {
    BUS_USB,
    0x0e20,
    0x0101,
};

struct version_response {
    uint8_t special_command;
    uint8_t command_version;
    uint8_t product_id;
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t analog_product_id;
    uint8_t tab_mode;
};

#endif /* NOTETAKER_INTERNAL_H */
