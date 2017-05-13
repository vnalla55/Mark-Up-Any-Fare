#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013, 2014

import os
import logging
log = logging.getLogger(__name__)

from . import bridge
from . import location
from . import plugins
from .exceptions import LinktypeError, NoInputError
from .utils import list_summary


class Primitives(object):
    def __init__(self, renv, linktype):
        self.renv = renv
        self.linktype = linktype

    def object(self, source, target=None):
        return self._linktype_dependent_obj_builder('object')(source, target)

    def library(self, name, objects):
        return self._linktype_dependent_obj_builder('library')(name, objects)

    def executable(self, name, objects):
        return self.renv.Program(name, objects)

    def installation(self, target_path, lib):
        return self.renv.Symlink(target_path, lib)

    def static_object(self, source, target=None):
        return self.renv.StaticObject(target=target, source=source)

    def shared_object(self, source, target=None):
        return self.renv.SharedObject(target=target, source=source)

    def static_library(self, libname, objects):
        return self.renv.StaticLibrary(libname, objects)

    def shared_library(self, libname, objects):
        return self.renv.SharedLibrary(libname, objects)

    def _linktype_dependent_obj_builder(self, objname):
        property = self.linktype + '_' + objname
        try:
            return getattr(self, property)
        except AttributeError:
            raise LinktypeError('Unknown linktype %s; unable to access %s' % (self.linktype, property))



class Factory(object):

    def __init__(self, env, config,
                 primitives_factory=None,
                 objmaker_klass=None,
                 libmaker_klass=None,
                 objlistmaker_klass=None,
                 progmaker_klass=None,
                 testmaker_klass=None,
                 testsuite_record_klass=None,
                 testsuite_klass=None,
                 suiterunner_klass=None,
                 serverinit_klass=None):
        '''
        Keys in config:
            linktype,
            object_cache,
            installdir,
            common_test_objects,
            test_include_paths,
            test_isystem_paths,
            utexe,
            utexe_db,
            utexe_ld_lib_paths,
            utexe_sysenv,
        '''

        self.primitives_factory = primitives_factory or Primitives
        self.env = env
        self.config = config

        self.objmaker_klass = objmaker_klass or ObjMaker
        self.libmaker_klass = libmaker_klass or LibMaker
        self.objlistmaker_klass = objlistmaker_klass or ObjectListMaker
        self.progmaker_klass = progmaker_klass or ProgMaker
        self.testmaker_klass = testmaker_klass or MultiTestMaker
        self.testsuite_record_klass = testsuite_record_klass or TestSuiteRecord
        self.testsuite_klass = testsuite_klass or TestSuite
        self.suiterunner_klass = suiterunner_klass or SuiteRunner
        self.serverinit_klass = serverinit_klass or LinkMaker

    def _effective_libmaker(self, libname, objobserver):
        # The cache may be empty but cannot be absent/None
        if self.config.get('object_cache') is not None:
            return self.objlistmaker_klass(self.env, libname,
                                           self.config['object_cache'],
                                           objobserver=objobserver)
        return self.libmaker_klass(self.env, libname,
                                   installdir=self.config.get('installdir'),
                                   objobserver=objobserver)

    @property
    def primitives(self):
        return self.primitives_factory(self.env.raw, self.config['linktype'])

    def objmaker(self, objobserver=None):
        return self.objmaker_klass(self.env, objobserver=objobserver)

    def libmaker(self, libname, objobserver=None):
        return self._effective_libmaker(libname, objobserver)

    def progmaker(self, name, objobserver=None):
        return self.progmaker_klass(env=self.env, name=name, objobserver=objobserver)

    def linkmaker(self, name):
        return self.serverinit_klass(env=self.env, name=name)

    def multitestmaker(self, whitelist=None, glob_pattern=None,
            blacklist=None, objobserver=None):
        return self.testmaker_klass(env=self.env,
            whitelist=whitelist, glob_pattern=glob_pattern,
            blacklist=blacklist, objobserver=objobserver)

    def tsrecord(self, whitelist=None, blacklist=None,
            glob_pattern=None, executor=None, name=None,
            libs=None, extra_ld_lib_paths=None):

        return self.testsuite_record_klass(
            whitelist=whitelist,
            blacklist=blacklist,
            glob_pattern=glob_pattern,
            executor=executor,
            name=name,
            libs=libs,
            extra_ld_lib_paths=extra_ld_lib_paths)

    def testsuite(self, test_suite_record,
            common_objects=None, common_libs=None,
            include_paths=None, isystem_paths=None,
            objobserver=None):
        common_objects = common_objects or self.config.get('common_test_objects')
        include_paths = include_paths or self.config.get('test_include_paths')
        isystem_paths = isystem_paths or self.config.get('test_isystem_paths')
        return self.testsuite_klass(self.env,
            test_suite_record=test_suite_record,
            common_objects=common_objects, common_libs=common_libs,
            include_paths=include_paths, isystem_paths=isystem_paths,
            objobserver=objobserver)

    def suiterunner(self, executor=None, startdir=None, skip_passed_tests=False):
        r = self.suiterunner_klass(self.env,
                utexe=self.config.get('utexe'),
                utexe_db=self.config.get('utexe_db'),
                executor=executor,
                startdir=startdir,
                skip_passed_tests=skip_passed_tests)

        ld_lib_paths = self.config.get('utexe_ld_lib_paths')
        if ld_lib_paths:
            r.add_ld_library_paths(*ld_lib_paths)
        r.update_sysenv(self.config.get('utexe_sysenv'))
        return r


class ObjMaker(object):
    '''
    Creates a list of object nodes from input files.
    '''

    def __init__(self, env, objobserver=None):
        self.env = env
        self.sources = []
        self.objobserver = objobserver

    def add_sources(self, sources):
        sources = self.env.splitsources(sources)
        self.sources.extend(sources)
        return self

    def make(self):
        if not self.sources:
            raise NoInputError('No input sources')

        log.info('making objects from sources = %s', list_summary(map(str, self.sources)))
        log.debug('making objects from sources = %s', map(str, self.sources))

        objects = []
        for s in self.sources:
            oo = self.env.factory.primitives.object(s)
            objects.extend(oo)

        if self.objobserver is not None:
            for o in objects:
                self.objobserver(o)

        return objects



class Objbank(object):
    '''
    Stores scons object nodes in its 'objects' property.
    Objects may be added either directly or as source
    files: Objbank converts sources to objects automatically.
    '''
    def __init__(self, objmaker_factory, objobserver=None):
        self.objmaker_factory = objmaker_factory
        self.objobserver = objobserver
        self.objects = []

    def add_sources(self, sources):
        objmaker = self.objmaker_factory(objobserver=self.objobserver)
        objmaker.add_sources(sources)
        self.add_objects(objmaker.make())

    def add_objects(self, objects):
        self.objects.extend(objects)



class CompositeMaker(object):
    '''
    To use this object, extend it with a strategy ('builder')
    that converts objects into a composite artifact, e.g.
    a library or a program. Then:
    - add objects
    - call make and collect the composite.
    '''
    def __init__(self, env, name, builder,
        objobserver=None, objbank_factory=None):

        self.env = env
        self.name = name
        self.builder = builder
        self.objbank_factory = objbank_factory or Objbank
        self.objbank = self.objbank_factory(
                self.env.factory.objmaker, objobserver)

    def add_sources(self, sources):
        self.objbank.add_sources(sources)
        return self

    def add_objects(self, objects):
        self.objbank.add_objects(objects)
        return self

    def make(self):
        return self.builder.make(self.env, self.name, self.objbank.objects)


class LibBuilder(object):
    def __init__(self, installdir=None, node_factory=None):
        self.installdir = installdir
        self.node_factory = node_factory or location.Node

    def make(self, env, name, objects):
        if not objects:
            raise NoInputError('No input objects')

        library = env.factory.primitives.library(name, objects)[0]

        log.info('making library %s from objects %s', library, list_summary(map(str, objects)))
        log.debug('making library %s from objects %s', library, map(str, objects))

        if self.installdir:
            path = self._calculate_install_path(library)
            log.info('installing library %s as %s', library, env.subst(path))
            env.factory.primitives.installation(path, library)

        return library

    def _calculate_install_path(self, library):
        return os.path.join(self.installdir, self.node_factory(library).basename)


def LibMaker(env, libname, installdir=None, objobserver=None):
    b = LibBuilder(installdir)
    return CompositeMaker(env, libname, b, objobserver=objobserver)


class ObjectListBuilder(object):
    def __init__(self, object_cache):
        self.object_cache = object_cache

    def make(self, env, name, objects):
        if not objects:
            raise NoInputError('No input objects')

        self.object_cache.setdefault(name, []).extend(objects)

        log.info('extending list of direct link objects %s',
                 list_summary(map(str, objects)))
        log.debug('extending list of direct link objects %s',
                  map(str, objects))

        return None  # We do not expect to use these objects directly


def ObjectListMaker(env, libname, object_cache, objobserver=None):
    b = ObjectListBuilder(object_cache)
    return CompositeMaker(env, libname, b, objobserver=objobserver)

class ProgBuilder(object):
    def make(self, env, name, objects):
        return env.factory.primitives.executable(name, objects)

def ProgMaker(env, name, objobserver=None):
    b = ProgBuilder()
    return CompositeMaker(env, name, b, objobserver=objobserver)


class MultiTestBuilder(object):
    DEFAULT_GLOB_PATTERN = 'test/*.cpp'
    DEFAULT_BLACKLIST = []

    def __init__(self, whitelist=None, glob_pattern=None, blacklist=None, objobserver=None):
        self.whitelist = whitelist or []
        self.glob_pattern = glob_pattern
        self.blacklist = self.DEFAULT_BLACKLIST[:]
        if blacklist:
            self.blacklist.extend(blacklist)

        self.objobserver = objobserver

    def make(self, env, name, objects):
        testfiles = self._filter_filelist(self._create_filelist(env))
        tests_as_shared_libs = []
        for testfile in testfiles:
            log.debug('making library for test file %s' % testfile)
            lm = env.factory.libmaker(
                    self._file_to_libname(testfile),
                    objobserver=self.objobserver)
            lm.add_sources(testfile)
            lm.add_objects(objects)
            tests_as_shared_libs.append(lm.make())
        return tests_as_shared_libs

    def _create_filelist(self, env):
        if self.whitelist and self.glob_pattern:
            raise ValueError('Use at most one of whitelist or glob_pattern to specify test files')
        test_files = self.whitelist
        if not test_files:
            if self.glob_pattern:
                pattern = self.glob_pattern
            else:
                pattern = self.DEFAULT_GLOB_PATTERN

            test_files = env.glob(pattern)
        return test_files

    def _filter_filelist(self, test_files):
        filtered = []
        for f in test_files:
            if f in self.blacklist:
                log.info('Dropping %s since in blacklist' % f)
            else:
                filtered.append(f)
        return filtered

    def _file_to_libname(self, testfile):
        return os.path.splitext(testfile)[0]

def MultiTestMaker(env, whitelist=None, glob_pattern=None, blacklist=None, objobserver=None):
    b = MultiTestBuilder(whitelist=whitelist, glob_pattern=glob_pattern,
            blacklist=blacklist, objobserver=objobserver)
    return CompositeMaker(env, None, b, objobserver=objobserver)


class TestSuiteRecord(object):
    DEFAULT_RUNNER = 'utexe'
    def __init__(self, whitelist=None, blacklist=None,
            glob_pattern=None, executor=None, name=None, libs=None,
            extra_ld_lib_paths=None, tests_as_shared_libs=None):
        self.whitelist = whitelist
        self.blacklist = blacklist
        self.glob_pattern = glob_pattern

        if executor is None:
            self.executor = self.DEFAULT_RUNNER
        else:
            self.executor = executor

        if name is None:
            self.name = self.executor
        else:
            self.name = name

        self.libs = libs or []
        self.extra_ld_lib_paths = extra_ld_lib_paths or []
        self.tests = tests_as_shared_libs



class TestSuite(object):

    def __init__(self, env, test_suite_record,
            common_objects=None, common_libs=None,
            include_paths=None, isystem_paths=None,
            objobserver=None):

        self.env = env.clone()
        self.tsrecord = test_suite_record

        self.common_objects = common_objects or []
        self.common_libs = common_libs or []
        self.include_paths = include_paths or []
        self.isystem_paths = isystem_paths or []

        self.objobserver = objobserver

    def _update_env(self):
        self.env.add_include_paths(*self.include_paths)
        self.env.add_isystem_paths(*self.isystem_paths)
        self.env.add_libs(*self.common_libs)
        self.env.add_libs(*self.tsrecord.libs)

    def make(self):
        self._update_env()
        tmaker = self.env.factory.multitestmaker(whitelist=self.tsrecord.whitelist,
                glob_pattern=self.tsrecord.glob_pattern, blacklist=self.tsrecord.blacklist,
                objobserver=self.objobserver)
        if self.common_objects:
            tmaker.add_objects(self.common_objects)
        self.tsrecord.tests = tmaker.make()
        return self.tsrecord


class SuiteRunner(object):

    def __init__(self, env, utexe, utexe_db, executor=None,
            startdir=None, calc_startdir=None, skip_passed_tests=False):
        self.env = env.clone()
        self.utexe = utexe
        self.utexe_db = utexe_db
        self.calc_startdir = calc_startdir or self._calc_startdir
        if startdir is None:
            self.startdir = self.calc_startdir()
        else:
            self.startdir = startdir
        self.executor = executor
        self._install_runner()
        self.skip_passed_tests = skip_passed_tests

    def add_ld_library_paths(self, *paths):
        self.env.add_ld_library_paths(*paths)

    def update_sysenv(self, sysenv):
        if sysenv is not None:
            self.env.sysenv.update(sysenv)

    def _install_runner(self):
        if self.executor == 'utexedb':
            app = location.abspath(self.utexe_db)
        else:
            app = location.abspath(self.utexe)
        b = plugins.UtexeRunner(app, self.startdir)
        self.env.add_builder('Utexe', b)

    def _calc_startdir(self):
        return self.env.location.absorig

    def run(self, logfile, tests):
        log_file_nodes = self.env.raw.Utexe(logfile, tests)
        if not self.skip_passed_tests:
            bridge.always_build(log_file_nodes)
        bridge.no_cache(log_file_nodes)
        return log_file_nodes


class LinkBuilder(object):
    def make(self, env, name, objects):
        return env.factory.primitives.installation(name, objects)

def LinkMaker(env, name):
    b = LinkBuilder()
    return CompositeMaker(env, name, b)
