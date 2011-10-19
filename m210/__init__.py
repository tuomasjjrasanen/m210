# -*- coding: utf-8 -*-
# Copyright © 2011 Tuomas Jorma Juhani Räsänen

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import absolute_import
from __future__ import print_function

import ctypes
import errno
import os
import os.path
import struct
import sys

DESCRIPTION = "Pegasus Tablet Mobile NoteTaker (M210) Controller"
AUTHOR_NAME = u"Tuomas Jorma Juhani Räsänen"
AUTHOR_EMAIL = "tuomasjjrasanen@tjjr.fi"
AUTHOR = "%s <%s>" % (AUTHOR_NAME, AUTHOR_EMAIL)
VERSION = "0.4"
SO_VERSION = "0"
SO_NAME = "libm210.so." + SO_VERSION
HOMEPAGE = "http://tjjr.fi/software/m210/"
__doc__ = """%s

This Python package provides pythonic bindings for %s. It provides API
for displaying device information, dumping stored notes as a binary
stream to a file and converting notes to various image formats.

Author: %s
Version: %s
Homepage: %s
""" % (DESCRIPTION, SO_NAME, AUTHOR, VERSION, HOMEPAGE)

class _struct_m210_dev_info(ctypes.Structure):
    _fields_ = [("firmware_version", ctypes.c_int16),
                ("analog_version", ctypes.c_int16),
                ("pad_version", ctypes.c_int16),
                ("mode", ctypes.c_int8),
                ("used_memory", ctypes.c_uint32),
                ]

_libm210 = ctypes.CDLL(SO_NAME , use_errno=True)

(_M210_ERR_OK,
 _M210_ERR_SYS,
 _M210_ERR_BAD_DEV,
 _M210_ERR_NO_DEV,
 _M210_ERR_BAD_DEV_MSG,
 _M210_ERR_DEV_TIMEOUT,
 _M210_ERR_BAD_NOTE_HEAD,
 _M210_ERR_BAD_NOTE_BODY,
 _M210_ERR_NOTE_EOF) = range(9)

class Error(Exception):
    pass

def _error_handler(result, fn, args):
    if result == _M210_ERR_SYS:
        code = ctypes.get_errno()
        raise OSError(code, os.strerror(code))
    elif result > _M210_ERR_SYS:
        raise Error(_libm210.m210_err_strerror(result))
    return result

_libm210.m210_dev_connect.errcheck = _error_handler
_libm210.m210_dev_disconnect.errcheck = _error_handler
_libm210.m210_dev_get_info.errcheck = _error_handler
_libm210.m210_dev_download_notes.errcheck = _error_handler
_libm210.m210_dev_delete_notes.errcheck = _error_handler
_libm210.m210_err_strerror.restype = ctypes.c_char_p

class Connection(object):

    """Connect to a m210 device.
    """

    def __init__(self):
        self.__dev_p = ctypes.c_void_p()
        self.__dev_info = _struct_m210_dev_info()

        _libm210.m210_dev_connect(ctypes.pointer(self.__dev_p))
        _libm210.m210_dev_get_info(self.__dev_p, ctypes.pointer(self.__dev_info))

    def device_info(self):
        return {
            "firmware_version": self.__dev_info.firmware_version,
            "analog_version": self.__dev_info.analog_version,
            "pad_version": self.__dev_info.pad_version,
            "mode": self.__dev_info.mode,
            "used_memory": self.__dev_info.used_memory
            }

    def download_notes(self, dest_file):
        _libm210.m210_dev_download_notes(self.__dev_p, dest_file.fileno())

    def delete_notes(self):
        _libm210.m210_dev_delete_notes(self.__dev_p)        

    def __del__(self):
        _libm210.m210_dev_disconnect(ctypes.pointer(self.__dev_p))

class InputFormatError(Error):
    pass

class OutputFormatError(Error):
    pass

def _iter_pen_paths(notes_file):

    class counting_read(object):

        def __init__(self, f):
            self.f = f
            self.fpos = 0

        def __call__(self, byte_count, *args, **kwargs):
            bytes = self.f.read(byte_count)
            self.fpos += len(bytes)
            return bytes

    notes_file_read = counting_read(notes_file)

    while True:

        # BEGIN HEADER.
        next_header_bytes = notes_file_read(3)
        if not next_header_bytes or next_header_bytes[0] == '\x00':
            break
        
        next_header_pos = struct.unpack("<I", next_header_bytes + "\x00")[0]

        state, note_num, note_max_num = struct.unpack("BBB", notes_file_read(3))
        if note_num > note_max_num:
            raise InputFormatError("Note number is bigger than the total number of notes.")
        notes_file_read(8) # Reserved for some unknown usage, perhaps timestamp?
        # END HEADER.

        # BEGIN DATA.
        pen_paths = []
        pen_path = []
        while notes_file_read.fpos < next_header_pos:
            data = notes_file_read(4)
            if data == "\x00\x00\x00\x80":
                pen_paths.append(pen_path)
                pen_path = []
            else:
                point = struct.unpack("<hh", data)
                pen_path.append(point)
        if pen_path:
            pen_paths.append(pen_path)
        # END DATA.

        yield pen_paths

INPUT_FILE_DEFAULT = sys.stdin
OUTPUT_DIR_DEFAULT = os.path.abspath(".")
OUTPUT_FILE_DEFAULT = sys.stdout
OUTPUT_FORMAT_DEFAULT = "svg"

def dump(output_file=OUTPUT_FILE_DEFAULT):
    connection = Connection()
    connection.download_notes(output_file)

def info():
    connection = Connection()
    for key, value in sorted(connection.device_info().items()):
        print("%s: %s" % (key.capitalize(), value))

def _format_svg(pen_paths):
    svg_polylines = []
    for pen_path in pen_paths:
        points = " ".join(["%d,%d" % (x, y) for x, y in pen_path])
        svg_polylines.append('<polyline points="%s" stroke-width="30" stroke="black" fill="none"/>' % points)
    return """<?xml version="1.0"?>
    <!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
    <svg width="210mm" height="297mm" viewBox="-7000 500 14000 20000" xmlns="http://www.w3.org/2000/svg" version="1.1">
        %s
    </svg>
    """ % "\n    ".join(svg_polylines)

_OUTPUT_FORMATTERS = {
    "svg": _format_svg,
    }

def _open_output_file(output_dir, n, output_format, overwrite):
    open_flags = os.O_CREAT | os.O_WRONLY
    if not overwrite:
        open_flags |= os.O_EXCL

    number_of_copies = 0

    while True:
        if number_of_copies:
            extra = ".%d" % number_of_copies
        else:
            extra = ""

        output_filepath = os.path.join(output_dir, "m210_note_%d%s.%s" % (n, extra, output_format))
        old_umask = os.umask(0133)
        try:
            return os.open(output_filepath, open_flags)
        except OSError, e:
            if e.errno == errno.EEXIST:
                number_of_copies += 1
                continue
            raise e
        finally:
            os.umask(old_umask)

def convert(output_format=OUTPUT_FORMAT_DEFAULT, output_dir=OUTPUT_DIR_DEFAULT,
            overwrite=False, input_file=sys.stdin):
    try:
        os.mkdir(output_dir)
    except OSError, e:
        if e.errno != errno.EEXIST:
            raise e

    try:
        output_formatter = _OUTPUT_FORMATTERS[output_format]
    except KeyError:
        raise OutputFormatError("Unknown output format: %r" % output_format)

    for n, pen_paths in enumerate(_iter_pen_paths(input_file), 1):

        fd = _open_output_file(output_dir, n, output_format, overwrite)
        try:
            os.write(fd, output_formatter(pen_paths))
        finally:
            os.close(fd)
