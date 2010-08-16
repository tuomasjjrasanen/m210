# -*- coding: utf-8 -*-
from __future__ import absolute_import

# Standard modules.
import optparse
import sys

# Third-party modules.
import dbus
import dbus.service
import dbus.mainloop.glib

import PyQt4.QtCore

# Project-specific modules.
import linux

DBUS_SERVICE_NAME = 'org.codegrove.notetaker'

class DaemonDBusObject(dbus.service.Object):

    def __init__(self, bus):
        dbus.service.Object.__init__(self, bus, '/daemon')

    @dbus.service.method(DBUS_SERVICE_NAME, in_signature='s', out_signature='s')
    def hello(self, name):
        return 'Hola %s!' % name

    @dbus.service.method(DBUS_SERVICE_NAME, in_signature='', out_signature='')
    def exit(self):
        PyQt4.QtCore.QCoreApplication.quit()

def attach_to_dbus():
    global _bus_name
    global _dbus_objects

    # Integrate to glib-mainloop. For some reason, DBusQtMainLoop does not work.
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    # Open a bus.
    bus = dbus.SessionBus()

    # Register a well-known name for the bus.
    _bus_name = dbus.service.BusName(DBUS_SERVICE_NAME, bus)

    # Export objects through the bus.
    _dbus_objects = []
    _dbus_objects.append(DaemonDBusObject(bus))

def parse_args():
    parser = optparse.OptionParser(version='''%s 0.1
Copyright (C) 2010 Tuomas (tuos) Räsänen <tuos@codegrove.org>
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

Written by Tuomas Räsänen.''' % (linux.PROGRAM_INVOCATION_SHORT_NAME))
    parser.add_option('', '--no-daemon', action='store_false',
                      default=True, dest='daemon',
                      help='do not run %s as a daemon'
                      % linux.PROGRAM_INVOCATION_SHORT_NAME)
    return parser.parse_args(sys.argv)

def main():
    options, args = parse_args()

    if options.daemon:
        linux.daemonize()

    attach_to_dbus()

    app = PyQt4.QtCore.QCoreApplication([])
    app.exec_()

if __name__ == '__main__':
    main()
