#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import scrunner
import sconstest
import pytools

import os

DEFAULT_ENV_PATH = '/atse_git/fbldfunc/python/bin:/usr/local/bin:/opt/bin:/bin:/usr/bin'
SCONS_PATH = '/atse_git/fbldfunc/scons/lib/scons/scons'
SCONS_LIB_PATH = '/atse_git/fbldfunc/scons/lib/scons/scons-local-2.3.0'
BUILD_DIR_NAME = 'build'
INSTALL_DIR_NAME = 'install'

APP_NAME = 'bulwa'
SOURCE_NAME = 'source.cpp'
EXTRA_SOURCES = ['extra1.cpp', 'extra2.cpp']


# generates 2 files: bulwa and source.o
PROGRAM_BUILD_LINE = ("{object} = env.Program('{appname}', ['{source}'])".format(
                object=APP_NAME, appname=APP_NAME, source=SOURCE_NAME), 2)
DUMMY_OBJ_NAME = 'dumy_object'

VERBOSE_VARIABLE = 'VERBOSE'

def get_build_dir():
    return pytools.path_from_fdir(__file__, BUILD_DIR_NAME)

def _get_scommon_lib_path():
    head = os.path.abspath(__file__)
    while True:
        head, tail = os.path.split(head)
        if tail == 'scommon':
            return head
        if tail == '':
            raise OSError('could not find scommon location')

def runner():
    pythonpath = _get_scommon_lib_path()
    return scrunner.SconsRunner(get_build_dir(),
            DEFAULT_ENV_PATH, pythonpath=pythonpath, sconsapp=SCONS_PATH)

def is_verbose():
    try:
        return os.environ[VERBOSE_VARIABLE] == '1'
    except KeyError:
        return False

class Test(sconstest.SconsTest):
    def __init__(self, *args, **kwargs):
        super(Test, self).__init__(runner, pytools.DirSweeper, verbose=is_verbose(), *args, **kwargs)

def extra_sources():
    return [os.path.join(get_build_dir(), f) for f in EXTRA_SOURCES]

