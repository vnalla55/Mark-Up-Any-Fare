#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import unittest
from mock import Mock, patch, call, MagicMock
import sconstestenv

import scommon.factory

import os
import subprocess

class TestObjectProducers(sconstestenv.Test):

    LIBPREFIX = 'lib'
    DUMMY_LIB_NAME = 'bar'
    OBJECT_EXTENSION = '.o'
    SHARED_OBJECT_EXTENSION = '.os'
    LIB_EXTENSION = '.a'
    SHARED_LIB_EXTENSION = '.so'
    DUMMY_APP_NAME = 'niceapp'
    APP_MESSAGE = 'Hello, test!\n'

    def _create_scons_file(self):
        self.sf.add_import("scommon.factory")
        self.sf.add_env_creation()

    def _object_creation_run(self, funcname, expected_obj_ext):
        self._create_scons_file()
        self.sf.append("oo = scommon.factory.{funcname}(env, '{source}')".format(funcname=funcname, source=self.file1), 1)
        self.sf.append("Library(oo)", 1)
        self.run_file(self.sf)

        # extra space to match exact suffix
        self.assert_output_contains(self.rawfilename + expected_obj_ext + ' ')
        self.assert_output_contains(self.LIBPREFIX + self.rawfilename)

    def _lib_creation_run(self, obj_factory, funcname, extension):
        self._create_scons_file()
        self.sf.append("oo = %s('%s')" % (obj_factory, self.file1), 1)
        self.sf.append("ll = scommon.factory.{funcname}(env, '{libname}', oo)".format(
                libname=self.DUMMY_LIB_NAME, funcname=funcname), 1)
        self.sf.append("Program(['%s', ll])" % sconstestenv.SOURCE_NAME, 2)
        self.run_file(self.sf)
        self.assert_output_contains(self.LIBPREFIX + self.DUMMY_LIB_NAME + extension)

    def _prog_creation_run(self):
        self._create_scons_file()
        self.sf.append("app = scommon.factory.make_executable(env, '{appname}', ['{source}'])".format(
                appname=self.DUMMY_APP_NAME, source=sconstestenv.SOURCE_NAME), 2)
        self.run_file(self.sf)
        self.assert_output_contains(self.DUMMY_APP_NAME)
        apppath = os.path.join(sconstestenv.get_build_dir(), self.DUMMY_APP_NAME)
        hello = subprocess.check_output([apppath])
        self.assertEqual(self.APP_MESSAGE, hello)

    def up(self):
        self.file1 = sconstestenv.extra_sources()[0]
        self.rawfilename = os.path.splitext(os.path.basename(self.file1))[0]

    def test_make_static_object(self):
        self._object_creation_run('make_static_object', self.OBJECT_EXTENSION)

    def test_make_shared_object(self):
        self._object_creation_run('make_shared_object', self.SHARED_OBJECT_EXTENSION)

    def test_make_static_library(self):
        self._lib_creation_run('Object', 'make_static_library', self.LIB_EXTENSION)

    def test_make_shared_library(self):
        self._lib_creation_run('SharedObject', 'make_shared_library', self.SHARED_LIB_EXTENSION)

    def test_make_executable(self):
        self._prog_creation_run()

class TestSymlinkInstall(unittest.TestCase):
    DUMMY_PATH = 'one/two/three'
    DUMMY_LIB = 'somelib'

    def test_forwards_install_request_correctly(self):
        env = Mock()
        scommon.factory.symlink_install(env, self.DUMMY_PATH, self.DUMMY_LIB)
        env.Symlink.assert_called_once_with(self.DUMMY_PATH, self.DUMMY_LIB)

class TestRawenvRunner(unittest.TestCase):

    DUMMY_ARG_1 = 'one'
    DUMMY_ARG_2 = 'two'
    DUMMY_DICT = {'three': 3, 'four': 4}

    def setUp(self):
        self.env = Mock()
        self.action = Mock()

    def test_forwards_call_to_action_correctly(self):
        runner = scommon.factory.RawenvRunner(self.env, self.action)
        runner(self.DUMMY_ARG_1, self.DUMMY_ARG_2, **self.DUMMY_DICT)
        self.env.rawenv.assert_called_once_with()
        self.action.assert_called_once_with(self.env.rawenv.return_value, self.DUMMY_ARG_1, self.DUMMY_ARG_2, **self.DUMMY_DICT)


class TestLinktypeDependentFactory(unittest.TestCase):
    LINKTYPES = ['static', 'shared']
    UNKNOWN_LINKTYPE = 'Michael Jackson'

    def setUp(self):
        self.env = Mock()
        self.objproducer_mock_klasses = self._create_mocks_for_linktypes()
        self.libproducer_mock_klasses = self._create_mocks_for_linktypes()

        self.f = scommon.factory.LinktypeDependentFactory(
            self.objproducer_mock_klasses['static'],
            self.objproducer_mock_klasses['shared'],
            self.libproducer_mock_klasses['static'],
            self.libproducer_mock_klasses['shared'])

    def _create_mocks_for_linktypes(self):
        mocks = {}
        for linktype in self.LINKTYPES:
            mocks[linktype] = Mock()
        return mocks

    def _test_factory_call(self, factory_call, mock_klasses):
        for linktype in self.LINKTYPES:
            returned_producer = factory_call(linktype)
            mock_klass = mock_klasses[linktype]
            mock_klass.assert_called_once_with(self.env)
            self.assertEqual(returned_producer, mock_klass.return_value)

    def test_returns_correct_objproducer(self):
        self._test_factory_call(lambda linktype: self.f.objproducer(linktype, self.env),
                self.objproducer_mock_klasses)

    def test_returns_correct_libproducer(self):
        self._test_factory_call(lambda linktype: self.f.libproducer(linktype, self.env),
                self.libproducer_mock_klasses)

    def test_raises_if_objproducer_linktype_unknown(self):
        with self.assertRaises(scommon.factory.UnknownLinktypeError):
            self.f.objproducer(self.UNKNOWN_LINKTYPE, self.env)

    def test_raises_if_libproducer_linktype_unknown(self):
        with self.assertRaises(scommon.factory.UnknownLinktypeError):
            self.f.libproducer(self.UNKNOWN_LINKTYPE, self.env)


class TestObjMaker(unittest.TestCase):

    FILELIST = ['one.cpp', 'two.cpp', 'three.cpp']
    DUMMY_STR = 'something.cpp'

    def _obj_prefix(self, filename):
        return 'OBJ_' + filename

    def setUp(self):
        self.env = Mock()
        self.splitsources = Mock()
        self.splitsources.return_value = self.FILELIST
        self.factory = Mock()
        self.factory.objproducer.return_value.side_effect = lambda f: [self._obj_prefix(f)]
        self.objmaker = scommon.factory.ObjMaker(self.env, splitsources=self.splitsources, factory=self.factory)

    def test_adds_sources_correctly(self):
        self.objmaker.add_sources(self.DUMMY_STR)
        self.splitsources.assert_called_once_with(self.DUMMY_STR)
        self.assertEqual(self.objmaker.get_sources(), self.FILELIST)

    def test_builds_objects_correctly(self):
        self.test_adds_sources_correctly()
        objects = self.objmaker.make_objects()
        expected = map(self._obj_prefix, self.FILELIST)
        self.assertEqual(objects, expected)

    def test_raises_if_no_objects(self):
        with self.assertRaises(scommon.factory.NoSourceFilesError):
            self.objmaker.make_objects()

# TODO: test objobserver
class TestLibMaker(unittest.TestCase):
    DUMMY_LIBNAME = 'somelib.o'
    DUMMY_INSTALLDIR = '/a/dir/somewhere'
    DUMMY_NODE_NAME = 'another.o'
    DUMMY_NODE_DIR = 'somewehere/is'
    DUMMY_SOURCE = 'asourcehere.c'
    DUMMY_LIBDEP_NAME = 'Somelib'

    def _make_libmaker(self, installdir=None):
        return scommon.factory.LibMaker(self.env, self.DUMMY_LIBNAME, installdir=installdir,
                nodepath=self.nodepath, factory=self.factory)

    def setUp(self):
        self.env = Mock()
        self.nodepath = Mock()
        self.factory = Mock()
        self.produced_lib = Mock()
        self.factory.libproducer.return_value.return_value = [self.produced_lib]

        self.objmaker_mocks = []
        self.libmaker = self._make_libmaker()

    def test_sets_name_properly(self):
        self.assertEqual(self.libmaker.get_name(), self.DUMMY_LIBNAME)
        self.assertEqual(self.env.mock_calls, [])

    def _install_objmaker_mock(self, objmaker_mock):
        # some hack aliasing
        objmaker_mock.make_objects.return_value = [objmaker_mock.return_value]
        self.objmaker_mocks.append(objmaker_mock)

    def _add_objmaker(self):
        objmaker_mock = Mock()
        self._install_objmaker_mock(objmaker_mock)
        self.libmaker.add_objmaker(objmaker_mock)

    def _add_sources(self):
        self.libmaker.add_sources(self.DUMMY_SOURCE)
        self._install_objmaker_mock(self.factory.objmaker.return_value)

    def _assert_all_objmakers_called(self):
        for objmaker in self.objmaker_mocks:
            objmaker.make_objects.assert_called_once_with()

    def _get_objmakers_return_values(self):
        return [objmaker.return_value for objmaker in self.objmaker_mocks]

    def _check_make_libraries_inner_processing(self):
        libraries = self.libmaker.make_libraries()
        self._assert_all_objmakers_called()
        self.factory.libproducer.assert_called_once_with(self.env)
        self.factory.libproducer.return_value.assert_called_once_with(
                self.DUMMY_LIBNAME, self._get_objmakers_return_values())
        name, node = libraries[0]
        self.assertEqual(name, self.DUMMY_LIBNAME)
        self.assertEqual(node, self.produced_lib)

    def test_produces_lib_from_objmakers_correctly(self):
        self._add_objmaker()
        self._add_objmaker()

        self._check_make_libraries_inner_processing()
        self.assertEqual(self.env.mock_calls, [])

    def _assert_objmaker_called(self):
        self.factory.objmaker.assert_called_once_with(self.env, objobserver=None)
        self.factory.objmaker.return_value.add_sources.assert_called_once_with(self.DUMMY_SOURCE)

    def test_produces_lib_from_sources_correctly(self):
        self._add_sources()

        self._check_make_libraries_inner_processing()

        self._assert_objmaker_called()
        self.assertEqual(self.env.mock_calls, [])

    def test_produces_lib_from_objmaker_src_mix(self):
        self._add_sources()
        self._add_objmaker()

        self._check_make_libraries_inner_processing()

        self._assert_objmaker_called()
        self.assertEqual(self.env.mock_calls, [])

    def test_raises_if_no_object_sources_added(self):
        with self.assertRaises(scommon.factory.NoObjectsError):
            libraries = self.libmaker.make_libraries()

    def test_installs_library_correctly(self):
        self.libmaker = self._make_libmaker(installdir=self.DUMMY_INSTALLDIR)
        self._add_sources()
        self.nodepath.return_value = os.path.join(self.DUMMY_NODE_DIR, self.DUMMY_NODE_NAME)

        self.libmaker.make_libraries()

        self.nodepath.assert_called_once_with(self.produced_lib)
        self.factory.libinstaller.assert_called_once_with(self.env)
        self.factory.libinstaller.return_value.assert_called_once_with(
                os.path.join(self.DUMMY_INSTALLDIR, self.DUMMY_NODE_NAME), self.produced_lib)
        self.assertEqual(self.env.mock_calls, [])

    def _add_lib_dependency(self):
        self._add_sources()
        self.libmaker.add_libname_dependencies(self.DUMMY_LIBDEP_NAME)
        libraries = self.libmaker.make_libraries()
        self.env.clone.assert_called_once_with()

    def test_handles_library_dependencies_if_present(self):
        self._add_lib_dependency()
        self.env.clone.return_value.add_libs.assert_called_once_with(self.DUMMY_LIBDEP_NAME)

    def test_handles_library_dependencies_and_dir(self):
        self.libmaker = self._make_libmaker(installdir=self.DUMMY_INSTALLDIR)
        self.nodepath.return_value = os.path.join(self.DUMMY_NODE_DIR, self.DUMMY_NODE_NAME)
        self._add_lib_dependency()
        self.env.clone.return_value.add_libs.assert_called_once_with(self.DUMMY_LIBDEP_NAME)
        self.env.clone.return_value.add_libpaths.assert_called_once_with(self.DUMMY_INSTALLDIR)


class TestMultiLibMaker(unittest.TestCase):
    DUMMY_LIBDATA = [('libone', 'somesource.cpp'), ('libtwo', 'anothersource.c')]
    RETURNED_NAMES = ['first', 'second']
    DUMMY_RETURNED_LIBS = [['lib1', 'lib2'], ['lib3']]
    DUMMY_LINKTYPE = 'cosmic'

    def setUp(self):
        self.env = Mock()
        self.factory = Mock()
        self.inner_factory = Mock()

        # I use this to deceive Mock so that it does not
        # account calls to the inner library to the main library
        # mock.
        def innerfactory(**kwargs):
            return self.inner_factory
        self.factory.reconfigured.side_effect = innerfactory

        self.libmaker_mocks = [Mock(), Mock()]
        self.inner_factory.libmaker.side_effect = self.libmaker_mocks

        self._set_names_returned_by_libmakers()
        self.multilibmaker = scommon.factory.MultiLibMaker(self.env, self.factory)

    def _set_names_returned_by_libmakers(self):
        for i, lmaker_mock in enumerate(self.libmaker_mocks):
            lmaker_mock.get_name.return_value = self.RETURNED_NAMES[i]
            lmaker_mock.make_libraries.return_value = self.DUMMY_RETURNED_LIBS[i]

    def test_adds_libraries_correctly(self):
        for libname, libsources in self.DUMMY_LIBDATA:
            self.multilibmaker.add_library(libname, libsources)
        self.factory.reconfigured.assert_has_calls([call(linktype=None)]*2)

        expected_calls = [call(self.env, self.DUMMY_LIBDATA[0][0]), call(self.env, self.DUMMY_LIBDATA[1][0])]
        self.inner_factory.libmaker.assert_has_calls(expected_calls, any_order=True)

        for i, lmaker_mock in enumerate(self.libmaker_mocks):
            lmaker_mock.add_sources.assert_called_once_with(self.DUMMY_LIBDATA[i][1])
        self.assertEqual(self.multilibmaker.get_libnames(), self.RETURNED_NAMES)

    def test_makes_libs_correctly(self):
        self.test_adds_libraries_correctly()
        libs = self.multilibmaker.make_libs()
        self.assertEqual(libs, sum(self.DUMMY_RETURNED_LIBS, []))
        for lmaker_mock in self.libmaker_mocks:
            lmaker_mock.make_libraries.assert_called_once_with()

    def test_passes_linktype_to_reconfigured(self):
        libname, sources = self.DUMMY_LIBDATA[0]
        self.multilibmaker.add_library(libname, sources, self.DUMMY_LINKTYPE)
        self.factory.reconfigured.assert_called_once_with(linktype=self.DUMMY_LINKTYPE)
        self.inner_factory.libmaker.assert_called_once_with(self.env, libname)
        self.libmaker_mocks[0].add_sources.assert_called_once_with(sources)
        self.assertEqual(self.multilibmaker.get_libnames(), self.RETURNED_NAMES[:1])

def add_link_prefix_suffix(libname):
        return '${LIBLINKPREFIX}' + libname + '${LIBLINKSUFFIX}'


# TODO: test objobserver
class TestProgMaker(unittest.TestCase):
    DUMMY_NAME = 'aprogramname'
    DUMMY_SOURCE = 'asourcehere.c'
    DUMMY_LIBNAMES = ['library_a', 'library_b']

    def setUp(self):
        self.env = Mock()
        self.factory = Mock()
        self.linking_helper_factory = Mock()
        self.linkinghelper = self.linking_helper_factory.return_value

        self.progmaker = scommon.factory.ProgMaker(self.env, name=self.DUMMY_NAME,
            factory=self.factory, linking_helper_factory=self.linking_helper_factory)

    def test_sets_name_correctly(self):
        self.assertEqual(self.DUMMY_NAME, self.progmaker.getname())

    def test_uses_cloned_env(self):
        self.progmaker.add_sources(self.DUMMY_SOURCE)
        self.factory.objmaker.assert_called_once_with(self.env.clone.return_value, objobserver=None)

    def test_creates_new_objmaker_for_added_sources(self):
        self.progmaker.add_sources(self.DUMMY_SOURCE)
        self.factory.objmaker.assert_called_once_with(self.env.clone.return_value, objobserver=None)
        self.factory.objmaker.return_value.add_sources.assert_called_once_with(self.DUMMY_SOURCE)

    def test_forwards_libname_dependencies_to_linkinghelper(self):
        self.progmaker.add_libname_dependencies(*self.DUMMY_LIBNAMES)
        self.linkinghelper.add_libnames.assert_called_once_with(*self.DUMMY_LIBNAMES)

    def test_forwards_libname_dependencies_whole_archive_to_linkinghelper(self):
        self.progmaker.add_libname_dependencies_whole_archive(*self.DUMMY_LIBNAMES)
        self.linkinghelper.add_libnames_whole_archive.assert_called_once_with(*self.DUMMY_LIBNAMES)


class TestLinkingHelper(unittest.TestCase):
    DUMMY_LIBS = ['normal1', 'normal2']
    DUMMY_WHOLE_ARCH_LIBS = ['wa1', 'wa2', 'wa3']

    NO_WHOLE_ARCHIVE_LIBS_VAR = 'NO_WHOLE_ARCHIVE_LIBS'
    WHOLE_ARCHIVE_LIBS_VAR = 'WHOLE_ARCHIVE_LIBS'
    WHOLE_ARCHIVE_START_VAR = 'WHOLE_ARCHIVE_START'
    WHOLE_ARCHIVE_END_VAR = 'WHOLE_ARCHIVE_END'

    EXPECTED_DICT = {
            'LINKCOM': '$LINK -o $TARGET $SOURCES $LINKFLAGS $_LIBDIRFLAGS $WHOLE_ARCHIVE_START $WHOLE_ARCHIVE_LIBS $WHOLE_ARCHIVE_END $NO_WHOLE_ARCHIVE_LIBS',
            NO_WHOLE_ARCHIVE_LIBS_VAR: [add_link_prefix_suffix(l) for l in DUMMY_LIBS],
            WHOLE_ARCHIVE_LIBS_VAR: [add_link_prefix_suffix(l) for l in DUMMY_WHOLE_ARCH_LIBS],
            WHOLE_ARCHIVE_START_VAR: '-Wl,-whole-archive',
            WHOLE_ARCHIVE_END_VAR: '-Wl,-no-whole-archive'}

    def setUp(self):
        self.env = MagicMock()
        self.helper = scommon.factory.LinkingHelper(self.env)

    def test_creates_link_command_dict_properly(self):
        self.helper.add_libnames(*self.DUMMY_LIBS)
        self.helper.add_libnames_whole_archive(*self.DUMMY_WHOLE_ARCH_LIBS)
        link_command_dict = self.helper.create_link_command_dict()
        self.assertEqual(link_command_dict, self.EXPECTED_DICT)

    def test_updates_env_for_normal_libs_only(self):
        self.helper.add_libnames(*self.DUMMY_LIBS)
        self.helper.update_env()
        self.env.add_libs.assert_called_once_with(*(self.DUMMY_LIBS))
        self.assertEqual(self.env.merge.mock_calls, [])

    def test_updates_env_for_both_lib_types(self):
        self.helper.add_libnames(*self.DUMMY_LIBS)
        self.helper.add_libnames_whole_archive(*self.DUMMY_WHOLE_ARCH_LIBS)
        self.helper.update_env()
        self.env.add_libs.assert_called_once_with(*(self.DUMMY_LIBS + self.DUMMY_WHOLE_ARCH_LIBS))
        self.env.merge.assert_called_once_with(self.EXPECTED_DICT)
        collision_check_calls = [
                call(self.NO_WHOLE_ARCHIVE_LIBS_VAR), call(self.WHOLE_ARCHIVE_LIBS_VAR),
                call(self.WHOLE_ARCHIVE_START_VAR), call(self.WHOLE_ARCHIVE_END_VAR)]
        self.env.__contains__.assert_has_calls(collision_check_calls, any_order=True)

    def test_raises_if_no_libs_added(self):
        with self.assertRaises(scommon.factory.LinkingHelper.NoLibsError):
            self.helper.update_env()

    def test_detects_env_variable_collision(self):
        self.helper.add_libnames_whole_archive(*self.DUMMY_WHOLE_ARCH_LIBS)
        self.env.__contains__.return_value = True
        with self.assertRaises(scommon.factory.LinkingHelper.DuplicateKeyError):
            self.helper.update_env()




