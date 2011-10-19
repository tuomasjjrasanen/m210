# -*- coding: utf-8 -*-
# Copyright © 2011 Tuomas Jorma Juhani Räsänen

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

import m210

setup(name='m210',
      provides=[
        'm210',
        ],
      version=m210.VERSION,
      author=m210.AUTHOR_NAME,
      author_email=m210.AUTHOR_EMAIL,
      maintainer=m210.AUTHOR_NAME,
      maintainer_email=m210.AUTHOR_EMAIL,
      url=m210.HOMEPAGE,
      download_url="http://launchpad.net/m210/trunk/%s/+download/m210-%s.tar.gz" % (m210.VERSION, m210.VERSION),
      description=m210.DESCRIPTION,
      long_description="""M210 provides tools for controlling Pegasus Tablet Mobile Notetaker M210
in Linux systems.""",
      keywords=[
        "pegasus",
        "notetaker",
        "m210",
        "notes",
        "digital pen",
        "pen",
        "irisnotes",
        "driver",
        ],
      platforms=[
        "Operating System :: POSIX :: Linux",
        ],
      license="License :: OSI Approved :: GNU General Public License (GPL)",
      classifiers=[
        "Intended Audience :: End Users/Desktop",
        "Development Status :: 3 - Alpha",
        "Environment :: Console",
        "License :: OSI Approved :: GNU General Public License (GPL)",
        "Natural Language :: English",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Topic :: Desktop Environment",
        ],
      packages=[
        'm210',
        ],
      scripts=[
        'bin/m210',
        ],
      )
