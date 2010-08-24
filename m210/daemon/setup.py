# -*- coding: utf-8 -*-

from distutils.core import setup

import m210

setup(name='m210-daemon',
      version=m210.VERSION,
      author='Tuomas Räsänen',
      author_email='tuos@codegrove.org',
      package_dir={'m210.daemon': 'src/lib'},
      packages=['m210.daemon'],
      scripts=[
        'src/bin/m210-daemon',
        ],
      data_files=[
        ('/etc/dbus-1/system.d',
         [
                'etc/dbus-1/system.d/org.codegrove.m210.daemon.conf',
                ],
         ),
        ('/etc/init',
         [
                'etc/init/m210-daemon.conf',
                ],
         ),
        ('/etc/udev/rules.d',
         [
                'etc/udev/rules.d/50-m210-daemon.rules',
                ],
         ),
        ],
      )
