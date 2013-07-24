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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/hidraw.h>
#include <linux/input.h>  /* BUS_USB */
#include <linux/limits.h> /* PATH_MAX */

#include "libudev.h"

#include "dev.h"

#define M210_DEV_READ_INTERVAL 100000 /* Microseconds. */
#define M210_DEV_RESPONSE_SIZE 64

#define M210_DEV_PACKET_SIZE 62

#define M210_DEV_USB_INTERFACE_COUNT 2

#define M210_DEV_MAX_TIMEOUT_RETRIES 5

struct m210_dev {
	int fds[M210_DEV_USB_INTERFACE_COUNT];
};

struct m210_dev_packet {
	uint16_t num;
	uint8_t data[M210_DEV_PACKET_SIZE];
} __attribute__((packed));

static struct hidraw_devinfo const DEVINFO_M210 = {
	BUS_USB,
	0x0e20,
	0x0101,
};

static enum m210_err m210_dev_write(struct m210_dev const *const dev_ptr,
				    uint8_t const *const bytes,
				    size_t const bytes_size)
{
	enum m210_err err = M210_ERR_OK;
	size_t const request_size = bytes_size + 3;
	uint8_t *request = malloc(request_size);

	if (request == NULL) {
		err = M210_ERR_SYS;
		goto out;
	}

	request[0] = 0x00;     /* Without this, response is not sent. Why?? */
	request[1] = 0x02;     /* report id */
	request[2] = bytes_size;

	/* Copy report paylod to the end of the request. */
	memcpy(request + 3, bytes, bytes_size);

	/* Send request to the interface 0. */
	if (write(dev_ptr->fds[0], request, request_size) == -1) {
		err = M210_ERR_SYS;
		goto out;
	}

out:
	free(request);
	return err;
}

static enum m210_err m210_dev_read(struct m210_dev const *const dev_ptr,
				   int const interface,
				   void *const response,
				   size_t const response_size)
{
	enum m210_err err = M210_ERR_OK;
	fd_set readfds;
	int const fd = dev_ptr->fds[interface];
	static struct timeval select_interval;

	memset(&select_interval, 0, sizeof(struct timeval));
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	select_interval.tv_usec = M210_DEV_READ_INTERVAL;

	switch (select(fd + 1, &readfds, NULL, NULL, &select_interval)) {
	case 0:
		err = M210_ERR_DEV_TIMEOUT;
		goto out;
	case -1:
		err = M210_ERR_SYS;
		goto out;
	default:
		break;
	}

	if (read(fd, response, response_size) == -1) {
		err = M210_ERR_SYS;
		goto out;
	}

out:
	return err;
}

static enum m210_err m210_dev_find_hidraw_devnode(int const iface,
						  char *const path_ptr,
						  size_t const path_size)
{
	enum m210_err err = M210_ERR_OK;
	struct udev_list_entry *list_entry = NULL;
	struct udev_enumerate *enumerate = NULL;
	struct udev *udev = NULL;

	udev = udev_new();
	if (udev == NULL) {
		err = M210_ERR_SYS;
		goto out;
	}

	enumerate = udev_enumerate_new(udev);
	if (enumerate == NULL) {
		err = M210_ERR_SYS;
		goto out;
	}

	if (udev_enumerate_add_match_subsystem(enumerate, "hidraw")) {
		err = M210_ERR_SYS;
		goto out;
	}

	if (udev_enumerate_scan_devices(enumerate)) {
		err = M210_ERR_SYS;
		goto out;
	}

	list_entry = udev_enumerate_get_list_entry(enumerate);

	while (list_entry != NULL) {
		int ifn;
		uint16_t vendor;
		uint16_t product;
		char const *syspath;
		struct udev_device *device;
		char const *devnode;
		struct udev_device *parent;

		syspath =  udev_list_entry_get_name(list_entry);
		device = udev_device_new_from_syspath(udev, syspath);
		devnode = udev_device_get_devnode(device);

		/* Second parent: usb */
		parent = udev_device_get_parent(device);
		parent = udev_device_get_parent(parent);

		ifn = atoi(udev_device_get_sysattr_value(parent,
							 "bInterfaceNumber"));
		parent = udev_device_get_parent(parent);
		vendor = strtol(udev_device_get_sysattr_value(parent,
							      "idVendor"),
				NULL, 16);
		product = strtol(udev_device_get_sysattr_value(parent,
							       "idProduct"),
				 NULL, 16);
		if (vendor == DEVINFO_M210.vendor
		    && product == DEVINFO_M210.product
		    && iface == ifn) {
			strncpy(path_ptr, devnode, path_size);
			udev_device_unref(device);
			goto out;
		}
		list_entry = udev_list_entry_get_next(list_entry);
		udev_device_unref(device);
	}
	err = M210_ERR_NO_DEV;
out:
	if (enumerate) {
		udev_enumerate_unref(enumerate);
	}

	if (udev) {
		udev_unref(udev);
	}

	return err;
}

static enum m210_err m210_dev_accept_download(struct m210_dev const *const dev_ptr)
{
	uint8_t const bytes[] = {0xb6};
	return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

static enum m210_err m210_dev_reject_download(struct m210_dev const *const dev_ptr)
{
	uint8_t const bytes[] = {0xb7};
	return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

static enum m210_err m210_dev_connect_hidraw(struct m210_dev *const dev_ptr,
					     char *const *const hidraw_path_ptrs)
{
	enum m210_err err = M210_ERR_OK;
	int fds[M210_DEV_USB_INTERFACE_COUNT];

	for (size_t i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
		struct hidraw_devinfo devinfo;
		int fd = open(hidraw_path_ptrs[i], O_RDWR);
		if (fd == -1) {
			err = M210_ERR_SYS;
			goto out;
		}

		if (ioctl(fd, HIDIOCGRAWINFO, &devinfo)) {
			err = M210_ERR_SYS;
			goto out;
		}

		if (memcmp(&devinfo, &DEVINFO_M210,
			   sizeof(struct hidraw_devinfo)) != 0) {
			err = M210_ERR_BAD_DEV;
			goto out;
		}

		fds[i] = fd;
	}

out:
	if (!err) {
		memcpy(dev_ptr->fds, fds, sizeof(fds));
	}
	return err;
}

/*
  Download request can be used for two purposes:

  1: Request just packet count.

  HOST		   DEVICE
  =============================
  GET_PACKET_COUNT >
  < PACKET_COUNT
  REJECT	   >

  2: Download packets.

  HOST		    DEVICE
  ==============================
  GET_PACKET_COUNT  >
  < PACKET_COUNT
  ACCEPT	    >
  < PACKET #1
  < PACKET #2
  .
  .
  .
  < PACKET #N
  RESEND #X	    >
  < PACKET #X
  RESEND #Y	    >
  < PACKET #Y
  ACCEPT	    >

*/
static enum m210_err m210_dev_begin_download(struct m210_dev const *const dev_ptr,
					     uint16_t *const packet_count_ptr)
{
	static uint8_t const bytes[] = {0xb5};
	uint8_t response[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	enum m210_err err = M210_ERR_OK;
	int timeout_retries = 0;

	while (timeout_retries < M210_DEV_MAX_TIMEOUT_RETRIES) {
		err = m210_dev_write(dev_ptr, bytes, sizeof(bytes));
		if (err) {
			goto out;
		}
		++timeout_retries;

		err = m210_dev_read(dev_ptr, 0, response, sizeof(response));
		if (err) {
			if (err == M210_ERR_DEV_TIMEOUT) {
				/* In addition to typical reasons for
				 * timeout, M210 device timeouts if
				 * queried the packet count but if it
				 * does not have any notes. By
				 * querying the packet count multiple
				 * times, we ensure that the
				 * timeouting is really due to lack of
				 * notes.
				 */
				continue;
			}
			goto out;
		}

		/* Check that the response is correct. */
		if (response[0] == 0xaa
		    && response[1] == 0xaa
		    && response[2] == 0xaa
		    && response[3] == 0xaa
		    && response[4] == 0xaa
		    && response[7] == 0x55
		    && response[8] == 0x55) {
			break;
		}

	}

	memcpy(packet_count_ptr, response + 5, 2);
	*packet_count_ptr = be16toh(*packet_count_ptr);

out:
	if (err) {
		if (err == M210_ERR_DEV_TIMEOUT) {
			/* M210 has timeouted because it does not have
			 * any notes: its ok, the packet count is set
			 * to zero. */
			err = M210_ERR_OK;
		} else {
			/* Try to leave the device as it was before
			 * the error. */
			m210_dev_reject_download(dev_ptr);
		}
	}
	return err;
}

static enum m210_err m210_dev_read_packet(struct m210_dev const *const dev_ptr,
					  struct m210_dev_packet *const packet_ptr)
{
	enum m210_err err = m210_dev_read(dev_ptr, 0, packet_ptr,
					  sizeof(struct m210_dev_packet));
	if (!err) {
		packet_ptr->num = be16toh(packet_ptr->num);
	}
	return err;
}

enum m210_err m210_dev_connect(struct m210_dev **const dev_ptr_ptr)
{
	struct m210_dev *dev_ptr = NULL;
	enum m210_err err = M210_ERR_OK;
	char iface0_path[PATH_MAX];
	char iface1_path[PATH_MAX];
	char *paths[M210_DEV_USB_INTERFACE_COUNT] = {iface0_path, iface1_path};

	dev_ptr = malloc(sizeof(struct m210_dev));
	if (!dev_ptr) {
		err = M210_ERR_SYS;
		goto out;
	}

	for (size_t i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
		memset(paths[i], 0, PATH_MAX);

		err = m210_dev_find_hidraw_devnode(i, paths[i], PATH_MAX);
		if (err) {
			goto out;
		}
	}
	err = m210_dev_connect_hidraw(dev_ptr, paths);
out:
	if (err) {
		free(dev_ptr);
		dev_ptr = NULL;
	}
	*dev_ptr_ptr = dev_ptr;
	return err;
}

enum m210_err m210_dev_disconnect(struct m210_dev **const dev_ptr_ptr)
{
	struct m210_dev *const dev_ptr = *dev_ptr_ptr;
	enum m210_err err = M210_ERR_OK;

	if (!dev_ptr) {
		goto out;
	}

	for (int i = 0; i < M210_DEV_USB_INTERFACE_COUNT; ++i) {
		if (close(dev_ptr->fds[i]) == -1) {
			err = M210_ERR_SYS;
		}
	}
	free(dev_ptr);
	*dev_ptr_ptr = NULL;
out:
	return err;
}

/*
  Return the total size of notes in bytes. Theoretical maximum size
  is 4063232:

  * Packets are numbered with 16 bit integers.
  => Maximum number of packets: 2**16 = 65536

  * Each packet is 64 bytes wide, last 62 bytes represent bytes in
  memory. The first two bytes represent the packet sequence number.
  => Maximum number of bytes in memory: 2**16 * 62 = 4063232

  * A 32bit integer can address 2**32 different bytes which is way
  more than the maximum number of bytes in devices memory.

*/
static enum m210_err m210_dev_get_notes_size(struct m210_dev const *const dev_ptr,
					     uint32_t *const size_ptr)
{
	uint16_t packet_count = 0;
	enum m210_err err = M210_ERR_OK;

	err = m210_dev_begin_download(dev_ptr, &packet_count);
	if (err) {
		goto out;
	}

	err = m210_dev_reject_download(dev_ptr);
out:
	if (!err) {
		*size_ptr = packet_count * M210_DEV_PACKET_SIZE;
	}
	return err;
}

enum m210_err m210_dev_get_info(struct m210_dev *const dev_ptr,
				struct m210_dev_info *const info_ptr)
{
	enum m210_err err = M210_ERR_OK;
	static uint8_t const bytes[] = {0x95};
	uint8_t response[M210_DEV_RESPONSE_SIZE];
	uint32_t used_memory = 0;

	err = m210_dev_write(dev_ptr, bytes, sizeof(bytes));
	if (err) {
		goto out;
	}

	while (1) {
		memset(response, 0, sizeof(response));
		err = m210_dev_read(dev_ptr, 0, response, sizeof(response));
		if (err) {
			goto out;
		}

		/* Check that the response is correct. */
		if (response[0] == 0x80
		    && response[1] == 0xa9
		    && response[2] == 0x28
		    && response[9] == 0x0e) {
			break;
		}
	}

	err = m210_dev_get_notes_size(dev_ptr, &used_memory);
	if (err) {
		goto out;
	}

	memcpy(&(info_ptr->firmware_version), response + 3, 2);
	memcpy(&(info_ptr->analog_version), response + 5, 2);
	memcpy(&(info_ptr->pad_version), response + 7, 2);

	info_ptr->firmware_version = be16toh(info_ptr->firmware_version);
	info_ptr->analog_version = be16toh(info_ptr->analog_version);
	info_ptr->pad_version = be16toh(info_ptr->pad_version);
	info_ptr->mode = response[10];
	info_ptr->used_memory = used_memory;

out:
	return err;
}

enum m210_err m210_dev_delete_notes(struct m210_dev *const dev_ptr)
{
	uint8_t const bytes[] = {0xb0};
	return m210_dev_write(dev_ptr, bytes, sizeof(bytes));
}

static enum m210_err m210_dev_download(struct m210_dev const *const dev_ptr,
				       uint16_t packet_count,
				       uint16_t *const lost_nums,
				       FILE *const file)
{
	enum m210_err err = M210_ERR_OK;
	uint16_t lost_count = 0;

	for (int i = 0; i < packet_count; ++i) {
		struct m210_dev_packet packet;
		uint16_t const expected_packet_number = i + 1;

		err = m210_dev_read_packet(dev_ptr, &packet);
		if (err) {
			goto out;
		}

		if (packet.num != expected_packet_number) {
			lost_nums[lost_count++] = expected_packet_number;
		}

		if (!lost_count) {
			if (fwrite(packet.data, sizeof(packet.data), 1,
				   file) != 1) {
				err = M210_ERR_SYS;
				goto out;
			}
		}
	}

	while (lost_count > 0) {
		struct m210_dev_packet packet;

		uint8_t resend_request[] = {0xb7, 0x00};
		resend_request[1] = htobe16(lost_nums[0]);

		err = m210_dev_write(dev_ptr, resend_request,
				     sizeof(resend_request));
		if (err) {
			goto out;
		}

		err = m210_dev_read_packet(dev_ptr, &packet);
		if (err) {
			goto out;
		}

		if (packet.num == lost_nums[0]) {
			lost_nums[0] = lost_nums[--lost_count];
			if (fwrite(packet.data, sizeof(packet.data), 1,
				   file) != 1) {
				err = M210_ERR_SYS;
				goto out;
			}
		}
	}
out:
	return err;
}

enum m210_err m210_dev_download_notes(struct m210_dev *const dev_ptr, FILE *file)
{
	enum m210_err err = M210_ERR_OK;
	uint16_t *lost_nums = NULL;
	uint16_t packet_count = 0;

	err = m210_dev_begin_download(dev_ptr, &packet_count);
	if (err) {
		goto out;
	}

	if (packet_count == 0) {
		err = m210_dev_reject_download(dev_ptr);
		goto out;
	}

	lost_nums = calloc(packet_count, sizeof(uint16_t));
	if (lost_nums == NULL) {
		int const original_errno = errno;
		m210_dev_reject_download(dev_ptr);
		errno = original_errno;
		err = M210_ERR_SYS;
		goto out;
	}

	err = m210_dev_accept_download(dev_ptr);
	if (err) {
		goto out;
	}

	err = m210_dev_download(dev_ptr, packet_count, lost_nums, file);
	if (err) {
		goto out;
	}

	/*
	  All packets have been received, time to thank the device for
	  cooperation.
	*/
	err = m210_dev_accept_download(dev_ptr);
out:
	if (file) {
		fflush(file);
	}
	free(lost_nums);
	return err;
}
