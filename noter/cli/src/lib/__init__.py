import os.path
import sys

def get_connected_devpath_or_exit(daemon):
    connected_devpaths = daemon.connected_devpaths()
    try:
        return connected_devpaths[0]
    except IndexError:
        prog = os.path.basename(sys.argv[0])
        print >>sys.stderr, "%s: error: not connected to any device" % prog
        sys.exit(1)
