#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013/2014

import os
import sys
import atexit
import logging
import re

import pytools.command
import pytools.log

import config

import scommon

log = logging.getLogger(__name__)


# global tools like Copy etc.
from SCons.Script import *

# general

def merge_config_dict(env):
    env.merge(config.ENV)

def handle_third_party(env):
    if env['THIRDPARTY']:
        env['THIRD_PARTY_LIBDIR_'] = env['THIRDPARTY']

def handle_binutils(env):
    if env.raw.has_key('BINUTILS'):
        env['BINUTILS_DIR_'] = env['BINUTILS']
    else:
        env['BINUTILS'] = env['BINUTILS_DIR_']

def handle_destination_directory(env):
    if env.raw.has_key('BUILDDIR'):
        env['BUILD_DIR_'] = env['BUILDDIR']
    else:
        env['BUILDDIR'] = env['BUILD_DIR_']

def read_redhat_version(env):
    with open('/etc/redhat-release') as f:
        banner = f.read()
        match = re.search(r'release\s+(\d+)\.?(\d+)?\s+', banner)
        major_version = int(match.group(1))
        if major_version not in [6]:
            env.error('Unknown Red Hat version = %s' % major_version)
        log.debug('redhat version = %s', major_version)
        env['REDHAT_VERSION_'] = major_version

def make_rh_version_dependant_setup(env):
    version = env['REDHAT_VERSION_']
    env.add_defines(('REDHAT_VERSION', version))
    env.merge(env['RH_VERSION_DEPENDENT_LIBS_'][version])

def handle_compiler_version(env):
    def matches(pattern, actual):
      re_pattern = re.escape(pattern).replace(r'\*', r'.*')
      return re.match(re_pattern, actual) != None

    compiler_version = env['COMPILER']

    for (pattern, settings) in env['COMPILERS_']:
      if matches(pattern, compiler_version):
        env.merge(settings)

def handle_cpp_standard_version(env):
    if env.raw.has_key('CPPSTD'):
        env['CPPSTD_'] = env['CPPSTD']
    else:
        env['CPPSTD'] = env['CPPSTD_']

def set_build_label(env):
    env.add_defines(('BUILD_LABEL', env.subst('$BUILD_LABEL')))

def enable_cache(env):
    path = env.subst('$CACHE_DIR_')
    if path:
        log.debug('setting cache directory to %s' % path)
        scommon.bridge.set_cache_directory(path)

def enable_symlink_install(env):
    env['INSTALL'] = scommon.plugins.install_as_symlink
    b = Builder(action = scommon.plugins.symlink_action)
    env.add_builder('Symlink', b)

def install_cpp_test_factory_builder(env):
    '''
    A.k.a. "Generator.pl"
    Builder accessible as env.raw.CppTestFactoryBuilder
    '''
    generator_path = os.path.join(env.absroot,
        env.subst('$GENERATOR_PL_RELATIVE_PATH_'))
    log.debug('Installing Cpp test factory builder a.k.a. Generator.pl from path %s', generator_path)
    b = scommon.plugins.TestFactoryBuilder(generator_path)
    env.add_builder('CppTestFactoryBuilder', b)


def disable_source_code(env):
    # some minor performance opt, see manual
    env.raw.SourceCode('.', None)

def setup_c_compiler(env):
    # Compile C with g++
    env['CC'] = env.subst('$CXX')
    # use append since we have two elements
    log.debug('cflags before C compiler setup: %s' % env['CFLAGS'])
    env.append('CFLAGS', ['-x', 'c'])
    log.debug('cflags after: %s' % env['CFLAGS'])

def set_umask(env):
    # umask 0002 = 2
    # to enable write acces for group
    os.umask(2)


def handle_build_type(env):
    buildtype = env['TYPE']
    env.merge(env['BUILD_TYPES_'][buildtype])

def setup_aliases(env):
    # Second server dependency is only to support legacy build directory
    #   - should be removed as soon as new way proof it's flexibility
    env.raw.Alias('server', [os.path.join(env.builddir, 'bin'),
                             'bin/debug'])
    env.raw.Alias('ldctools', [os.path.join(env.builddir, 'Tools/ldc'),
                               os.path.join(env.builddir, 'Tools/ldc/ldc-utils')])
    env.raw.Alias('package', ['server', 'ldctools'])
    # there are some problems when mixing variant dir
    # and aliases, e.g. env.raw.Alias('tests-run', 'testroot/run/')
    # does not work for some reason (i.e. nothing is built).
    env.raw.Alias('tests-lib', os.path.join(env.builddir, 'test/'))
    env.raw.Alias('tests-build', ['tests-lib', os.path.join(env.builddir, 'testroot/bld/')])
    env.raw.Alias('tests-run', os.path.join(env.builddir, 'testroot/run/'))
    env.raw.Alias('tests', 'tests-run')
    env.raw.Alias('all', ['server', 'ldctools', 'tests'])
    Default('server')

def setup_script_dir_selector(env):

    def selector(build_dirs, tools_build_dirs):
        s = scommon.sdselector.ScriptDirSelector()
        s.add_dir_group('build', *build_dirs)
        s.add_dir_group('tools', *tools_build_dirs)

        #dependencies for server and tools were separated but contains common elements
        # duplicate are not allowed so they need to be eliminated
        # TODO: Add appropriate method to pytools (?)
        resulting_list = list(build_dirs)
        resulting_list.extend(x for x in tools_build_dirs if x not in resulting_list)
        s.add_dir_group('pack-all', *resulting_list)

        s.add_dir_group('test', 'test')
        s.add_dir_group('testroot', 'testroot')

        name_algo = scommon.sdselector.TargetNameAlgorithm()
        name_algo.add_groups_for_target('server', 'build')
        name_algo.add_groups_for_target('ldctools', 'tools')
        name_algo.add_groups_for_target('package', 'pack-all')
        name_algo.add_groups_for_target('tests-lib', 'build', 'test')
        name_algo.add_groups_for_target('tests-build', 'build', 'test', 'testroot')
        name_algo.add_groups_for_target('tests-run', 'build', 'test', 'testroot')
        name_algo.add_groups_for_target('tests', 'build', 'test', 'testroot')
        name_algo.add_groups_for_target('all', 'build', 'test', 'testroot')
        s.add_next_algorithm(name_algo)

        path_algo = scommon.sdselector.PathAlgorithm(env.absroot)
        path_algo.add_groups_for_prefix('.', 'build')
        path_algo.add_groups_for_prefix('Tools', 'tools')
        path_algo.add_groups_for_prefix(env.builddir, 'pack-all')
        path_algo.add_groups_for_prefix('test', 'build', 'test')
        path_algo.add_groups_for_prefix('testroot', 'build', 'test', 'testroot')
        path_algo.add_groups_for_prefix(os.path.join(env.builddir, 'test'), 'build', 'test')
        path_algo.add_groups_for_prefix(os.path.join(env.builddir, 'testroot'), 'build', 'test', 'testroot')
        s.add_next_algorithm(path_algo)

        default_algo = scommon.sdselector.DefaultGroupsAlgorithm('build', 'test', 'testroot')
        s.add_next_algorithm(default_algo)
        return s

    env['SCRIPT_DIR_SELECTOR_'] = selector(env['BUILD_DIRS_'], env['TOOLS_BUILD_DIRS_'])

def handle_link_type(env):
    linktype = env['LINKTYPE']
    env.merge(env['LINKTYPES_'][linktype])
    # Here we store object nodes that are not
    # used immediately to create a library but
    # later to build a binary, e.g. the server.
    env.stash['DIRECTLY_LINKED_OBJECTS'] = {}

def handle_cxx_debug(env):
    if env['CXX_DEBUG']:
        env.merge(env['CXX_DEBUG_'])

def handle_disable_asap(env):
    if env['DISABLE_ASAP']:
        env.merge(env['DISABLE_ASAP_'])
        env['ASAP_LIBLIST_'] = []

def handle_disable_debug_symbols(env):
    if env['DISABLE_DEBUG_SYMBOLS']:
        env.merge(env['DISABLE_DEBUG_SYMBOLS_'])

def handle_enable_debug_symbols_compression(env):
    if env['ENABLE_SYMBOLS_COMPRESSION']:
        env.merge(env['ENABLE_SYMBOLS_COMPRESSION_'])

def handle_coverage(env):
    if env['COVERAGE']:
        env.merge(env['COVERAGE_FLAG_'])

def handle_extend_branch_prediction(env):
    if env['EXTEND_BRANCH_PREDICTION']:
        env.merge(env['EXTEND_BP_FLAG_'])

def handle_defines(env):
    if env['DEFINES']:
      env.add_defines(*env['DEFINES'].split(','))
        
def handle_staticcpp_link(env):
    if env['LINKTYPE'] == 'static':
        if env['ENABLE_STDCPP_STATIC_LINK']:
            env.merge(env['ENABLE_STDCPP_STATIC_LINK_FLAG_'])

def handle_sanitizers(env):
    any_sanitizer = False

    if env['ASAN']:
        any_sanitizer = True
        env.merge(env['ASAN_'])

    if env['UBSAN']:
        any_sanitizer = True
        env.merge(env['UBSAN_'])

    if env['TSAN']:
        any_sanitizer = True
        env.merge(env['TSAN_'])

    if any_sanitizer and env['TYPE'] == 'debug':
        env.merge(env['SAN_DEBUG_'])

def handle_null_allocator(env):
    if env['NULL_ALLOCATOR'] == 'default':
        return
    env['NULL_ALLOCATOR_'] = (env['NULL_ALLOCATOR'] == 'yes')

def handle_tracking(env):
    if env['TRACKING']:
        env.merge(env['TRACKING_'])

def handle_test_report_destination(env):
    if not env.raw.has_key('TESTS_REPORTS_DESTINATION'):
        if env['COVERAGE']:
            env['TESTS_REPORTS_DESTINATION'] = 'sc_coverage'
        else:
            # set the default value to suppress error message
            # TODO: refactor the argument reading ASAP
            env['TESTS_REPORTS_DESTINATION'] = None
    if env['TESTS_REPORTS_DESTINATION']:
        xml_dir = os.path.abspath(env['TESTS_REPORTS_DESTINATION'])
        if not os.path.isdir(xml_dir):
            log.info('Test report directory ("{:s}") doesn\'t exist - creating'.format(xml_dir))
            try:
                os.mkdir(xml_dir)
            except Exception as e:
                log.error('Can\'t create directory "{:s}": {:s}'.format(xml_dir, str(e)))
                Exit(1)
        env['TESTS_REPORTS_DESTINATION_'] = xml_dir

# misc. options

def dist_jobs_threshold(env):
    try:
        import multiprocessing
        return multiprocessing.cpu_count()
    except ImportError:
        return env['DEFAULT_DIST_JOBS_THRESHOLD_']

def handle_num_jobs(env):
    num_jobs = GetOption('num_jobs')

    limit = env['JOBS_LIMIT_']
    if num_jobs > limit:
        log.error('Refusing to build with -j%d due to limit excess' % num_jobs)
        log.error('Please, lower to at most -j%d' % limit)
        Exit(1)

    threshold = dist_jobs_threshold(env)
    if not env['DIST'] and num_jobs > threshold:
        log.info('Forcing DIST=1 due to -j%d > %d' % (num_jobs, threshold))
        env['DIST'] = True

def handle_distributed_build(env):
    if not env['DIST']:
        return

    import octopus.client.scons

    log_level = pytools.log.level_str_to_int(env['DISTLOG'].upper())
    logging.getLogger('twisted').setLevel(log_level)
    logging.getLogger('octopus').setLevel(log_level)

    octopus = octopus.client.scons.SconsClient(env['DISTCFG'])
    env['OCTOPUS'] = octopus

    if env['COMPILER_STDERR']:
        try:
            octopus.set_error_file(sys.stderr)
        except AttributeError:
            pass

    env['SPAWN'] = scommon.plugins.fork_for_dist_build(env['SPAWN'], octopus.spawn)

def setup_verbose_colors(env):
    from scommon.toolkit.colors import colors

    compile_source_message = '%sCompiling %s==> %s$SOURCE%s' % \
       (colors['blue'], colors['violet'], colors['yellow'], colors['clear'])

    compile_shared_source_message = '%sCompiling shared %s==> %s$SOURCE%s' % \
       (colors['blue'], colors['violet'], colors['yellow'], colors['clear'])

    link_program_message = '%sLinking Program %s==> %s$TARGET%s' % \
       (colors['green'], colors['violet'], colors['yellow'], colors['clear'])

    link_shared_library_message = '%sLinking Shared Library %s==> %s$TARGET%s' % \
       (colors['green'], colors['violet'], colors['yellow'], colors['clear'])

    env['CCCOMSTR'] = compile_source_message
    env['CXXCOMSTR'] = compile_source_message
    env['SHCCCOMSTR'] = compile_source_message
    env['SHCXXCOMSTR'] = compile_source_message
    env['LINKCOMSTR'] = link_program_message
    env['SHLINKCOMSTR'] = link_shared_library_message

def setup_verbosity(env):
    if not env['VERBOSE']:
        # provide short compile/link message schemas
        env['CCCOMSTR'] = "Compiling $SOURCE"
        env['SHCCCOMSTR'] = "Compiling $SOURCE"
        env['CXXCOMSTR'] = "Compiling $SOURCE"
        env['SHCXXCOMSTR'] = "Compiling $SOURCE"
        env['LINKCOMSTR'] = "Linking $TARGET"
        env['SHLINKCOMSTR'] = "Linking $TARGET"

        #If the output is not a terminal, remove the colors
        if env['EXT_LOG'] == 'extended' and sys.stdout.isatty():
            setup_verbose_colors(env)

# Scons processing optimisation
# these tasks are run in this order to let a task
# override user variables used by some following tasks

def handle_opt(env):
    opt = env['OPT']

    if opt == 'quick':
        # enable implicit cache
        scommon.optiontools.set_option('implicit_cache', 1)
        return

    if opt == 'normal':
        # do not use implicit cache
        return

    if opt == 'deep':
        # turn on ISYSTEM_DEPS
        # but still use cache
        env['ISYSTEM_DEPS'] = 1
        scommon.optiontools.set_option('implicit_cache', 1)
        return

    if opt == 'paranoid':
        # turn on ISYSTEM_DEPS
        env['ISYSTEM_DEPS'] = 1
        # force MD5 decider
        env['DECIDER'] = 'md5'
        return

def handle_decider(env):
    if env['DECIDER'] == 'md5':
        Decider('MD5')
    else:
        # hybrid
        Decider('MD5-timestamp')

def setup_isystem_dependencies(env):
    '''
    Scons does not handle the gcc's -isystem option for library headers'
    locations but we want to since:
    a) we want to keep the -isystem option in gcc calls, as for regular makefiles,
    b) we don't want to scan "isystem" paths for dependency changes.

    Consequently, two separable variables: INCLUDE_PATH_ and ISYSTEM_PATH_
    have been created. Then:
    ad. a) we override the _CPPINCFLAGS scons variable so that it contains
           entries from both INCLUDE_PATH_ and ISYSTEM_PATH_, but with
           the isystem keywords in the second case.
    ad. b) only entries from INCLUDE_PATH_ are added to the CPPPATH variable
           which stores directories visited by the scons' dependency scanner. Except
           we set ISYSTEM_DEPS to True (normally disabled): we use it when we want
           to track dependencies also in "library" directories (but this slows
           scons down noticeably).

    JOIN_LISTS_ is a key where we store a function for lists concatenation.
    This way we can force scons to resolve CPPPATH to a list being sum
    of two lists: normally (e.g. C = "$A $B") we end up with a "flat" string.
    '''
    f = '$( ${_concat(INCPREFIX, INCLUDE_PATH_, INCSUFFIX, __env__, RDirs, TARGET, SOURCE)} ' +\
            '${_concat(ISYSTEMPREFIX_, ISYSTEM_PATH_, ISYSTEMSUFFIX_, __env__, RDirs, TARGET, SOURCE)} $)'
    env['_CPPINCFLAGS'] = f
    log.debug('set _CPPINCFLAGS to %s' % f)

    def join_lists(a, b):
        rv = a + b
        return rv

    env['JOIN_LISTS_'] = join_lists

    if env['ISYSTEM_DEPS']:
        env['CPPPATH'] = '${JOIN_LISTS_(INCLUDE_PATH_, ISYSTEM_PATH_)}'
    else:
        env['CPPPATH'] = '$INCLUDE_PATH_'

def precalc_common_defines(env):
    env['TSE_VOB_DIR_DEFINE_'] = ('TSE_VOB_DIR', scommon.strshell(env.absroot))

def install_env_to_factory_config_callback(env):

    LD_LIB_PATHS = [
        'libshared',
        'test',
        'test/DBAccessMock',
        'test/Runner',
        'test/testdata',
        'test/testdata/tinyxml',
    ]

    utexe_ld_lib_paths = [os.path.join(env.absbuilddir, suffix) for suffix in LD_LIB_PATHS]
    utexe_ld_lib_paths += [env.subst('$STD_LIBPATH_')]
    utexe_ld_lib_paths += map(env.subst, env['EXTERNAL_LIBPATHS_'])
    utexe_ld_lib_paths += map(env.subst, env['EXTERNAL_TEST_LIBPATHS_'])

    utexe_sysenv = {}
    utexe_sysenv['TNS_ADMIN'] = os.path.join(env.absroot, 'scons', 'scripts')
    utexe_sysenv['HOSTNAME'] = 'ltxl'

    def envtodict(env):
        config = {}
        config['linktype'] = env.linktype
        config['installdir'] = env.installdir
        config['common_test_objects'] = [env.stash.get('TEST_PLUGIN')]
        config['test_include_paths'] = [env.absbuilddir],
        config['test_isystem_paths'] = ['$CPPUNIT_CPPPATH_', '$GMOCK_CPPPATH_', '$GTEST_CPPPATH_']
        config['utexe'] = env.stash.get('utexe')
        config['utexe_db'] = env.stash.get('utexe_db')
        config['utexe_ld_lib_paths'] = utexe_ld_lib_paths
        config['utexe_sysenv'] = utexe_sysenv
        if env.direct_object_link:
            config['object_cache'] = env.stash['DIRECTLY_LINKED_OBJECTS']
        return config

    env.set_env_to_factory_config_callback(envtodict)

def init_testbank(env):
    bank = scommon.Testbank(
            bld_root = 'testroot/bld',
            run_root = 'testroot/run')
    env.testbank = bank

class OctopusMessageGetter(object):
    def __init__(self, octopus):
        self.octopus = octopus

    def get(self, command):
        return self.octopus.get_message_of(command)

    def get_all(self):
        return self.octopus.get_messages()

def enable_build_status_details(env):
    ext_log = (env['EXT_LOG'] == 'extended' and sys.stdout.isatty())

    messages = None
    if env['DIST']:
        octopus = env['OCTOPUS']
        try:
            octopus.get_message_of
            octopus.get_messages
        except AttributeError:
            pass
        else:
            messages = OctopusMessageGetter(octopus)

    file = sys.stdout
    if env['COMPILER_STDERR']:
        file = sys.stderr

    atexit.register(scommon.get_display_build_status(
        ext_log=ext_log,
        display_warnings=env['DISPLAY_WARNINGS'],
        messages=messages,
        file=file))

def handle_package_build(env):
    if COMMAND_LINE_TARGETS:
        targets = COMMAND_LINE_TARGETS
    else:
        targets = DEFAULT_TARGETS
    if 'package' in targets:
        '''
        Create configuration to run PackageManager
        Install builder
        '''
        pass

TASKS = [
    #Import configuration into environment
    merge_config_dict,

    #Setup basic ssytem configrations
    handle_third_party,
    handle_binutils,
    read_redhat_version,
    make_rh_version_dependant_setup,
    handle_compiler_version,
    handle_cpp_standard_version,
    set_build_label,
    enable_cache,
    enable_symlink_install,
    install_cpp_test_factory_builder,
    disable_source_code,
    setup_c_compiler,
    set_umask,

    #Setup building initiall data
    handle_build_type,
    handle_destination_directory, #This method possible modifies build directory so it has to be called before any call to env.builddir

    #Setup specific building data
    setup_aliases,
    setup_script_dir_selector,
    handle_link_type,
    handle_cxx_debug,
    handle_disable_asap,
    handle_coverage,
    handle_extend_branch_prediction,
    handle_defines,
    handle_staticcpp_link,
    handle_test_report_destination,
    handle_sanitizers,
    handle_null_allocator,
    handle_tracking,

    handle_num_jobs,
    handle_distributed_build,
    setup_verbosity,

    handle_opt,
    handle_decider,
    handle_disable_debug_symbols,
    handle_enable_debug_symbols_compression,
    setup_isystem_dependencies,
    precalc_common_defines,
    install_env_to_factory_config_callback,
    init_testbank,
    enable_build_status_details,
]



def tasks_as_command(env):
    subcommands = [pytools.command.command(t, env) for t in TASKS]
    return pytools.command.composite('Configuring SCons for V2', subcommands)

def script_dirs(env):
    s = env['SCRIPT_DIR_SELECTOR_']
    return s.get_dirs(scommon.bridge.get_targets())
