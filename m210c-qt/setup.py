# -*- coding: utf-8 -*-
# m210c-qt
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

from distutils.core import setup

setup(name='m210c-qt',
      version='0.4',
      author=u'Tuomas Räsänen (tuos)',
      author_email='tuos@codegrove.org',
      url="http://codegrove.org/projects/m210",
      description="Qt client for m210d.",
      long_description="""M210 is a software project which aims to provide easy-to-use tools for
controlling and using Pegasus Mobile Notetaker M210 in modern Linux
desktop environments.""",
      license="GPLv3+",
      platforms=[
        "Linux",
        ],
      package_dir={
        'm210.qt': 'src/lib',
        },
      packages=[
        'm210.qt',
        'm210.qt.ui',
        ],
      scripts=[
        'src/bin/m210c-qt',
        ],
      data_files=[
        ('/usr/share/applications',
         [
                'share/applications/m210c-qt.desktop',
                ],
         ),
        ('/usr/share/pixmaps',
         [
                'share/pixmaps/m210c-qt.png',
                ],
         ),
        ],
      )
