# -*- coding: utf-8 -*-

from distutils.core import setup

import noter

setup(name='noter-cli',
      version=noter.VERSION,
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      scripts=[
        'src/bin/noter-connect',
        'src/bin/noter-disconnect',
        'src/bin/noter-download',
        'src/bin/noter-info',
        'src/bin/noter-erase',
        ],
      )
