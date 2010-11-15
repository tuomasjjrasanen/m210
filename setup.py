# -*- coding: utf-8 -*-
# m210
# Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from distutils.core import setup, Extension

modulehidraw = Extension('m210.hidraw',
                         sources=['python-m210/m210/modulehidraw.c'])

moduleinput = Extension('m210.input',
                        sources=['python-m210/m210/moduleinput.c'])

setup(name='m210',
      provides=[
        'm210',
        ],
      requires=[
        'PyQt4',
        'dbus',
        ],
      version='0.4',
      author=u'Tuomas Räsänen (tuos)',
      author_email='tuos@codegrove.org',
      maintainer=u'Tuomas Räsänen (tuos)',
      maintainer_email='tuos@codegrove.org',
      url="http://codegrove.org/projects/m210",
      download_url="http://pypi.python.org/packages/source/m/m210/m210-0.4.tar.gz",
      description="Tools for Pegasus Mobile Notetaker M210",
      long_description="""M210 is a software project which aims to provide easy-to-use tools for
controlling and using Pegasus Mobile Notetaker M210 in modern Linux
desktop environments.""",
      keywords=[
        "pegasus",
        "notetaker",
        "m210",
        "gui",
        "daemon",
        "notes",
        "digital pen",
        "pen",
        "irisnotes",
        "qt",
        ],
      platforms=[
        "Operating System :: POSIX :: Linux",
        ],
      license="License :: OSI Approved :: GNU General Public License (GPL)",
      classifiers=[
        "Intended Audience :: Developers",
        "Intended Audience :: End Users/Desktop",
        "Development Status :: 3 - Alpha",
        "Environment :: Console",
        "Environment :: No Input/Output (Daemon)",
        "Environment :: X11 Applications :: Qt",
        "License :: OSI Approved :: GNU General Public License (GPL)",
        "Natural Language :: English",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C",
        "Programming Language :: Python :: 2.6",
        "Topic :: Desktop Environment",
        "Topic :: Documentation",
        "Topic :: Education",
        ],
      package_dir={
        'm210':           'python-m210/m210',
        'm210.qt':        'm210c-qt/src/lib',
        },
      packages=[
        'm210',
        'm210.qt',
        'm210.qt.ui',
        ],
      scripts=[
        'm210d/m210d',
        'm210c/m210c',
        'm210c-qt/src/bin/m210c-qt',
        ],
      data_files=[
        ('/etc/dbus-1/system.d',
         [
                'm210d/etc/dbus-1/system.d/org.codegrove.m210.daemon.conf',
                ],
         ),
        ('/etc/init',
         [
                'm210d/etc/init/m210d.conf',
                ],
         ),
        ('/etc/udev/rules.d',
         [
                'm210d/etc/udev/rules.d/50-m210d.rules',
                ],
         ),
        ('/usr/share/applications',
         [
                'm210c-qt/share/applications/m210c-qt.desktop',
                ],
         ),
        ('/usr/share/pixmaps',
         [
                'm210c-qt/share/pixmaps/m210c-qt.png',
                ],
         ),
        ],
      ext_modules=[modulehidraw, moduleinput],
      )
