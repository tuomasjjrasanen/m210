# -*- coding: utf-8 -*-

from distutils.core import setup

import m210

setup(name='m210-cli',
      version=m210.VERSION,
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'m210.cli': 'src/lib'},
      packages=['m210.cli'],
      scripts=[
        'src/bin/m210-connect',
        'src/bin/m210-disconnect',
        'src/bin/m210-download',
        'src/bin/m210-info',
        'src/bin/m210-erase',
        ],
      )
