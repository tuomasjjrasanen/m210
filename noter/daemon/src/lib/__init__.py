from __future__ import absolute_import

import errno
import os
import os.path
import signal
import sys
import syslog

# Third-party modules.
import dbus

def daemonize(syslog_options=syslog.LOG_PID | syslog.LOG_CONS):
    """Summon a quite proper Linux daemon process quite properly.

    1. Fork and exit parent.

    2. Create a new session.

    3. Ignore SIGHUP.

    4. Fork and exit parent again.

    5. Clear umask.

    6. Change current directory to /.

    7. Close all file descriptors.

    8. Open new files for sys.stdin, sys.stdout and sys.stderr.

    9. Redirect them to /dev/null.

    10. Optionally open syslog if syslog_options is not None. Default
    syslog_options is syslog.LOG_PID | syslog.LOG_CONS.
 
    """

    if os.fork():
        os._exit(0)
    else:
        os.setsid()
        signal.signal(signal.SIGHUP, signal.SIG_IGN)
        if os.fork():
            os._exit(0)
    os.umask(0)
    os.chdir('/')

    # Close all file descriptors inherited from the parent process.
    for i in range(os.sysconf('SC_OPEN_MAX')):
        try:
            os.close(i)
        except OSError, e:
            if e.errno == errno.EBADF:
                continue # Ignore if the file descriptor does not exists.
            raise e # Other exceptions shall fly through.
            
    # Close old standard file objects and open new ones. open() is
    # guaranteed to allocate lowest available file descriptor, that
    # means 0, 1, 2 because all file descriptors were just closed.
    sys.stdin.close()
    sys.stdin = open(os.devnull, sys.stdin.mode)
    sys.stdout.close()
    sys.stdout = open(os.devnull, sys.stdout.mode)
    sys.stderr.close()
    sys.stderr = open(os.devnull, sys.stderr.mode)

    if syslog_options is not None:
        syslog.openlog(os.path.basename(sys.argv[0]), syslog_options,
                       syslog.LOG_DAEMON)

NAME = 'org.codegrove.noter.daemon'
OBJECT_PATH = '/org/codegrove/noter/daemon'
INTERFACE = 'org.codegrove.noter.daemon'

class Interface(dbus.Interface):

    def __init__(self):
        bus = dbus.SystemBus()
        remote_object = bus.get_object(NAME, OBJECT_PATH)
        dbus.Interface.__init__(self, remote_object, INTERFACE)

    def disconnected_devices(self):
        connected_devs = self.connected_devices()
        for dirpath in find_m210_dirpaths():
            if dirpath not in connected_devs:
                yield dirpath

def find_m210_dirpaths():
    find_cmd = r"""
find /sys/devices/pci0000\:00 -type d \
-execdir grep -q 0e20 '{}'/idVendor \; \
-execdir grep -q 0101 '{}'/idProduct \; \
-print 2>/dev/null
"""
    
    return os.popen(find_cmd).read().strip().splitlines()

def udev_root():
    return os.popen("udevadm info --root").read().strip().splitlines()[0]

def find_m210_paths(dirpath):
    find_cmd = r"""
find %s -iregex .*%s[0-9][0-9]*$ | sed 's/.*\(%s[0-9][0-9]*\)/\1/'
"""
    
    find_hidraw = find_cmd % (dirpath, "hidraw", "hidraw")
    hidraw_names = os.popen(find_hidraw).read().strip().splitlines()
    hidraw_paths = [os.path.join(udev_root(), name) for name in hidraw_names]
    
    find_event = find_cmd % (dirpath, "event", "event")
    event_name = os.popen(find_event).read().strip().splitlines()[0]
    event_path = os.path.join(udev_root(), "input", event_name)
    
    return hidraw_paths[0], hidraw_paths[1], event_path
