# -*- coding: utf-8 -*-
# python-m210
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
                         sources=['m210/modulehidraw.c'])

moduleinput = Extension('m210.input',
                        sources=['m210/moduleinput.c'])

setup(name="python-m210",
      version="0.4",
      author="Tuomas Räsänen (tuos)",
      author_email="tuos@codegrove.org",
      url="http://codegrove.org/projects/m210",
      description="Python Library for Pegasus Tablet Mobile Notetaker M210",
      long_description="""M210 is a software project which aims to provide easy-to-use tools for
controlling and using Pegasus Mobile Notetaker M210 in modern Linux
desktop environments.""",
      license="GPLv3+",
      platforms=[
        "Linux",
        ],
      package_dir={
        'm210': 'm210',
        },
      packages=[
        'm210',
        ],
      ext_modules=[modulehidraw, moduleinput],
      )
