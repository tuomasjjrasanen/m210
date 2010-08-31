# -*- coding: utf-8 -*-
# m210
# Copyright © 2010 Tuomas Räsänen (tuos) <tuos@codegrove.org>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import absolute_import

import optparse

import m210

VERSION_TEXT = '''%s %s
Copyright (C) 2010 Tuomas (tuos) Räsänen <tuos@codegrove.org>
License GPLv3: GNU GPL version 3 <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

Written by Tuomas Räsänen.''' % ('%prog', m210.VERSION)

class OptionParser(optparse.OptionParser):

    def __init__(self, *args, **kwargs):
        kwargs['version'] = VERSION_TEXT
        optparse.OptionParser.__init__(self, *args, **kwargs)
