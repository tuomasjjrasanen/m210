from __future__ import absolute_import

import os.path
import syslog

# Third-party modules.
import dbus
import dbus.service
import dbus.mainloop.glib

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

import linux.input
import pegatech

SERVICE_NAME = 'org.codegrove.notetaker'
DAEMON_INTERFACE_NAME = 'org.codegrove.notetaker.daemon'

class M210Connection(object):

    def __init__(self, hidraw0_path, hidraw1_path, event_path):
        self._event_fd = None
        self._event_fd = os.open(event_path, os.O_RDWR)
        linux.input.grab_event_device(self._event_fd)
        self.m210 = pegatech.M210((hidraw0_path, hidraw1_path))

    def __del__(self):
        if self._event_fd is not None:
            linux.input.release_event_device(self._event_fd)

class Daemon(dbus.service.Object):

    OBJECT_PATH = '/org/codegrove/notetaker/daemon'

    def __init__(self, bus):
        dbus.service.Object.__init__(self, bus, Daemon.OBJECT_PATH)
        self._m210_connections = []
        self._triplet = [None, None, None]

    @dbus.service.method(DAEMON_INTERFACE_NAME, in_signature='iss', out_signature='')
    def m210_connected(self, index, udev_root, kernel_name):
        self._triplet[index] = os.path.join(udev_root, kernel_name)
        if not None in self._triplet:
            self._m210_connections.append(M210Connection(*self._triplet))
            self._triplet = [None, None, None]

def attach():
    global _name
    global _dbus_objects

    # Integrate to glib-mainloop. For some reason, DBusQtMainLoop does not work.
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    # Open a bus.
    bus = dbus.SystemBus()

    # Register a well-known name for the service.
    _name = dbus.service.BusName(SERVICE_NAME, bus)

    # Export objects through the bus.
    _dbus_objects = []
    _dbus_objects.append(Daemon(bus))
