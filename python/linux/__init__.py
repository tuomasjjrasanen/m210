# -*- coding: utf-8 -*-
"""
Various helpers and wrappers for Linux programming.

Modules:
linux
linux.hidraw
linux.input

This module contains:
linux.PROGRAM_INVOCATION_SHORT_NAME
linux.daemonize()

Author: Tuomas (tuos) Räsänen <tuos@codegrove.org>
"""

import errno
import os
import os.path
import signal
import sys
import syslog

__all__ = [
    'PROGRAM_INVOCATION_SHORT_NAME',
    'daemonize',
    ]

# As defined by glibc:
PROGRAM_INVOCATION_SHORT_NAME = os.path.basename(sys.argv[0])

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
        syslog.openlog(PROGRAM_INVOCATION_SHORT_NAME, syslog_options,
                       syslog.LOG_DAEMON)
