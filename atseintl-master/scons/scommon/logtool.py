#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import logging
import os

class Logtool(object):
    DEFAULT_MSG = 'reading SConscript'
    DEFAULT_ROOT = 'scroot'
    DEFAULT_ORIGIN_BASENAME = 'SConscript'

    def __init__(self, script_reading_msg=None, logger_name_root=None):
        if script_reading_msg is None:
            self.script_reading_msg = self.DEFAULT_MSG
        else:
            self.script_reading_msg = script_reading_msg

        if logger_name_root is None:
            self.logger_name_root = self.DEFAULT_ROOT
        else:
            self.logger_name_root = logger_name_root

    def getlogger(self, origin_dirname, origin_basename=None):
        if origin_basename is None:
            origin_basename = self.DEFAULT_ORIGIN_BASENAME
        path = [self.logger_name_root]
        if origin_dirname == '.':
            suffix = []
        else:
            suffix = [origin_dirname.replace(os.sep, '.')]
        path.extend(suffix)
        path.append(origin_basename)
        name = '.'.join(path)
        return logging.getLogger(name)

