# -*- coding: utf-8 -*-

import errno
import os
import os.path

from distutils.command.build_py import build_py as _build_py
from distutils.core import setup

import m210

class build_py(_build_py):
    """Generate Python-modules from .ui files."""

    def run(self):
        try:
            filenames = os.listdir('ui')
        except OSError, e:
            if e.errno == errno.ENOENT:
                # ui directory does not exists. fine, no ui-files then.
                filenames = []
            raise e # Something else went wrong during listing, panic!

        # ui/*.ui -> src/lib/ui/*.py
        for filename in os.listdir('ui'):
            filename = os.path.basename(filename)
            name, ext = os.path.splitext(filename)
            if ext != '.ui':
                continue
            src_path = 'ui/%s' % filename
            source_mtime = os.stat(src_path).st_mtime
            dest_path = 'src/lib/ui/%s.py' % name
            if not os.path.exists(dest_path):
                dest_mtime = -1
            else:
                dest_mtime = os.stat(dest_path).st_mtime
            if source_mtime > dest_mtime:
                os.system('pyuic4 -o src/lib/ui/%s.py ui/%s' % (name, filename))

        return _build_py.run(self) # Proceed normally.

setup(name='m210-qt',
      version=m210.VERSION,
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={
        'm210.qt': 'src/lib',
        },
      packages=[
        'm210.qt',
        'm210.qt.ui',
        ],
      scripts=[
        'src/bin/m210-qt',
        ],
      cmdclass={
        'build_py': build_py,
        },
      )
