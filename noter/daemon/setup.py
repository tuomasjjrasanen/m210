# -*- coding: utf-8 -*-

from distutils.core import setup

import noter

setup(name='noter-daemon',
      version=noter.VERSION,
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'noter.daemon': 'src/lib'},
      packages=['noter.daemon'],
      scripts=[
        'src/bin/noter-daemon',
        ],
      data_files=[
        ('/etc/dbus-1/system.d',
         [
                'etc/dbus-1/system.d/org.codegrove.noter.daemon.conf',
                ],
         ),
        ('/etc/init',
         [
                'etc/init/noter-daemon.conf',
                ],
         ),
        ('/etc/udev/rules.d',
         [
                'etc/udev/rules.d/50-noter-daemon.rules',
                ],
         ),
        ],
      )
