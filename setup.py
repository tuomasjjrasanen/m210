# -*- coding: utf-8 -*-
# m210
# Copyright © 2011 Tuomas Jorma Juhani Räsänen <tuomasjjrasanen@tjjr.fi>

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

setup(name='m210',
      provides=[
        'm210',
        ],
      version='0.4',
      author=u'Tuomas Jorma Juhani Räsänen',
      author_email='tuomasjjrasanen@tjjr.fi',
      maintainer=u'Tuomas Jorma Juhani Räsänen',
      maintainer_email='tuomasjjrasanen@tjjr.fi',
      url="http://tjjr.fi/software/m210/",
      download_url="http://pypi.python.org/packages/source/m/m210/m210-0.4.tar.gz",
      description="Tools for Pegasus Mobile Notetaker M210",
      long_description="""M210 provides tools for using Pegasus Tablet Mobile Notetaker M210
<http://www.pegatech.com/?CategoryID=218> in Linux systems.""",
      keywords=[
        "pegasus",
        "notetaker",
        "m210",
        "notes",
        "digital pen",
        "pen",
        "irisnotes",
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
