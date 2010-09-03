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

from __future__ import absolute_import
from __future__ import print_function
from __future__ import division

# Third-party modules.
import dbus

NAME = 'org.codegrove.m210.daemon'
OBJECT_PATH = '/org/codegrove/m210/daemon'
INTERFACE = 'org.codegrove.m210.daemon'

class AlreadyConnectedError(Exception):
    pass

class NotConnectedError(Exception):
    pass

class UnknownDeviceError(Exception):
    pass

class Interface(dbus.Interface):

    def __init__(self):
        bus = dbus.SystemBus()
        remote_object = bus.get_object(NAME, OBJECT_PATH)
        dbus.Interface.__init__(self, remote_object, INTERFACE)
