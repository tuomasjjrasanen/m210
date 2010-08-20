import os
import os.path

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
