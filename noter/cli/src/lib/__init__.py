def _print_devices(status, dirpaths):
    for dirpath in dirpaths:
        print status, dirpath

def print_disconnected_devices(dirpaths):
    return _print_devices("D", dirpaths)

def print_connected_devices(dirpaths):
    return _print_devices("C", dirpaths)
