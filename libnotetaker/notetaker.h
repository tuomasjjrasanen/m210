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

enum notetaker_err {
    err_ok,
    err_sys,
    err_baddev,
    err_nodev,
    err_badmsg
};

/**
   An opaque object representing a NoteTaker device.
*/
typedef struct notetaker* notetaker_t;

typedef enum notetaker_err notetaker_err_t;

/**
   Open a hid connetion to a first found NoteTaker device and return
   an object representing it.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_nodev - a NoteTaker device was not found

   @notetaker an address of an object representing the NoteTaker
   device.

*/
notetaker_err_t notetaker_open(notetaker_t *notetaker);

/**
   Open a hid connetion to a NoteTaker device and return an object
   representing it.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_baddev - hidraw_paths did not represent interfaces 0 and 1 of
   a NoteTaker device.

   @notetaker an address of an object representing the NoteTaker
   device.

   @hidraw_paths a list of two null-terminated paths of hidraw device
   nodes for interface 0 and 1 respectively.

*/
notetaker_err_t notetaker_open_from_hidraw_paths(notetaker_t *notetaker,
                                                 char **hidraw_paths);

/**
   Close the NoteTaker device connection and free all resources.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set
   appropriately. Memory is not freed!

   @notetaker an object represeting the NoteTaker device.

*/
notetaker_err_t notetaker_free(notetaker_t notetaker);

/**
   Return information about the NoteTaker device.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @notetaker an object represeting the NoteTaker device.

   @info an address where the info will be stored.

*/
notetaker_err_t notetaker_get_info(notetaker_t notetaker,
                                   struct notetaker_info *info);

/**
   Requests the NoteTaker device to delete all notes.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @notetaker an object represeting the NoteTaker device.

*/
notetaker_err_t notetaker_delete_notes(notetaker_t notetaker);

/**
   Return data size required for full download.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @notetaker an object represeting the NoteTaker device.

   @size an address where the size will be stored.

*/
notetaker_err_t notetaker_get_data_size(notetaker_t notetaker, size_t *size);

#endif /* NOTETAKER_H */
