# -*- coding: utf-8 -*-

from distutils.core import setup

setup(name='noter-core',
      version='0.1',
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'noter': 'src/lib'},
      packages=['noter', 'noter.core'],
      )
