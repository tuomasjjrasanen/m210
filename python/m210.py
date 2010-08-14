import select

import hidraw

class CommunicationError(Exception):
    pass

class ModeButtonInterrupt(Exception):
    pass

class TimeoutError(Exception):
    pass

class M210(object):

    def __init__(self, hidraw_filepaths):
        self._files = []
        for filepath, mode in zip(hidraw_filepaths, ('rwb', 'rb')):
            f = open(filepath, mode)
            devinfo = hidraw.get_devinfo(f.fileno())
            if devinfo != {'product': 257, 'vendor': 3616, 'bustype': 3}:
                raise ValueError('%s is not a M210 hidraw device.' % filepath)
            self._files.append(f)

    def _read(self, iface_n):
        f = self._files[iface_n]

        rlist, _, _ = select.select([f], [], [], 1.0)
        if not f in rlist:
            raise TimeoutError("Reading timeouted.")

        response_size = (64, 9)[iface_n]
        response = f.read(response_size)

        if iface_n == 0:
            if response[:2] == '\x80\xb5':
                raise ModeButtonInterrupt()

        return response

    def _write(self, request):
        request_header = struct.pack('BBB', 0x00, 0x02, len(request))
        _, wlist, _ = select.select([], [self._files[0]], [], 1.0)
        if not self._files[0] in wlist:
            raise TimeoutError("Writing timeouted.")
        self._files[0].write(request_header + request)
        self._files[0].flush()

    def _wait_ready(self):
        while True:
            self._write('\x95')
            try:
                response = self._read(0)
            except TimeoutError:
                continue
            break

        if (response[:3] != '\x80\xa9\x28'
            or response[9] != '\x0e'):
            raise CommunicationError('Unexpected response to info request: %s'
                                     % response)

        firmware_ver, analog_ver, pad_ver = struct.unpack('>HHH', response[3:9])

        mode = {'\x01': 'mouse', '\x02': 'tablet'}.get(response[10])

        return {'firmware_version': firmware_ver,
                'analog_version': analog_ver,
                'pad_version': pad_ver,
                'mode': mode}

    def get_info(self):
        return self._wait_ready()
