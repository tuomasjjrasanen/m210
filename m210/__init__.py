# -*- coding: utf-8 -*-
# m210 - Python bindings for libm210
# Copyright © 2011 Tuomas Jorma Juhani Räsänen <tuomasjjrasanen@tjjr.fi>

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

"""
Python bindings for libm210.
"""

from __future__ import absolute_import

import ctypes
import os
import os.path
import sys

DESCRIPTION = "Control Pegasus Tablet Mobile NoteTaker (M210)."
VERSION = "0.4"
SO_VERSION = "0"

class _struct_m210_dev_info(ctypes.Structure):
    _fields_ = [("firmware_version", ctypes.c_int16),
                ("analog_version", ctypes.c_int16),
                ("pad_version", ctypes.c_int16),
                ("mode", ctypes.c_int8),
                ("used_memory", ctypes.c_uint32),
                ]

_libm210 = ctypes.CDLL("libm210.so." + SO_VERSION , use_errno=True)

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
        print "%s: %s" % (key.capitalize(), value)

def convert(output_format=OUTPUT_FORMAT_DEFAULT, output_dir=OUTPUT_DIR_DEFAULT,
            overwrite=False, input_file=sys.stdin):
    pass
