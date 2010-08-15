import os
import os.path
import signal
import sys
import syslog

# As defined by glibc:
program_invocation_short_name = os.path.basename(sys.argv[0])

def daemonize():
    signal.signal(signal.SIGINT, signal.SIG_IGN)
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
        except OSError:
            continue
            
    # Close old standard file objects and open new ones. open() is
    # guaranteed to allocate lowest available file descriptor, that
    # means 0, 1, 2 because all file descriptors were just closed.
    sys.stdin.close()
    sys.stdin = open(os.devnull, sys.stdin.mode)
    sys.stdout.close()
    sys.stdout = open(os.devnull, sys.stdout.mode)
    sys.stderr.close()
    sys.stderr = open(os.devnull, sys.stderr.mode)

    syslog.openlog(program_invocation_short_name, syslog.LOG_PID,
                   syslog.LOG_DAEMON)
