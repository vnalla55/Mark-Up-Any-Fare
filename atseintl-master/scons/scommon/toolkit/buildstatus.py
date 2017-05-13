#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013

'''
Based on Scons manual.
'''
from __future__ import print_function

import ast
import functools
import re
import sys

RE_UNIT_TEST_FAILURE = re.compile(r'^action\(\["[^"]*"\], (\[[^\]]*\])\)$')

def command_to_str(command):
    if type(command) is list:
        return ' '.join(command)

    if type(command) is not str:
        command = str(command)

    m = RE_UNIT_TEST_FAILURE.match(command)
    if m is not None:
        # Special case: unit test failure.
        parts = ast.literal_eval(m.group(1))

        utexe = parts[-1]
        command = ' '.join([utexe] + ['-L %s' % lib for lib in parts[:-1]])

    return command

class BuildFailureAnalyzer(object):
    def __init__(self, messages):
        self.messages = messages

    class Analysis(object):
        def __init__(self, message, command=None):
            self.message = message
            self.command = command

    def analyze(self, bf):
        import SCons.Errors
        if bf is None:
            return self.Analysis('(unknown target)')

        if isinstance(bf, SCons.Errors.StopError):
            return self.Analysis(str(bf))

        if bf.node:
            if bf.command is None:
                # If build was interrupted, command is None.
                return self.Analysis('%s: %s' % (bf.node, bf.errstr))

            command = command_to_str(bf.command)

            message = '%s.\nCommand: %s' %\
                (bf.node, command)

            if self.messages is not None:
                additional = self.messages.get(bf.command)
                if additional is not None:
                    message += '\n\n%s' % additional

            return self.Analysis(message, command=command)

        if bf.filename:
            return self.Analysis(bf.filename + ': ' + bf.errstr)

        return self.Analysis('unknown failure: ' + bf.errstr)

def build_status(messages, display_warnings=False):
    """Convert the build status to a 2-tuple, (status, msg)."""
    from SCons.Script import GetBuildFailures
    bf = GetBuildFailures()

    files = set()
    analyzer = BuildFailureAnalyzer(messages)
    additional_message = ''

    if bf:
        # bf is normally a list of build failures; if an element is None,
        # it's because of a target that scons doesn't know anything about.
        status = 'failed'
        for failure in bf:
            analysis = analyzer.analyze(failure)

            if analysis.command is not None:
                files.add(analysis.command)

            if len(additional_message) != 0:
                additional_message += "\n"
            additional_message += "SCONS FAILURE: failed building %s\n" %\
                analysis.message
    else:
        # if bf is None, the build completed successfully.
        status = 'ok'

    if display_warnings:
        warnings = []

        for command, message in messages.get_all():
            command = command_to_str(command)

            if command in files:
                # This command already failed.
                continue

            warnings.append('%s:\n\n%s' % (command, message))

        if len(warnings) > 0:
            additional_message = 'Warnings:\n\n%s\n%s' %\
                ('\n'.join(warnings), additional_message)

    return (status, additional_message)

def display_build_status_common(printer,
                                messages=None,
                                display_warnings=False,
                                file=sys.stdout):
    """Display the build status.  Called by atexit."""
    status, additional_message = build_status(messages, display_warnings)

    if len(additional_message) > 0:
        printer(additional_message, file=file)

    if status == 'ok':
       print("scons final status: [ OK ]", file=file)
    else:
       print("scons final status: [ FAILURE ]", file=file)

def display_build_fail_status_basic(message, file):
    print('-------------', file=file)
    print(message, file=file)

def display_build_fail_status_ext(message, file):
    from colors import colors
    print(colors['fail'], file=file)
    print('-------------', file=file)
    print(message, file=file)
    print(colors['clear'], file=file)

def get_display_build_status(ext_log=False,
                             **kwargs):
    if ext_log:
        printer = display_build_fail_status_ext
    else:
        printer = display_build_fail_status_basic

    return functools.partial(display_build_status_common,
                             printer=printer,
                             **kwargs)
