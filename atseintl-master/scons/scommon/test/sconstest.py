#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import unittest

import os
import sys
import subprocess

class SconsFileLine(object):
    def __init__(self, *line_data):
        self.text = line_data[0]
        try:
            self.files_produced = line_data[1]
        except IndexError:
            self.files_produced = 0

class LineformBroker(object):
    IMPORT = "import %s"
    ENV_CREATION = "env = Environment()"
    PROGRAM_BUILD = "{variable} = env.Program('{appname}', ['{source_file}'])"

    def import_line(self, module_name):
        return (self.IMPORT % module_name, 0)

    def env_creation_line(self):
        return (self.ENV_CREATION, 0)

    def program_creation_line(self, appname, source_file, variable=None):
        if variable is None:
            variable = appname
        # two files created:
        # object file
        # app file
        return (self.PROGRAM_BUILD.format(variable=variable, appname=appname, source_file=source_file), 2)

class SconsFile(object):
    def __init__(self, line_factory=None, lineforms_broker=None):
        self.lines = []
        self.line_factory = SconsFileLine if line_factory is None else line_factory
        self.lineforms_broker = LineformBroker() if lineforms_broker is None else lineforms_broker

    def append(self, *line_data):
        self.lines.append(self.line_factory(*line_data))

    def add_import(self, module_name):
        self.append(*self.lineforms_broker.import_line(module_name))

    def add_env_creation(self):
        self.append(*self.lineforms_broker.env_creation_line())

    def add_program_creation(self, appname, source_file, variable=None):
        self.append(*self.lineforms_broker.program_creation_line(appname, source_file, variable))

    def get_lines(self):
        return self.lines

    def get_text(self):
        return '\n'.join([l.text for l in self.get_lines()])

    def files_produced(self):
        return sum([l.files_produced for l in self.get_lines()])


class SconsTest(unittest.TestCase):
    VERBOSE_ARG = 'verbose'

    def __init__(self, runner_factory, sweeper_factory, *args, **kwargs):
        if self.VERBOSE_ARG in kwargs:
            self.verbose = kwargs[self.VERBOSE_ARG]
            del kwargs[self.VERBOSE_ARG]
        else:
            self.verbose = False
        super(SconsTest, self).__init__(*args, **kwargs)
        self.runner_factory = runner_factory
        self.sweeper_factory = sweeper_factory
        self.expected_del_count = 0
        self.extra_scons_args = []

    # To be overloaded by child classes, if necessary
    def up(self): pass
    def down(self): pass

    def create_script(self):
        return SconsFile()

    def run_file(self, scons_file=None, pythonpath_suffix=None):
        if scons_file is None:
            scons_file = self.sf
        if pythonpath_suffix is not None:
            self.runner.append_to_pythonpath(pythonpath_suffix)
        text = scons_file.get_text()
        if self.verbose:
            self._emit_text('USING TEXT\n%s' % text)
        try:
            self.output = self.runner.run(text, self.extra_scons_args)
        except subprocess.CalledProcessError, e:
            self._emit_text('CalledProcessError: %s, %s, %s' % (e.returncode, e.cmd, e.output))
            raise

        if self.verbose:
            self._emit_text('GOT OUTPUT\n%s' % self.output)
        self.expected_del_count = scons_file.files_produced()

    def assert_output_contains(self, text):
        if not self.text_contains(self.output, text):
            self._emit_error('%s not found in\n%s' % (text, self.output))
            self.assertTrue(0)

    def assert_file_exists(self, path):
        self.assertTrue(os.path.exists(os.path.join(self.builddir(), path)))

    def output_line_containing(self, text):
        for line in self.output.splitlines():
            if self.text_contains(line, text):
                return line
        raise ValueError('No line with text %s' % text)

    def builddir(self):
        return self.runner.get_builddir()

    def setUp(self):
        self.runner = self.runner_factory()
        self.sweeper = self.sweeper_factory(self.builddir())
        self.sf = self.create_script()
        self.up()

    def tearDown(self):
        self.down()
        removed_files = self.sweeper.sweep()
        if self.verbose:
            for rf in removed_files:
                self._emit_text('removed file %s' % rf)
        if len(removed_files) != self.expected_del_count:
            self._emit_error('Expected %s files to delete. Got %s' % (self.expected_del_count, removed_files))
            self.assertTrue(0)

    def text_contains(self, text, phrase):
        return text.find(phrase) != -1

    def add_extra_scons_args(self, *args):
        self.extra_scons_args.extend(args)

    def _emit_text(self, text):
        print text

    def _emit_error(self, text):
        sys.stderr.write(text)
        sys.stderr.write('\n')
