#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import os.path
import logging
log = logging.getLogger(__name__)
import datetime

import pytools

from .location import abspath
from .toolkit import Metabuilder

class ExternalSpawn(object):
    OK_STATUS = 0
    ERROR_STATUS = 1

    def __init__(self, spawn):
        '''
        spawn must be a callable which takes a command string
        and executes it
        '''
        self.spawn = spawn

    def __call__(self, sh, escape, cmd, args, env):
        cmdstring = self._build_cmd_string(args)
        try:
            log.debug('spawning command %s' % cmdstring)
            self.spawn(cmdstring)
            log.debug('finished command %s' % cmdstring)
        except Exception, e:
            log.error('error spawning %s: %s' % (cmdstring, e))
            return self.ERROR_STATUS
        return self.OK_STATUS

    def _build_cmd_string(self, args):
        return ' '.join(args)


def fork_for_dist_build(local_spawn, dist_spawn):
    f = SpawnFork(DistDecider(dist_blacklist=['sed']))
    f.add_output('local', local_spawn)
    f.add_output('dist', dist_spawn)
    return f

class DistDecider(object):
    LOCAL = 'local'
    DIST = 'dist'

    def __init__(self, dist_blacklist=None):
        self.dist_blacklist = dist_blacklist or []

    def __call__(self, args):
        appname = self._appname(args)
        log.debug('Got command %s (appname = %s)' % (args, appname))
        if appname in self.dist_blacklist:
            log.debug('LOCAL spawn: %s, %s', appname, pytools.strsummary(' '.join(args), 30))
            return 'local'
        log.debug('DIST spawn: %s, %s', appname, pytools.strsummary(' '.join(args), 30))
        return 'dist'

    def _appname(self, args):
        return os.path.basename(args[0])

class SpawnFork(object):
    def __init__(self, decider=None):
        self.decider = decider
        self.outputs = {}

    def add_output(self, name, spawn):
        self.outputs[name] = spawn

    def __call__(self, sh, escape, cmd, args, env):
        out = self.decider(args)
        return self.outputs[out](sh, escape, cmd, args, env)



def symlink_action(target, source, env):
    sname = os.path.abspath(str(source[0]))
    tname = os.path.abspath(str(target[0]))
    log.debug('creating symlink %s -> %s' % (tname, sname))
    pytools.force_symlink(sname, tname)


def install_as_symlink(dest, source, env):
    '''
    From Scons manual:

    The function takes the following arguments:
    def install(dest, source, env):
    dest is the path name of the destination file.
    source is the path name of the source file.
    env is the construction environment (a dictionary of construction values) in force for this file installation.
    '''
    pytools.force_symlink(os.path.abspath(str(source)), os.path.abspath(str(dest)))


class TestFactoryBuilderSchema(object):
    '''
    After specifying the path to Generator.pl in __init__,
    you can call make_builder to obtain a builder object
    that invokes Generator.pl in order to transform a .def
    file into a pair of .h/.cpp files containing a factory
    implementation.
    The builder, after being passed a .def file, produces
    a pair of nodes corresponding to the .h/.cpp pair.
    If multiple .def files are supplied to this builder,
    SCons raises an error.
    '''

    SRC_SUFFIX = '.def'
    ARMORS = [('Test', 'Factory.h'), ('Test', 'Factory.cpp')]

    def __init__(self, generator_pl_path):
        self.generator_pl_path = generator_pl_path

    def __call__(self, metabuilder, executor):
        metabuilder.action = self.action
        metabuilder.emitter = self.emitter
        metabuilder.src_suffix = self.SRC_SUFFIX
        metabuilder.add_implicit_deps(self.generator_pl_path)

    def emitter(self, target, source, env):
        '''
        For source like one/two/File.def we get
        target like one/two/File.
        And we return targets like:
        TestFileFactory.h, TestFileFactory.cpp
        (without the preceding path)
        to allow SCons place it in the correct
        place in the variant_dir output tree.
        (SCons just works like that).
        '''
        del target[:]
        basename = os.path.basename(str(source[0]))
        root = os.path.splitext(basename)[0]
        for armor in self.ARMORS:
            target.append(root.join(armor))
        return target, source

    def action(self, target, source, env):

        # (targets are now two files .h/.cpp and thats OK: SCons
        # track the correct dependencies. But we do not need to pass
        # these filenames to Generator.pl: enough to specify
        # the output directory).
        #print('my targets are %s' % map(str, target))
        output_dir = os.path.dirname(str(target[0]))
        if not output_dir:
            output_dir = '.'

        # We need to import some perl code from
        # the directory containing the script (Generator.pl)
        additional_lib_path = os.path.dirname(self.generator_pl_path)

        cmd = ['perl', '-I', additional_lib_path, self.generator_pl_path, str(source[0]), output_dir]
        self.executor(env, cmd)

def TestFactoryBuilder(generator_script_path):
    schema = TestFactoryBuilderSchema(generator_script_path)
    return Metabuilder(schema).build()



class DataHandleBuilderSchema(object):
    TARGETS = {'CreateDHCpp': 'DataHandle.cpp',
               'CreateDHMockH': 'DataHandleMock.h',
               'CreateDHMockCpp': 'DataHandleMock.cpp'}

    def __init__(self, data_handle_gen_app, creation_type):
        self.data_handle_gen_app = data_handle_gen_app
        self.creation_type = creation_type
        self.target = self.TARGETS[self.creation_type]


    def __call__(self, metabuilder, executor):
        metabuilder.action = self.action
        metabuilder.emitter = self.emitter
        metabuilder.add_implicit_deps(self.data_handle_gen_app)


    def action(self, target, source, env):
        cmd = [
            str(self.data_handle_gen_app),
            '--configFile', str(source[0]),
            '--{ctype}'.format(ctype=self.creation_type),
            '--OutputDir', os.path.dirname(str(target[0])) + os.sep]

        self.executor(env, cmd)

    def emitter(self, target, source, env):
        del target[:]
        target.append(self.target)
        return target, source


def DataHandleBuilder(data_handle_gen_app, creation_type):
    schema = DataHandleBuilderSchema(data_handle_gen_app, creation_type)
    return Metabuilder(schema).build()


class UtexeRunnerSchema(object):
    def __init__(self, utexe_app, starting_dir, timestamp=None, text_to_file=None):
        self.utexe_app = utexe_app
        self.starting_dir = starting_dir
        self.timestamp = timestamp or datetime.datetime.now
        self.text_to_file = text_to_file or self._text_to_file

    def __call__(self, metabuilder, executor):
        metabuilder.action = self.action
        metabuilder.add_implicit_deps(self.utexe_app)
        executor.cwd = os.path.join(self.starting_dir, 'test')

    def _text_to_file(self, text, filename):
        f = open(filename, 'w')
        try:
            f.write(text)
        finally:
            f.close()

    def action(self, target, source, env):
        '''
        The target is a file to write a timestamp to.
        '''
        cmd = [self.utexe_app]
        for s in source:
            cmd.append('-L')
            cmd.append(abspath(s))
        # Add -jxml argument for coverage build
        if 'TESTS_REPORTS_DESTINATION_' in env:
            xml_dir = env['TESTS_REPORTS_DESTINATION_']
            suite_name = os.path.basename(self.starting_dir)
            xml_path = os.path.join(xml_dir, suite_name + '.xml')
            cmd.append('--jxml')
            cmd.append(xml_path)
        self.executor(env, cmd)
        self.text_to_file(str(self.timestamp()) + '\n', str(target[0]))


def UtexeRunner(utexe_app, starting_dir):
    schema = UtexeRunnerSchema(utexe_app, starting_dir)
    return Metabuilder(schema).build()


