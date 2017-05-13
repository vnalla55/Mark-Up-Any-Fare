#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import unittest
from mock import Mock, patch, call

import sconstestenv

import scommon.plugins

import pytools
import os

class TestExternalSpawn(sconstestenv.Test):
    FAKE_SPAWN_MSG = 'scommonfakes.spawn called with'
    def test_calls_external_function_for_spawn(self):
        self.sf.add_import('scommon.plugins')
        self.sf.add_import("scommonfakes")
        self.sf.add_env_creation()
        self.sf.append("env['SPAWN'] = scommon.plugins.ExternalSpawn(scommonfakes.spawn)")
        # no real files will be produced since we use a fake instead of the compiler
        self.sf.append("env.Object('%s')" % sconstestenv.SOURCE_NAME)

        # add the test dir to enable scommonfakes
        self.run_file(pythonpath_suffix=pytools.path_from_fdir(__file__))

        self.assert_output_contains(self.FAKE_SPAWN_MSG)

class TestSymlinks(sconstestenv.Test):

    def up(self):
        self.appfile = os.path.join(self.builddir(), sconstestenv.APP_NAME)
        self.linkfile = os.path.join(self.builddir(), sconstestenv.INSTALL_DIR_NAME, sconstestenv.APP_NAME)
        self._create_scons_file()

    def _assert_islink(self, filename, destination):
        self.assertTrue(os.path.islink(filename))
        self.assertEqual(os.readlink(filename), os.path.abspath(destination))

    def _create_scons_file(self):
        self.sf.add_import('scommon.plugins')
        self.sf.add_env_creation()

    def test_install_as_symlink(self):
        self.sf.append("env.Replace(INSTALL = scommon.plugins.install_as_symlink)")
        self.sf.add_program_creation(sconstestenv.APP_NAME, sconstestenv.SOURCE_NAME)
        self.sf.append("env.Install('%s', %s)" % (sconstestenv.INSTALL_DIR_NAME, sconstestenv.APP_NAME), 1)
        self.run_file()
        self._assert_islink(self.linkfile, self.appfile)

    def test_symlink_builder(self):
        self.sf.append("b = Builder(action = scommon.plugins.symlink_action)")
        self.sf.append("env.Append(BUILDERS = {'Symlink' : b})")
        self.sf.add_program_creation(sconstestenv.APP_NAME, sconstestenv.SOURCE_NAME)
        self.sf.append("env.Symlink('%s', %s)" % (self.linkfile, sconstestenv.APP_NAME), 1)
        self.run_file()
        self._assert_islink(self.linkfile, self.appfile)


class TestTestFactoryBuilder(sconstestenv.Test):

    def up(self):
        self.verbose = 1
        self.add_extra_scons_args('--tree=all')
        self.sf.add_import('scommon.plugins')
        self.sf.add_env_creation()
        self.sf.append("bld = scommon.plugins.TestFactoryBuilder('factory_generator_pl/Generator.pl')")
        self.sf.append("env.Append(BUILDERS = {'CppTestFactoryBuilder' : bld})")

    def test_builder_produces_factory_files(self):
        self.sf.append("env.CppTestFactoryBuilder('factorydefs/BlackoutInfo.def')", 2)
        self.run_file()
        for ofile in ['TestBlackoutInfoFactory.h', 'TestBlackoutInfoFactory.cpp']:
            self.assert_file_exists(ofile)


