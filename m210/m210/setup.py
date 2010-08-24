# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

modulehidraw = Extension('m210.hidraw',
                         sources=['src/lib/modulehidraw.c'])

moduleinput = Extension('m210.input',
                         sources=['src/lib/moduleinput.c'])

setup(name='m210',
      version='0.1',
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'m210': 'src/lib'},
      packages=['m210'],
      ext_modules=[modulehidraw, moduleinput],
      )
