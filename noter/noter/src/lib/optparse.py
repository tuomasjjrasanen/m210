# -*- coding: utf-8 -*-
from __future__ import absolute_import

import optparse

import noter

VERSION_TEXT = '''%s %s
Copyright (C) 2010 Tuomas (tuos) Räsänen <tuos@codegrove.org>
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

Written by Tuomas Räsänen.''' % ('%prog', noter.VERSION)

class OptionParser(optparse.OptionParser):

    def __init__(self, *args, **kwargs):
        kwargs['version'] = VERSION_TEXT
        optparse.OptionParser.__init__(self, *args, **kwargs)
