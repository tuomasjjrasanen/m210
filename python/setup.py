from distutils.core import setup, Extension

modulehidraw = Extension('hidraw',
                         sources=['modulehidraw.c'])

setup(name='python-hidraw',
      version='0.1',
      py_modules=['m210'],
      ext_modules=[modulehidraw])
