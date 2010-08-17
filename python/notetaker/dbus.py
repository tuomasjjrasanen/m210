from __future__ import absolute_import

# Third-party modules.
import dbus
import dbus.service
import dbus.mainloop.glib

dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

SERVICE_NAME = 'org.codegrove.notetaker'
DAEMON_INTERFACE_NAME = 'org.codegrove.notetaker.daemon'

class Daemon(dbus.service.Object):

    OBJECT_PATH = '/org/codegrove/notetaer/daemon'

    def __init__(self, bus):
        dbus.service.Object.__init__(self, bus, Daemon.OBJECT_PATH)

    @dbus.service.method(DAEMON_INTERFACE_NAME, in_signature='s', out_signature='s')
    def hello(self, name):
        return 'Hola %s!' % name

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
