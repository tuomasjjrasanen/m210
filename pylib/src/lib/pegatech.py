# -*- coding: utf-8 -*-
# m210
# Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Python API to Pegasus Notetaker devices.

Currently only Pegasus Mobile Notetaker `M210`_ is supported.

.. _M210: http://www.pegatech.com/?CategoryID=207&ArticleID=268

"""

from __future__ import absolute_import
from __future__ import print_function
from __future__ import division

import os
import select
import struct

import m210.hidraw
import m210.collections

__all__ =  [
    "CommunicationError",
    "TimeoutError",
    "M210",
    ]

class CommunicationError(Exception):
    """Raised when an unexpected message is received."""
    pass

class TimeoutError(Exception):
    """Raised when communication timeouts."""
    pass

class TabletSizeError(Exception):
    pass

class TabletOrientationError(Exception):
    pass

class ModeError(Exception):
    pass

class M210(object):
    """
    M210 exposes two interfaces via USB-connection. By default, udev
    creates two hidraw devices to represent these interfaces when the
    device is plugged in. Paths to these devices must be passed to
    initialize a M210-connection. Interface 1 is used for reading and
    writing, interface 2 only for reading.

    Usage example::

      >>> import pegatech
      >>> dev = pegatech.M210(("/dev/hidraw1", "/dev/hidraw2"))
      >>> dev.get_info()
      {'used_memory': 1364, 'firmware_version': 337, 'analog_version': 265, 'pad_version': 32028, 'mode': 'TABLET'}
      >>> download_destination = open("m210notes", "wb")
      >>> dev.download_notes_to(download_destination)
      1364
      >>> download_destination.tell()
      1364
      >>> dev.delete_notes()
      >>> dev.get_info()
      {'used_memory': 0, 'firmware_version': 337, 'analog_version': 265, 'pad_version': 32028, 'mode': 'TABLET'}
    
    """

    _MODE_BYTE_TO_STR_MAP = {'\x01': 'MOUSE', '\x02': 'TABLET'}
    _MODE_STR_TO_BYTE_MAP = {'MOUSE': '\x01', 'TABLET': '\x02'}
    _MODE_BYTE_TO_INDICATOR_MAP = {'\x01': '\x02', '\x02': '\x01'}

    _ORIENTATION_STR_TO_BYTE_MAP = {'NW': '\x01', 'N': '\x00', 'NE': '\x02'}

    _PACKET_PAYLOAD_SIZE = 62

    def __init__(self, hidraw_filepaths, read_timeout=1.0):
        self.read_timeout = read_timeout
        self._fds = []
        for filepath, mode in zip(hidraw_filepaths, (os.O_RDWR, os.O_RDONLY)):
            fd = os.open(filepath, mode)
            devinfo = m210.hidraw.get_devinfo(fd)
            if devinfo != {'product': 257, 'vendor': 3616, 'bustype': 3}:
                raise ValueError('%s is not a M210 hidraw device.' % filepath)
            self._fds.append(fd)

    def _read(self, iface_n):
        fd = self._fds[iface_n]

        rlist, _, _ = select.select([fd], [], [], self.read_timeout)
        if not fd in rlist:
            raise TimeoutError("Reading timeouted.")

        response_size = (64, 9)[iface_n]

        while True:
            response = os.read(fd, response_size)

            if iface_n == 0:
                if response[:2] == '\x80\xb5':
                    continue # Ignore mode button events, at least for now.
            break

        return response

    def _write(self, request):
        request_header = struct.pack('BBB', 0x00, 0x02, len(request))
        os.write(self._fds[0], request_header + request)

    def _wait_ready(self):
        while True:
            self._write('\x95')
            try:
                response = self._read(0)
            except TimeoutError:
                continue
            break

        if (response[:3] != '\x80\xa9\x28'
            or response[9] != '\x0e'):
            raise CommunicationError('Unexpected response to info request: %s'
                                     % response)

        mode_str = M210._MODE_BYTE_TO_STR_MAP[response[10]]

        firmware_ver, analog_ver, pad_ver = struct.unpack('>HHH', response[3:9])

        return {'firmware_version': firmware_ver,
                'analog_version': analog_ver,
                'pad_version': pad_ver,
                'mode': mode_str}

    def _accept_upload(self):
        self._write('\xb6')

    def _reject_upload(self):
        self._write('\xb7')

    def _begin_upload(self):
        """Return packet count."""

        self._write('\xb5')
        try:
            try:
                response = self._read(0)
            except TimeoutError:
                # M210 with zero notes stored in it does not send any response.
                return 0
            if (not response.startswith('\xaa\xaa\xaa\xaa\xaa')
                or response[7:9] != '\x55\x55'):
                raise CommunicationError("Unrecognized upload response: %s"
                                         % response[:9])
            return struct.unpack('>H', response[5:7])[0]
        except Exception, e:
            # If just anything goes wrong, try to leave the device in
            # a decent state by sending a reject request. Then just
            # raise the original exception.
            try:
                self._reject_upload()
            except:
                pass
            raise e

    def set_tablet_settings(self, size, orientation):
        if size not in range(10):
            raise TabletSizeError(size)

        try:
            orient_byte = M210._ORIENTATION_STR_TO_BYTE_MAP[orientation]
        except KeyError:
            raise TabletOrientationError(orientation)

        # Size byte in M210 is counter-intuitive: 0x00 means the largest,
        # 0x09 the smallest. However, in our API, sizes are handled
        # intuitively and therefore needs to be changed here.
        size_byte = chr(abs(size - 9))

        self._wait_ready()
        self._write(''.join(('\x80\xb6', size_byte, orient_byte)))

    def set_mode(self, new_mode):
        """Set the operation mode of the device. 
        Value of `new_mode` should be 'TABLET' or 'MOUSE', ModeError
        is raised otherwise.
        """

        try:
            mode_byte = M210._MODE_STR_TO_BYTE_MAP[new_mode]
        except KeyError:
            raise ModeError(new_mode)

        mode_indicator = M210._MODE_BYTE_TO_INDICATOR_MAP[mode_byte]

        self._wait_ready()
        self._write(''.join(('\x80\xb5', mode_indicator, mode_byte)))

    def get_info(self):
        """Return a dict containing information about versions, current mode
        and the size of stored notes in bytes.
        """

        info = self._wait_ready()
        packet_count = self._begin_upload()
        self._reject_upload()
        info['used_memory'] = packet_count * M210._PACKET_PAYLOAD_SIZE
        return info

    def delete_notes(self):
        """Delete all notes stored in the device."""

        self._wait_ready()
        self._write('\xb0')

    def _receive_packet(self):
        response = self._read(0)
        packet_number = struct.unpack('>H', response[:2])[0]
        return packet_number, response[2:]

    def download_notes_to(self, destination_file):
        """Download notes to an open `destination_file` in one pass.

        Return the total size (bytes) of downloaded notes.
        """

        def request_lost_packets(lost_packet_numbers):
            while lost_packet_numbers:
                lost_packet_number = lost_packet_numbers[0]
                self._write('\xb7' + struct.pack('>H', lost_packet_number))
                packet_number, packet_payload = self._receive_packet()
                if packet_number == lost_packet_number:
                    lost_packet_numbers.remove(lost_packet_number)
                    destination_file.write(packet_payload)

        # Wait until the device is ready to upload packages to
        # us. This is needed because previous public API call might
        # have failed and the device hasn't had enough time to recover
        # from that incident yet.
        self._wait_ready()

        packet_count = self._begin_upload()
        if packet_count == 0:
            # The device does not have any notes, inform that it's ok
            # and let it rest.
            self._reject_upload()
            return 0

        self._accept_upload()
        lost_packet_numbers = m210.collections.OrderedSet()

        # For some odd reason, packet numbering starts from 1 in
        # M210's memory.
        for expected_number in range(1, packet_count + 1):

            packet_number, packet_payload = self._receive_packet()

            if packet_number != expected_number:
                lost_packet_numbers.add(expected_number)

            if not lost_packet_numbers:
                # It's safe to write only when all expected packets so
                # far have been received. The behavior is changed as
                # soon as the first package is lost.
                destination_file.write(packet_payload)

        request_lost_packets(lost_packet_numbers)

        # Thank the device for cooperation and let it rest.
        self._accept_upload()

        return packet_count * M210._PACKET_PAYLOAD_SIZE
