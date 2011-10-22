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
      version="0.5",
      author="Tuomas Jorma Juhani Räsänen",
      author_email="tuomasjjrasanen@tjjr.fi",
      maintainer="Tuomas Jorma Juhani Räsänen",
      maintainer_email="tuomasjjrasanen@tjjr.fi",
      url="http://tjjr.fi/software/m210/",
      description="Pegasus Mobile NoteTaker (M210) Controller",
      long_description="""Tools for controlling Pegasus Tablet Mobile Notetaker M210
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
