#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import collections
import logging
import os
log = logging.getLogger(__name__)


from .factory import Factory
from . import bridge
from . import utils
from . import location
from .scout import newscout
from .exceptions import EnvironmentError, SconsRuntimeError
from .logtool import Logtool


def make_summary(env, width=20):
    ret = []
    format = '%-' + str(width) + 's - %s'
    for k in ['CC', 'CXX']:
        ret.append(format % (k, env[k]))
    return '\n'.join(ret)


class Environment(collections.MutableMapping):

    DEFAULT_MS_FILENAME = 'Makefile.sources'

    def __init__(self, renv, stash=None, makesummary=None,
        node_factory=None, env_to_factory_config=None,
        logtool_factory=None):

        self.renv = renv
        self.makesummary = makesummary or make_summary
        self.stash = {} if stash is None else stash
        self.node_factory = node_factory or location.Node
        self.env_to_factory_config = env_to_factory_config \
            or self._simple_env_to_factory_config
        self.logtool_factory = logtool_factory or Logtool
        self.logtool = self.logtool_factory()

    def script_reading_msg(self):
        self.log.info(self.logtool.script_reading_msg)

    # Env manipulation

    def merge(self, d):
        '''
        Merges a regular dictionary to this environment.
        '''
        utils.merge(self, d)

    def append(self, key, val):
        log.info('appending %s to %s=%s', utils.pformat(val), key, utils.pformat(self.get(key)))
        self.renv.Append(**{key: val})

    def append_unique(self, key, val):
        log.info('unique appending %s to %s=%s', utils.pformat(val), key, utils.pformat(self.get(key)))
        self.renv.AppendUnique(**{key: val})

    def subst(self, text):
        return self.renv.subst(text)

    def clone(self):
        return Environment(
            self.renv.Clone(), self.stash,
            makesummary=self.makesummary,
            node_factory=self.node_factory,
            env_to_factory_config=self.env_to_factory_config)

    def add_builder(self, name, builder):
        self.append('BUILDERS', {name : builder})

    # glob

    def glob(self, globexpr, excludes=None):
        return utils.glob(self.raw, globexpr, excludes)

    def multiglob(self, *expressions, **kwargs):
        return utils.multiglob(self.raw, *expressions, **kwargs)

    def splitsources(self, sources):
        return bridge.split_sources(sources)

    # Convenience methods for the most commonly used env variables

    def add_flags(self, *flags):
        '''
        Adds general options that are passed to the C and C++ compilers.
        '''
        self.merge(dict(CCFLAGS = list(flags)))

    def add_include_paths(self, *paths):
        '''
        Adds header files search paths. They will be passed
        using the -I option.
        '''
        self.merge(dict(INCLUDE_PATH_ = list(paths)))

    def add_isystem_paths(self, *paths):
        '''
        Adds header files search paths, but they will be
        passed using the -isystem option.
        '''
        self.merge(dict(ISYSTEM_PATH_ = list(paths)))

    def add_libs(self, *libnames):
        '''
        Adds libraries with given names to link with. They will
        be passed using the -l option.
        '''
        self.merge(dict(LIBS = list(libnames)))

    def add_libs_whole_archive(self, *libnames):
        raise NotImplementedError('TBD')

    def add_libpaths(self, *paths):
        '''
        Adds library search paths. They will be passed using
        the -L option.
        '''
        self.merge(dict(LIBPATH = list(paths)))

    def add_libs_from(self, path, *libnames):
        self.add_libpaths(path)
        self.add_libs(*libnames)

    def add_linkflags(self, *flags):
        '''
        Adds general user options passed to the linker.
        '''
        self.merge(dict(LINKFLAGS = list(flags)))


    def add_defines(self, *defines):
        '''
        Adds Cpp defines. Each element in the input list may be:
        - a string, e.g. 'ABC', which results in definition -DABC
        - a tuple (key, value), e.g. ('ABC', 'Something')
          which results in definition -DABC=Something
        '''
        self.merge(dict(CPPDEFINES= list(defines)))

    @property
    def sysenv(self):
        return self['ENV']

    def add_ld_library_paths(self, *paths):
        '''
        Extends the system LD_LIBRARY_PATH variable
        that contains paths to libraries loaded in runtime.
        '''
        if paths:
            allpaths = [self.sysenv.get('LD_LIBRARY_PATH', '')] + [p.strip() for p in paths]
            joined = os.pathsep.join(allpaths).strip(os.pathsep)
            self.sysenv['LD_LIBRARY_PATH'] = joined

    def add_rpaths(self, *paths):
        '''
        Adds rpath (runtime library search paths). They will be passed using
        the -Wl,-rpath option.
        '''
        self.merge(dict(RPATH = list(paths)))

    # Two types of Scout object (with and without makefile sources).

    def scout(self, envdict_to_merge=None,
            makefile_sources_path=None, emit_msg=True):
        scout_env = self.clone()
        if envdict_to_merge is not None:
            scout_env.merge(envdict_to_merge)
        if emit_msg:
            self.script_reading_msg()
        return newscout(scout_env, werror=self.werror,
                makefile_sources_path=makefile_sources_path)

    def msscout(self, envdict_to_merge=None,
            makefile_sources_path=None, emit_msg=True):
        if makefile_sources_path is None:
            makefile_sources_path = self.DEFAULT_MS_FILENAME
        return self.scout(envdict_to_merge, makefile_sources_path)


    def getlinktype(self):
        return self['LINKTYPE']

    def setlinktype(self, type):
        self['LINKTYPE'] = type

    linktype = property(getlinktype, setlinktype)

    def get_direct_object_link(self):
        return self.get('DIRECT_OBJECT_LINK_')

    def set_direct_object_link(self, olink):
        self['DIRECT_OBJECT_LINK_'] = olink

    direct_object_link = property(get_direct_object_link, set_direct_object_link)

    def getinstalldir(self):
        return self.get('LIB_INSTALL_DIR_', None)

    def setinstalldir(self, dir):
        self['LIB_INSTALL_DIR_'] = None

    installdir = property(getinstalldir, setinstalldir)

    @property
    def location(self):
        return self.node_factory(self.raw.Dir('.'))

    @property
    def absroot(self):
        return self.node_factory(self.raw.Dir(location.SCONS_ROOT)).absdest

    @property
    def builddir(self):
        return self.subst('$BUILD_DIR_')

    @property
    def absbuilddir(self):
        return os.path.join(self.absroot, self.builddir)

    def node(self, scons_node):
        return self.node_factory(scons_node)

    def getwerror(self):
        return self['WERROR_']
    def setwerror(self, flag):
        self['WERROR_'] = flag
    werror = property(getwerror, setwerror)

    @property
    def factory(self):
        return Factory(self, config=self.env_to_factory_config(self))

    @property
    def log(self):
        return self.logger()
    def logger(self, origin_basename=None):
        return self.logtool.getlogger(self.location.orig,
                origin_basename=origin_basename)

    # Tests
    TESTBANK_KEY = 'testbank_'
    def gettestbank(self):
        return self.stash[self.TESTBANK_KEY]
    def settestbank(self, bank):
        self.stash[self.TESTBANK_KEY] = bank
    testbank = property(gettestbank, settestbank)

    def add_test_suite_record(self, tsrecord):
        self.testbank.add_tsrecord(self.location.orig, tsrecord)

    def get_test_suite_records(self):
        return self.testbank.get_tsrecords(self.location.orig)


    @property
    def raw(self):
        return self.renv

    @property
    def summary(self):
        return self.makesummary(self)

    def error(self, msg):
        raise SconsRuntimeError(msg)

    # Configuration

    def set_env_to_factory_config_callback(self, callback):
        self.env_to_factory_config = callback

    def _simple_env_to_factory_config(self, env):
        return dict(linktype = env.linktype, installdir = env.installdir)

    # Covering the mutable mapping interface.

    def __getitem__(self, key):
        return self.renv[key]

    def __setitem__(self, key, value):
        log.info('replacing %s=%s with %s', key, utils.pformat(self.get(key)), utils.pformat(value))
        self.renv[key] = value

    def __delitem__(self, key):
        raise EnvironmentError('deletion for Environment not supported')

    def __iter__(self):
        return iter(self.renv.Dictionary())

    def __len__(self):
        return len(self.renv.Dictionary())

