# -*- coding: utf-8 -*-
from distutils.core import setup, Extension

modulehidraw = Extension('linux.hidraw',
                         sources=['linux/modulehidraw.c'])

moduleinput = Extension('linux.input',
                         sources=['linux/moduleinput.c'])

setup(name='notetaker',
      version='0.1',
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      py_modules=['peganotes'],
      packages=['linux'],
      scripts=['notetakerd'],
      ext_modules=[modulehidraw, moduleinput])
