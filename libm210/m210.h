#ifndef M210_H
#define M210_H

#define M210_IFACE_COUNT 2

struct m210_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
    uint8_t mode;
};

enum m210_err {
    err_ok,
    err_sys,
    err_baddev,
    err_nodev,
    err_badmsg
};

/**
   An opaque object representing a M210 device.
*/
typedef struct m210* m210_t;

typedef enum m210_err m210_err_t;

/**
   Open a hid connetion to a M210 device and return an object
   representing it.

   Succesfully opened device must be freed with m210_free().

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_nodev - a M210 device was not found

   - err_baddev - hidraw_paths did not represent interfaces 0 and 1 of
   a M210 device.

   @m210 an address of an object representing the M210 device.

   @hidraw_paths a list of two null-terminated paths of hidraw device
   nodes for interface 0 and 1 respectively or NULL in which case
   proper nodes are searched with udev.

*/
m210_err_t m210_open(m210_t *m210, char** hidraw_paths);

/**
   Close the M210 device connection and free all resources.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set
   appropriately. Memory is not freed!

   @m210 an object represeting the M210 device.

*/
m210_err_t m210_free(m210_t m210);

/**
   Return information about the M210 device.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @m210 an object represeting the M210 device.

   @info an address where the info will be stored.

*/
m210_err_t m210_get_info(m210_t m210, struct m210_info *info);

/**
   Requests the M210 device to delete all notes.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   @m210 an object represeting the M210 device.

*/
m210_err_t m210_delete_notes(m210_t m210);

/**
   Return size of stored notes.

   Return values:

   - err_ok - success

   - err_sys - system call failed and errno is set appropriately

   - err_badmsg - device sent unexpected message

   @m210 an object represeting the M210 device.

   @size an address where the size will be stored.

*/
m210_err_t m210_get_data_size(m210_t m210, size_t *size);

#endif /* M210_H */
