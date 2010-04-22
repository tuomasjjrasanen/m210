#ifndef NOTETAKER_H
#define NOTETAKER_H

#define NOTETAKER_MODE_XY     0x01
#define NOTETAKER_MODE_TABLET 0x02

#define NOTETAKER_ORIENTATION_TOP   0x00
#define NOTETAKER_ORIENTATION_LEFT  0x01
#define NOTETAKER_ORIENTATION_RIGHT 0x02

#define NOTETAKER_SCALE_0   0x00
#define NOTETAKER_SCALE_1   0x01
#define NOTETAKER_SCALE_2   0x02
#define NOTETAKER_SCALE_3   0x03
#define NOTETAKER_SCALE_4   0x04
#define NOTETAKER_SCALE_5   0x05
#define NOTETAKER_SCALE_6   0x06
#define NOTETAKER_SCALE_7   0x07
#define NOTETAKER_SCALE_8   0x08
#define NOTETAKER_SCALE_9   0x09
#define NOTETAKER_SCALE_MIN NOTETAKER_SCALE_0
#define NOTETAKER_SCALE_MAX NOTETAKER_SCALE_9

#define NOTETAKER_IFACE_COUNT 2

struct notetaker_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t mode;
};

/**
   Opaque object representing a NoteTaker device.
*/
typedef struct notetaker notetaker_t;

notetaker_t *notetaker_open(void);

/**
   Open a hid connetion to a NoteTaker device and return an object
   representing it.

   On error, NULL is returned and errno is set appropriately.

   ERRORS
     EINVAL
       hidraw_paths did not represent interfaces 0 and 1 of
       a NoteTaker device.

   @hidraw_paths a list of two null-terminated paths of hidraw device
   nodes for interface 0 and 1 respectively.
 */
notetaker_t *notetaker_open_from_hidraw_paths(char **hidraw_paths);

/**
   Close the NoteTaker device connection and free all resources.

   @notetaker an address of the object represeting the NoteTaker
   device.
 */
void notetaker_close(notetaker_t *notetaker);

/**
   Return information about the NoteTaker device.

   On error, -1 is returned and errno is set appropriately. In this
   case, *info is left unmodified.

   @notetaker an address of the object represeting the NoteTaker
   device.

   @info an address where the info will be stored.
 */
int notetaker_get_info(notetaker_t *notetaker, struct notetaker_info *info);

/**
   Wait until the NoteTaker device is ready or until the time limit is
   reached. If timeout is NULL, notetaker_wait_ready() blocks until
   the NoteTaker device becomes ready.

   Return 0 if the time limit is reached and the device is not
   ready. Return 1 when the device is ready. On error, -1 is returned
   and errno is set appropriately.

   @notetaker an address of the object represeting the NoteTaker
   device.

   @timeout an address of the maximum time value to wait before giving
   up. Can be NULL in which case notetaker_wait_ready() blocks
   forever.
 */
int notetaker_wait_ready(notetaker_t *notetaker, const struct timeval *timeout);

/**
   Requests the NoteTaker device to delete all notes.

   On error, -1 is returned and errno is set appropriately.

   @notetaker an address of the object represeting the NoteTaker
   device.
 */
int notetaker_delete_notes(notetaker_t *notetaker);

#endif /* NOTETAKER_H */
