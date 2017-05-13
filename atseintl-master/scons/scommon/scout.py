#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013/2014

import os
import logging

log = logging.getLogger(__name__)

from .utils import TaskToggles
from .toolkit import MakefileSources
from .exceptions import NoTestsError


class Dircontext(object):
    '''
    Stores data used for building code in a particular directory.
    Scout's tasks have access to this data: can use or modify it
    as a "blackboard".

    Meaning of the attributes:
    werror:         boolean variable telling if we want to use the -Werror flag
                    for the directory.
    libentries:     pairs (name, sources) for libraries built in this directory.
                    The common place to store build info for most of libraries
                    however some scons tasks may not use it.
    tsrecords:      list of records storing info about tests being built
                    for this directory. Purpose similar to libentries.
    tmp:            a temporary dict used to pass arbitrary objects from earlier
                    to later scout tasks.
    toreturn:       scout will return this dict from its "build" method if it
                    is not empty. It can be passed later on to the Return function
                    inside a SConscript:
                    toreturn = s.build()
                    Return('toreturn')

    '''
    def __init__(self, origin, werror=False, mksources_path=None,
            mswrapper_factory=None, isabs=None):

        self.origin = origin
        self.mswrapper_factory = mswrapper_factory or MakefileSources
        self.isabs = isabs or os.path.isabs
        self._init_makefile_sources(mksources_path)

        self.werror = werror
        self.libentries = []
        self.tsrecords = []
        self.tmp = {}
        self.toreturn = {}

    def _init_makefile_sources(self, mksources_path):
        if mksources_path is not None and not self.isabs(mksources_path):
            mksources_path = os.path.join(self.origin, mksources_path)

        if mksources_path is not None:
            log.info('reading Makefile sources: %s' % mksources_path)
            self.mksources = self.mswrapper_factory(mksources_path)
        else:
            log.debug('no Makefile sources')
            self.mksources = None

    def add_lib_entry(self, name, sources):
        self.libentries.append((name, sources))


class Scout(object):
    def __init__(self, env, mksources_path=None, dircontext_factory=None,
                werror=True, tasktoggles_factory=None):

        self.dircontext_factory = dircontext_factory or Dircontext
        self.tasktoggles_factory = tasktoggles_factory or TaskToggles

        self.env = env
        self.dircontext = self.dircontext_factory(
                self.env.location.absorig, werror=werror,
                mksources_path=mksources_path)
        self.tasktoggles = self.tasktoggles_factory()

    # Toggle werror.

    def getwerror(self):
        return self.dircontext.werror

    def setwerror(self, flag=True):
        self.dircontext.werror = flag
        self.tasktoggles.enable_task('update_werror')

    werror = property(getwerror, setwerror)

    # Common library building interface.

    def library(self, name, sources):
        self.dircontext.add_lib_entry(name, sources)
        self.tasktoggles.enable_task('build_libraries')

    def mslibrary(self, name, variable_name):
        sources = self.dircontext.mksources.filelist(variable_name)
        self.library(name, sources)

    # Common test building & running interface.

    def tests(self, whitelist=None, blacklist=None, glob_pattern=None,
            executor=None, libs=None, locallib=False):

        if locallib:
            # todo: you can also upgrae env['LIBPATH'] based on that
            log.info('extra ld lib path = %s' % self.env.raw.Dir('.').abspath)
            extra_ld_lib_paths = [self.env.raw.Dir('.').abspath]
        else:
            extra_ld_lib_paths = []

        tsrecord = self.env.factory.tsrecord(whitelist=whitelist,
                blacklist=blacklist, glob_pattern=glob_pattern,
                libs=libs, executor=executor,
                extra_ld_lib_paths=extra_ld_lib_paths)

        self.dircontext.tsrecords.append(tsrecord)
        self.tasktoggles.enable_task('make_tests')

    def runtests(self):
        self.tasktoggles.enable_task('run_tests')

    # Task installation & management.

    def add_task(self, callable, copyenv=False, enabled=True):
        self.tasktoggles.add_task(
                self.create_task(callable, copyenv),
                enabled=enabled)

    def replace_task(self, replaced_task_name, callable, copyenv=False, enabled=True):
        self.tasktoggles.replace_task(
                replaced_task_name,
                self.create_task(callable, copyenv),
                enabled=enabled)

    def create_task(self, callable, copyenv):
        if copyenv:
            env = self.env.clone()
        else:
            env = self.env
        return self.tasktoggles.task(callable, env, self.dircontext)

    # Task running.

    def build(self):
        log.info('Scout starting in %s', self.env.location.absorig)
        self.tasktoggles.run()
        if self.dircontext.toreturn:
            return self.dircontext.toreturn
        return None


# Common tasks for Scout.

def update_werror(env, dircontext):
    if dircontext.werror:
        log.info('setting -Werror')
        env.merge(dict(CCFLAGS=['-Werror']))


def build_libraries(env, dircontext):
    for name, sources in dircontext.libentries:
        mkr = env.factory.libmaker(name)
        log.debug('building library %s' % mkr.name)
        mkr.add_sources(sources)
        mkr.make()


def make_tests(env, dircontext):
    for tsrecord in dircontext.tsrecords:
        env.factory.testsuite(tsrecord).make()
        env.add_test_suite_record(tsrecord)

def run_tests(env, dircontext):
    try:
        tsrecords = env.get_test_suite_records()
    except NoTestsError, e:
        log.warning('Running tests in %s: %s', env.location.orig, str(e))
        return

    for tsrecord in tsrecords:
        sr = env.factory.suiterunner(
                executor=tsrecord.executor,
                skip_passed_tests=env['SKIPPASSED'])
        if tsrecord.extra_ld_lib_paths:
            sr.add_ld_library_paths(*tsrecord.extra_ld_lib_paths)
        logname = tsrecord.name + '.log'
        sr.run(logname, tsrecord.tests)


def newscout(env, werror=True, makefile_sources_path=None, factory=Scout):
    '''
    Produces a Scout object with default tasks added.
    '''
    log.debug('constructing scout with makefile_sources_path = %s' % makefile_sources_path)
    sc = factory(env, makefile_sources_path, werror=werror)
    sc.add_task(update_werror, enabled=True)
    sc.add_task(build_libraries, enabled=False)
    sc.add_task(make_tests, enabled=False)
    sc.add_task(run_tests, enabled=False)
    return sc

