from distutils.core import setup, Extension

modulehidraw = Extension('linux.hidraw',
                         sources=['linux/modulehidraw.c'])

moduleinput = Extension('linux.input',
                         sources=['linux/moduleinput.c'])

setup(name='notetaker',
      version='0.1',
      py_modules=['peganotes'],
      packages=['linux'],
      ext_modules=[modulehidraw, moduleinput])
