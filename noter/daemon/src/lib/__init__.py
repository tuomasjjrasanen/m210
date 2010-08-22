from __future__ import absolute_import

# Third-party modules.
import dbus

NAME = 'org.codegrove.noter.daemon'
OBJECT_PATH = '/org/codegrove/noter/daemon'
INTERFACE = 'org.codegrove.noter.daemon'

class Interface(dbus.Interface):

    def __init__(self):
        bus = dbus.SystemBus()
        remote_object = bus.get_object(NAME, OBJECT_PATH)
        dbus.Interface.__init__(self, remote_object, INTERFACE)
