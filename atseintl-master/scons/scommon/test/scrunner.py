#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import os
import subprocess

def create_file_with_content(filepath, content):
    f = open(filepath, 'w')
    f.write(content)
    f.close()

def subprocess_spawn(arglist, env):
    return subprocess.check_output(arglist, env=env)

class Dirutils(object):
    def getcwd(self):
        return os.getcwd()
    def chdir(self, path):
        os.chdir(path)
    def exists(self, path):
        return os.path.exists(path)

class SconsRunner(object):
    DEFAULT_SCONS_NAME = 'scons'
    SCONSTRUCT_FILE_NAME = 'SConstruct'
    PATH_VAR_NAME = 'PATH'
    PYTHONPATH_VAR_NAME = 'PYTHONPATH'
    SCONSIGN_NAME = '.sconsign.dblite'

    def __init__(self, builddir, default_path, pythonpath, sconsapp=None,
            filecreator=None, unlink=None, spawn=None, dirutils=None):
        self.builddir = builddir
        self.default_path = self._strip_path(default_path)
        self.pythonpath = self._strip_path(pythonpath)
        self.sconsapp = self.DEFAULT_SCONS_NAME if sconsapp is None else sconsapp
        self.filecreator = create_file_with_content if filecreator is None else filecreator
        self.unlink = os.unlink if unlink is None else unlink
        self.spawn = subprocess_spawn if spawn is None else spawn
        self.dirutils = Dirutils() if dirutils is None else dirutils
        self.startdir = self.dirutils.getcwd()

    def get_builddir(self):
        return self.builddir

    def append_to_pythonpath(self, path):
        self.pythonpath = os.pathsep.join([self.pythonpath, self._strip_path(path)])

    def run(self, sconstruct_content, extra_args=None):
        self.dirutils.chdir(self.builddir)
        try:
            self.filecreator(self.SCONSTRUCT_FILE_NAME, sconstruct_content)
            try:
                args = [self.sconsapp]
                if extra_args:
                    args.extend(extra_args)
                return self.spawn(args, env=self._create_env())
            finally:
                self.unlink(self.SCONSTRUCT_FILE_NAME)
                self._remove_sideffect_files()
        finally:
            self.dirutils.chdir(self.startdir)

    def _create_env(self):
        env = {
            self.PATH_VAR_NAME: self.default_path,
            self.PYTHONPATH_VAR_NAME: self.pythonpath
        }
        return env

    def _strip_path(self, path):
        return path.strip(os.pathsep)

    def _remove_sideffect_files(self):
        sconsign_file = os.path.join(self.builddir, self.SCONSIGN_NAME)
        if self.dirutils.exists(sconsign_file):
            self.unlink(sconsign_file)
