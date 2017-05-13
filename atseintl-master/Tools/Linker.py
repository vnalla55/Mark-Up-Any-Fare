#!/usr/bin/env python

from __future__ import print_function

import json
import os
import re
import subprocess
import sys
import tempfile
from collections import OrderedDict


def printErr(*args):
    """
    Helper function to print to stderr.
    """

    print(*args, file=sys.stderr)


def listArg(list=None):
    """
    Converts None to empty list.
    """

    if list is None:
        return []
    return list


def getArgumentsLimit():
    """
    Returns the maximal amount of bytes the command line arguments on current
    system can use.
    """

    child = subprocess.Popen(['getconf', 'ARG_MAX'], stdout=subprocess.PIPE)
    limit = int(child.stdout.read())
    child.wait()

    # The returned limit is the maximal amount of bytes the arguments and
    # environment variables can use in Linux process startup. Excluding
    # arguments and environment variables, it consists of null characters,
    # `=' characters, etc. The following heurestic reduces this limit
    # by 15% to account for these additional data.
    return int(limit * 0.85)


# Configuration constants.
ARGUMENTS_LIMIT = getArgumentsLimit()
MODULE_SEARCH_PATH = os.path.normpath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), '..'))


def isPathInModuleSearchPath(path):
    """
    Check whether the path should be searched for *.module files.

    Currently, *.module files can only reside in MODULE_SEARCH_PATH and
    its subdirectories.
    """

    msp = os.path.normpath(os.path.abspath(MODULE_SEARCH_PATH))
    path = os.path.normpath(os.path.abspath(path))
    return os.path.commonprefix([msp, path]) == msp


def executeCommand(program, args):
    """
    Execute command, optionally using @file if argument lists exceeds limit.
    """

    merged = ' '.join(args)
    listFile = None

    try:
        if len(program) + 1 + len(merged) > ARGUMENTS_LIMIT:
            with tempfile.NamedTemporaryFile(delete=False) as file:
                listFile = file.name
                file.write(merged)
            args = ['@'+listFile]

        result = subprocess.call([program] + args)

    finally:
        if listFile is not None:
            os.unlink(listFile)

    return result


class Arguments:
    """
    Command line arguments manipulation class.
    """

    class Error(Exception):
        """
        Exception indicating wrong arguments.
        """

    @staticmethod
    def usage():
        return """Linker.py: Linker wrapper supporting static linking.

Usage:
    %s <linker executable> [<linker argument>|<Linker.py option>]...

Linker.py options:
    --help              Print this message
    --linker-verbose    Verbose output from Linker.py

Recognized linker options:
    -o <output name>    Output file location
    -l <library name>   Required library name
    -L <libdir name>    Path where to look for libraries
""" % sys.argv[0]

    def __init__(self):
        if len(sys.argv) < 2:
            raise Arguments.Error('Too few arguments')
        self.linker = sys.argv[1]
        self.rest = sys.argv[2:]
        self.verbose = False
        self.help = False

        if self.cutOptions('--linker-verbose') > 0:
            self.verbose = True
        if self.cutOptions('--help') > 0:
            self.help = True

    class SimpleArgumentFilter:
        def __init__(self, predicate):
            self.predicate = predicate
            self.values = []

        def __call__(self, arg):
            if self.predicate(arg):
                self.values.append(arg)
                return False
            else:
                return True

    class ContinuationArgumentFilter:
        def __init__(self, option):
            self.option = option
            self.values = []
            self.needsArgument = False

        def __call__(self, arg):
            if self.needsArgument:
                self.needsArgument = False
                self.values.append(arg)
                return False

            if not arg.startswith(self.option):
                return True

            arg = arg[len(self.option):].strip()
            if len(arg) == 0:
                self.needsArgument = True
            else:
                self.values.append(arg)
            return False

        @property
        def isComplete(self):
            return not self.needsArgument

    def _cut(self, argumentFilter):
        self.rest = filter(argumentFilter, self.rest)
        return argumentFilter.values

    def cutFreeArguments(self):
        """Cut non-option arguments and return them."""
        return self._cut(Arguments.SimpleArgumentFilter(
            lambda arg: not arg.startswith('-')))

    def cutOptions(self, option):
        """Cut options and return their count."""
        return len(self._cut(Arguments.SimpleArgumentFilter(
            lambda arg: arg == option)))

    def cutShortOptionsWithArgument(self, option):
        """Cut options with argument and return the arguments."""
        argumentFilter = Arguments.ContinuationArgumentFilter(option)
        values = self._cut(argumentFilter)

        if not argumentFilter.isComplete:
            raise Arguments.Error(
                "Option `%s' without argument at the end of line" % option)

        return values

    def cutSingleShortOptionWithArgument(self, option):
        """Cut a single option with argument and return the argument."""
        values = self.cutShortOptionsWithArgument(option)
        if len(values) != 1:
            raise Arguments.Error("Expected single option `%s', found %d" %
                                  (option, len(values)))
        return values[0]


class Module:
    """
    In-memory representation of *.module file.

    Module is a "fake" library that wraps its linking arguments in a file
    to use it at later time. It behaves similarly to shared library,
    but the generated file is not self-contained (the objects are
    untouched) and it doesn't require position independent code (-fPIC).

    When linking into module, the objects, libraries and library
    directories are recorded in the file. Only the names of these
    files/directories are used, i.e. no objects are copied.

    Later, when "real" binary is being linked (like executable or shared
    library) all the modules it links against (either directly
    or indirectly) are resolved. The resolve process retrieves all
    the objects and real libraries of these modules, uniques them and
    feeds them to the real linker.

    Using modules instead of static or shared libraries increases speed,
    because no objects are unnecessarily copied/relocated.
    From the point of view of real linker, all the composing objects
    are given as command line arguments when linking the final executable.
    """

    class ParseError(Exception):
        """
        Exception raised on parse error.
        """

        def __init__(self, message, file=None, line=None):
            self.originalMessage = message
            self.file = file
            self.line = line

            if file is None:
                file = '<stdin>'
            if line is None:
                Exception.__init__(self, '%s: %s' % (file, message))
            else:
                Exception.__init__(self, '%s:%d: %s' % (file, line, message))

    _SetPattern = re.compile(r'^\s*(\w+)\s*=(.+)$')
    _ModulePattern = re.compile(r'^lib([\w_-]+)\.module$')
    _Names = dict(SOURCES='sources', LIBS='libs', LIBDIRS='libdirs')
    _Schema = """# This file serves as a fake library.

SOURCES = {0}

LIBS = {1}

LIBDIRS = {2}
"""

    @staticmethod
    def moduleName(path):
        """
        Get module name from path (lib*.module).
        """

        m = Module._ModulePattern.match(os.path.basename(path))
        if m is None:
            return None
        return m.group(1)

    @staticmethod
    def isPathModule(path):
        """
        Whether a path is a module.
        """

        return Module.moduleName(path) is not None

    @staticmethod
    def fileName(name):
        """
        Get file name from module name.
        """

        return 'lib'+name+'.module'

    @staticmethod
    def fromString(str, fileName=None):
        """
        Construct module from file contents.
        """

        module = dict()

        for (nr, line) in enumerate(str.split('\n'), start=1):
            line = line.partition('#')[0].strip()
            if len(line) == 0:
                continue

            m = Module._SetPattern.match(line)
            if m is None:
                raise Module.ParseError('Line is not an assignment',
                                        file=fileName, line=nr)

            name, value = m.group(1, 2)
            if name in module:
                raise Module.ParseError(name + ' has already been assigned',
                                        file=fileName, line=nr)

            try:
                name = Module._Names[name]
            except KeyError:
                raise Module.ParseError('Unknown property: ' + name,
                                        file=fileName, line=nr)

            try:
                value = json.loads(value)
            except Exception as e:
                raise Module.ParseError('Cannot decode JSON: %s' % e,
                                        file=fileName, line=nr)

            try:
                value = [s.encode('utf-8') for s in value]
            except Exception:
                raise Module.ParseError(
                    'Assignment value is not valid JSON array of strings',
                    file=fileName, line=nr)

            module[name] = value

        return Module(**module)

    @staticmethod
    def fromFile(fileName):
        """
        Construct module from file.
        """

        with open(fileName, 'r') as file:
            return Module.fromString(file.read(), fileName)

    def __init__(self, sources=None, libs=None, libdirs=None):
        self.sources = listArg(sources)
        self.libs = listArg(libs)
        self.libdirs = listArg(libdirs)

    def save(self, fileName):
        """
        Save module to file.
        """

        with open(fileName, 'w') as file:
            file.write(Module._Schema.format(
                json.dumps(self.sources),
                json.dumps(self.libs),
                json.dumps(self.libdirs)))


class Target:
    """
    Makefile target.

    It represents output file with a method to create it.
    """

    _LibPattern = re.compile(r'^lib([\w_-]+)\.(so|module)$')

    def __init__(self, root, output, sources, libs, libdirs):
        self.output = os.path.normpath(os.path.join(root, output))
        self.sources = [os.path.normpath(os.path.join(root, path))
                        for path in sources]
        self.libs = OrderedDict([(lib, True) for lib in libs])
        self.libdirs = [os.path.normpath(os.path.join(root, path))
                        for path in libdirs]

    def _resolveModule(self, omit, lib, fileName):
        module = Module.fromFile(fileName)

        omit.add(lib)
        target = ModuleTarget.fromModule(fileName, module)
        target.resolveLibs(omit)

        self.libs.update(target.libs)
        self.libdirs.update([(libdir, True) for libdir in target.libdirs])
        self.sources.extend(target.sources)

        for lib in target.libs:
            omit.add(lib)

    def resolveLibs(self, omit=None):
        """
        Resolve indirect library dependencies.
        """

        if omit is None:
            omit = set()

        m = Target._LibPattern.match(os.path.basename(self.output))
        if m is not None:
            omit.add(m.group(1))

        old_libs = self.libs
        old_libdirs = self.libdirs
        self.libs = OrderedDict()
        self.libdirs = OrderedDict([(libdir, True) for libdir in self.libdirs])

        for lib in old_libs.iterkeys():
            if lib in omit:
                continue

            for libdir in old_libdirs:
                if not isPathInModuleSearchPath(libdir):
                    continue

                try:
                    fileName = os.path.join(libdir, Module.fileName(lib))
                    self._resolveModule(omit, lib, fileName)
                except IOError:
                    continue

                break
            else:
                self.libs[lib] = True
                omit.add(lib)

        modules = filter(Module.isPathModule, self.sources)
        for fileName in modules:
            self.sources.remove(fileName)
            lib = Module.moduleName(fileName)
            if not lib in omit:
                self._resolveModule(omit, lib, fileName)

        self.libdirs = self.libdirs.keys()

    def relativePaths(self, root=os.curdir):
        """
        Return relative paths of sources and libdirs.
        """

        sources = [os.path.relpath(source, root) for source in self.sources]
        libdirs = []
        for libdir in self.libdirs:
            if isPathInModuleSearchPath(libdir):
                libdir = os.path.relpath(libdir, root)
            libdirs.append(libdir)

        return (sources, libdirs)


class NativeTarget(Target):
    """
    A target built using native linker.
    """

    def execute(self, args):
        cmd = args.rest[:]
        cmd.extend(['-o', self.output])

        sources, libdirs = self.relativePaths()

        cmd.extend(sources)
        cmd.extend(['-L'+libdir for libdir in libdirs])
        cmd.extend(['-l'+lib for lib in self.libs])

        if args.verbose:
            printErr(args.linker, repr(cmd))
            printErr("Arguments: ", len(cmd))
            printErr("Length: ", len(" ".join(cmd)))
        return executeCommand(args.linker, cmd)


class ModuleTarget(Target):
    """
    A target representing module.
    """

    @staticmethod
    def fromModule(fileName, module):
        root = os.path.dirname(fileName)
        name = os.path.basename(fileName)
        return ModuleTarget(root, name,
                            module.sources, module.libs, module.libdirs)

    def execute(self, args):
        root = os.path.dirname(output)

        sources, libdirs = self.relativePaths(root)

        module = Module(
            sources,
            [lib for lib in self.libs],
            libdirs
        )

        module.save(self.output)
        return 0


def createTarget(root, output, sources, libs, libdirs):
    """
    Choose target type, depending on output file name.
    """

    if Module.isPathModule(output):
        return ModuleTarget(root, output, sources, libs, libdirs)
    else:
        return NativeTarget(root, output, sources, libs, libdirs)


if __name__ == '__main__':
    try:
        args = Arguments()
        if args.help:
            print(Arguments.usage())
            exit(0)

        output = args.cutSingleShortOptionWithArgument('-o')
        libs = args.cutShortOptionsWithArgument('-l')
        libdirs = args.cutShortOptionsWithArgument('-L')
        sources = args.cutFreeArguments()

    except Arguments.Error as e:
        printErr(e.message)
        printErr(Arguments.usage())
        exit(1)

    target = createTarget(os.getcwd(), output, sources, libs, libdirs)
    if isinstance(target, NativeTarget):
        target.resolveLibs()

    exit(target.execute(args))
