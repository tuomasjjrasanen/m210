# -*- coding: utf-8 -*-
import os

from distutils.core import setup, Extension, Command
from distutils.command.install import install

modulehidraw = Extension('linux.hidraw',
                         sources=['linux/modulehidraw.c'])

moduleinput = Extension('linux.input',
                         sources=['linux/moduleinput.c'])

class recording_install(install):

    def initialize_options(self):
        install.initialize_options(self)
        self.record = '.installed_files'

class uninstall(Command):
    description = 'uninstall all previously installed files'
    user_options = [('record=', None,
                     'filename containing a list of installed files, default=.installed_files'),
                    ]

    def initialize_options(self):
        self.record = '.installed_files'

    def finalize_options(self):
        pass

    def run(self):
        for installed_filename in open(self.record):
            installed_filename = installed_filename.strip()
            if self.dry_run:
                print('Would remove %s' % installed_filename)
            else:
                print('Removing %s' % installed_filename)
                os.system('rm %s' % installed_filename)

setup(name='noter',
      version='0.1',
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      packages=['linux', 'noter'],
      scripts=[
        'bin/noter-daemon',
        'bin/noter-connect',
        'bin/noter-disconnect',
        ],
      data_files=[
        ('/etc/dbus-1/system.d',
         [
                'etc/dbus-1/system.d/org.codegrove.noter.conf',
                ],
         ),
        ('/etc/init',
         [
                'etc/init/noter-daemon.conf',
                ],
         ),
        ('/etc/udev/rules.d',
         [
                'etc/udev/rules.d/50-noter.rules',
                ],
         ),
        ],
      ext_modules=[modulehidraw, moduleinput],
      cmdclass={
        'install': recording_install,
        'uninstall': uninstall,
        },
      )
