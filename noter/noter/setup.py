# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

modulehidraw = Extension('noter.hidraw',
                         sources=['src/lib/modulehidraw.c'])

moduleinput = Extension('noter.input',
                         sources=['src/lib/moduleinput.c'])

setup(name='noter',
      version='0.1',
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'noter': 'src/lib'},
      packages=['noter'],
      ext_modules=[modulehidraw, moduleinput],
      )
