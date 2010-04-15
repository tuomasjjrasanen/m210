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

#define NOTETAKER_ERRNO_UNKNOWN_DEVICE 2

struct notetaker_version_info {
    uint16_t firmware_version;
    uint16_t analog_version;
    uint16_t pad_version;
};

typedef struct notetaker notetaker_t;

notetaker_t *notetaker_open(char **hidraw_paths, int *notetaker_errno);
int notetaker_close(notetaker_t *notetaker);
int notetaker_get_version_info(notetaker_t *notetaker,
                               struct notetaker_version_info *version_info);

#endif /* NOTETAKER_H */
