#!/usr/bin/env python

import os
import re
import sys
import shutil
import glob
import subprocess
import logging
import fnmatch
from argparse import ArgumentParser


logger = logging.getLogger(__name__)
logger_csf = logging.getLogger('csf')


class PackageCreatorError(Exception):
    pass


class Configuration(object):
    '''
    Keeps all configuration of the packagea creation process
    - baseline - version of the binary package like: atsev2.2014.00
    - build_subdirectory - where to look for binary/lib files: debug/release/...
    - stage - whether to deploy package after the build into the remote locations
    - strip - whether to strip debug symbols into separate package
    - drop_symbols - whether to create package with striped debug symbols
    - binary - server binary name
    -
    '''
    _CSF_PKGCREATOR    = '/opt/atse/common/csf_pkgmaker.sh'
    _OBJCPY            = "/opt/binutils-2.23.2-gcc-4.8.1/bin/objcopy"
    _OBJSTRIP          = "/opt/binutils-2.23.2-gcc-4.8.1/bin/strips"
    _PKG_BACKUPS_ROOT  = '/project/build_storage/packagesbak'
    _PKG_LOCAL_STORAGE = 'packages'
    _PKG_TMP_DIR       = 'package_tmp'
    _PKG_SYMBOLS_TMP_DIR = 'package_symbols_tmp'
    _BSL_MATCHER       = r'(.*)\.(\d{4}\.\d{2}.*)'
    _RH_MATCHER        = r'.*(\d{1})\.(\d{1}).*'
    _SYMBOLS_SUFFIX    = '.debug'
    _LIB_PATTERN       = '.so'
    _LDC_BINARIES = ['cachetest', 'listBDB', 'cacheCompare', 'listCacheEvents', 'bdbCompare', 'readTest', 'threadTest']

    def __init__(self,
                 baseline,
                 build_subdirectory,
                 stage = False,
                 strip = False,
                 drop_symbols = False,
                 binary = "tseserver.static"):
        self.baseline = baseline.strip()
        self.family  = None
        self.version = None
        self.build_subdirectory = build_subdirectory
        self.redhat = None
        self.stage = stage
        self.strip = strip
        self.drop_symbols = drop_symbols
        self.root = None
        self.binary = binary

        self.binary_path = None
        self.libraries_paths = []
        self.ldc_binaries_paths = []

        self.systemtypes = {
                            'cfg.DevHybrid':'dev',
                            'cfg.TestHybrid':'integ',
                            'cfg.CertHybrid':'cert',
                            'cfg.PlabHybrid':'cert',
                            'cfg.ProdHybrid':'prod'
                            }

        self._get_rh_version()
        self._get_root_path()
        self._baseline_parse()

        self.pkgname = "%s-Linux-x86_64-rh%s.tar.gz" % (self.baseline, self.redhat)
        self.pkgname_symbols = "%s%s-Linux-x86_64-rh%s.tar.gz" % (self.baseline,
                                                                  self._SYMBOLS_SUFFIX,
                                                                  self.redhat)

        backup_location = "%s.%s" % (self.family, self.version)
        self.pkg_backup_location = os.path.join(self._PKG_BACKUPS_ROOT, backup_location)
        self.local_pkg_storage = os.path.join(self.root, self._PKG_LOCAL_STORAGE)
        self.tmp_pkg_storage = os.path.join(self.root, self._PKG_TMP_DIR)
        self.tmp_symbols_pkg_storage = os.path.join(self.root, self._PKG_SYMBOLS_TMP_DIR)

        self.config_dir = os.path.join(self.root, 'deploy')
        self.apps_dir = os.path.join(self.config_dir, 'apps')
        self.ldctools_dir = os.path.join(self.root, 'Tools', 'ldc')

    def __str__(self):
        output = []
        output.append("CSF script: %s" % self._CSF_PKGCREATOR)
        output.append("Strip cmd path: %s" % self._OBJCPY)
        output.append("Pkg name: %s" % self.pkgname)
        output.append("Pkg backup root location: %s" % self._PKG_BACKUPS_ROOT)
        output.append("Pkg backup location: %s" % self.pkg_backup_location)
        output.append("Pkg local storage location: %s" % self.local_pkg_storage)
        output.append("Pkg tmp local storage location: %s" % self.tmp_pkg_storage)
        output.append("Pkg symbols tmp local storage location: %s" % self.tmp_symbols_pkg_storage)
        output.append("RedHat version: %s" % self.redhat)
        output.append("Family: %s" % self.family)
        output.append("Version: %s" % self.version)
        output.append("Root dir: %s" % self.root)
        output.append("Build subdirectory: %s" % self.build_subdirectory)
        output.append("Binary: %s" % self.binary_path)
        for item in self.libraries_paths:
            output.append("Library: %s" % item)
        output.append("LDC dir: %s" % self.ldctools_dir)
        for item in self.ldc_binaries_paths:
            output.append("LDC binary: %s" % item)
        return '\n'.join(output)

    def _baseline_parse(self):
        '''
        Parse baseline name and retrieve family and version from it
        '''
        result = re.match(self._BSL_MATCHER, self.baseline, re.S|re.I)
        if result:
            self.family  = result.group(1)
            self.version = result.group(2)
        else:
            raise PackageCreatorError('Incorrect baseline name. Expected: <family>.<release>')

    def _get_rh_version(self):
        '''
        Get RedHat version
        '''
        try:
            with open('/etc/redhat-release','r') as filehandle:
                line = filehandle.read()
                result = re.match(self._RH_MATCHER, line, re.S|re.I)
                self.redhat = result.group(1)
        except:
            raise PackageCreatorError('Cannot determine RedHat version')

    def _get_root_path(self):
        '''
        Get application root path
          Just get ../.. as it will check script location in subdirectory
        '''
        self.root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    def _check_file_exists(self, filepath):
        '''
        Check if file exists, otherwise throw an exception
        '''
        if not os.path.isfile(filepath):
            raise PackageCreatorError('Missing: [%s]' % filepath)

    def verify(self):
        self._check_file_exists(self.binary_path)
        if len(self.ldc_binaries_paths) != len(self._LDC_BINARIES):
            raise PackageCreatorError('Missing ldctool binary. Found paths [%s]. Expected binaries [%s]' % (self.ldc_binaries_paths, self._LDC_BINARIES))


class SconsConfiguration(Configuration):
    '''
    Keeps all configuration specific for Scons of the packagea creation process
    - build_subdirectory - specifies subdirectory where scons build objects are dropped (i.e. sc_debug)
    '''

    _LIBSHARED_DIR = 'libshared'

    def __init__(self,
                 baseline,
                 build_subdirectory,
                 stage = False,
                 strip = False,
                 drop_symbols = False,
                 binary = "tseserver.static"):
        super(SconsConfiguration, self).__init__(baseline,
                                                 build_subdirectory,
                                                 stage,
                                                 strip,
                                                 drop_symbols,
                                                 binary)

    def __str__(self):
        return super(SconsConfiguration, self).__str__()

    def _get_server_file(self):
        self.binary_path = os.path.join(self.root, self.build_subdirectory, 'Server', self.binary)

    def _get_ldctools_files_list(self):
        ldc_path = os.path.join(self.root, self.build_subdirectory, 'Tools', 'ldc')
        for root, dirs, files in os.walk(ldc_path):
            for name in files:
                if name in self._LDC_BINARIES:
                    self.ldc_binaries_paths.append(os.path.join(root, name))

    def _get_libraries_list(self):
        build_path = os.path.join(self.root, self.build_subdirectory, self._LIBSHARED_DIR)
        for root, dirs, files in os.walk(build_path):
            for name in files:
                if name.endswith(self._LIB_PATTERN):
                    self.libraries_paths.append(os.path.join(root, name))

    def collect(self):
        self._get_server_file()
        self._get_ldctools_files_list()
        self._get_libraries_list()


class MakeConfiguration(Configuration):
    '''
    Keeps all configuration specific for Make of the packagea creation process
    - build_subdirectory - specifies subdirectory where scons build objects are dropped (i.e. debug)
    '''

    def __init__(self,
                 baseline,
                 build_subdirectory,
                 stage = False,
                 strip = False,
                 drop_symbols = False,
                 binary = "tseserver.static"):
        super(MakeConfiguration, self).__init__(baseline,
                                                build_subdirectory,
                                                stage,
                                                strip,
                                                drop_symbols,
                                                binary)

    def __str__(self):
        return super(MakeConfiguration, self).__str__()

    def _get_server_file(self):
        self.binary_path = os.path.join(self.root, 'bin', self.build_subdirectory, self.binary)

    def _get_ldctools_files_list(self):
        ldc_path = os.path.join(self.root, 'Tools', 'ldc', self.build_subdirectory)
        for root, dirs, files in os.walk(ldc_path):
            for name in files:
                if name in self._LDC_BINARIES:
                    self.ldc_binaries_paths.append(os.path.join(root, name))

    def _get_libraries_list(self):
        build_path = os.path.join(self.root, 'lib', self.build_subdirectory)
        for root, dirs, files in os.walk(build_path):
            for name in files:
                if name.endswith(self._LIB_PATTERN):
                    self.libraries_paths.append(os.path.join(root, name))

    def collect(self):
        self._get_server_file()
        self._get_ldctools_files_list()
        self._get_libraries_list()


class PackageCreator:
    '''
    Class responsible for copy necessary files, create package and deploy it to standard locations
    '''
    def __init__(self, configuration):
        self._configuration = configuration

    def _check_file_exists(self, path, binary = None, exists_check = True):
        '''
        Check if file exists, otherwise throw an exception
        '''
        file = path
        if binary:
            file = os.path.join(path, binary)

        if not os.path.isfile(file):
            if exists_check:
                raise PackageCreatorError('Missing: [%s]' % file)
        elif not exists_check:
            raise PackageCreatorError('File already exists: [%s]' % file)

    def _check_package_exists(self):
        '''
        Check if elements to be created does not already exists
        '''
        self._check_file_exists(self._configuration.local_pkg_storage,
                                self._configuration.pkgname, False)
        self._check_file_exists(self._configuration.local_pkg_storage,
                                self._configuration.pkgname_symbols, False)

    def _create_directory(self, directory):
        '''
        Create directory if it does not exists yet
        '''
        try:
            if not os.path.exists(directory):
                os.makedirs(directory)
        except OSError as exc:
            raise PackageCreatorError('Cannot create [%s]:\n%s' % (directory, str(exc)))

    def _collect_sub_directories(self, root_dir):
        '''
        Collect all direct subdirectories within root_dir
        '''
        return [dir for dir in os.listdir(root_dir) if os.path.isdir(os.path.join(root_dir, dir))]

    def _get_config_dir(self, server_dir, env, app):
        '''
        Get configuration directory for an application in particular environment

        Some environments might not have any specific setup and use common configuration
         from server directory. In such a case config subdirectory for them is missing
        '''
        src_config_dir = os.path.join(self._configuration.config_dir, env, 'config', app)
        if not os.path.isdir(src_config_dir):
            src_config_dir = server_dir
        return src_config_dir

    def _create_symlink(self, srcfile, destination_dir):
        '''
        Create symbolic links from srcfile to destination_dir.
        '''
        os.symlink(os.path.join(destination_dir, srcfile), srcfile);

    def _copy_app_files(self):
        '''
        Copy necessary files for all applications except of tseshared and ldctools
        '''
        SKIPPED_APPS = ['tseshared', 'ldctools']

        APP_LINKS_LIST = ['prestart.sh', 'checkhung.sh', 'appmon_hook.sh', 'cfgshell_functions.sh', 'ld_path.sh', 'ld_path_dbg.sh']
        APP_SYS_CFG_FILES = ['dbaccess*ini', 'log4cxx.xml', 'tseserver.cfg.user']
        APP_CFG_FILES = ['config.vars']

        server_dir = os.path.join(self._configuration.root, "Server")
        apps = self._collect_sub_directories(self._configuration.apps_dir)

        for app in apps:
            if app in SKIPPED_APPS:
                continue
            logger.info("Processing [%s]" % app)

            app_directory = os.path.join(self._configuration.apps_dir, app)
            destination = os.path.join(self._configuration.tmp_pkg_storage, app)
            self._create_directory(destination)
            os.chdir(destination)

            dstbsl = "../tseshared.%s" % self._configuration.baseline
            for link in APP_LINKS_LIST:
                self._create_symlink(link, dstbsl)

            for (system, env) in self._configuration.systemtypes.items():
                self._create_directory(system)
                src_config_dir = self._get_config_dir(server_dir, env, app)
                dst_config_dir = os.path.join(destination, system)
                for item in APP_SYS_CFG_FILES:
                    self._copy_files(src_config_dir, dst_config_dir, item)

            if app == 'shoppingesv':
                self._copy_files(server_dir, destination, 'VIS_Beta.txt')
                self._copy_files(app_directory, destination, 'binderdatacopy.sh')

            for item in APP_CFG_FILES:
                self._copy_files(app_directory, destination, item)
        os.chdir(self._configuration.root)

    def _copy_tseshared_files(self):
        '''
        Copy necessary files for tseshared
        '''
        logger.info("Processing [tseshared]")

        TSE_CFG_FILES = ['asap.ini', 'tsi.ini', 'cacheNotify.xml']
        TSE_ANALYZE_SCRIPTS = ['analyseCore', 'processCore', 'lastRequests', 'lastRequests.awk']
        TSE_FILES = ['*']

        xform_conf_dir  = os.path.join(self._configuration.root, 'Xform')
        destination     = os.path.join(self._configuration.tmp_pkg_storage, 'tseshared')
        destination_lib = os.path.join(destination, 'lib')

        self._create_directory(destination_lib)

        self._copy_files(xform_conf_dir, destination, '*.cfg')
        self._copy_files(os.path.join(self._configuration.config_dir, 'integ/config/bin'), destination, 'log4cxx.xml')

        self._copy_files(os.path.dirname(self._configuration.binary_path), destination,
                         os.path.basename(self._configuration.binary_path) + '*',
                         '/' + self._configuration._SYMBOLS_SUFFIX + '/', False, True)

        for lib in self._configuration.libraries_paths:
            self._copy_file(lib, destination_lib, False, True)

        server_dir    = os.path.join(self._configuration.root, 'Server')
        for cfg in TSE_CFG_FILES:
            self._copy_files(server_dir, destination, cfg)

        scripts_dir = os.path.join(self._configuration.root, 'Tools', 'Scripts')
        core_analysis_dir = os.path.join(scripts_dir, 'core_analysis')
        for script in TSE_ANALYZE_SCRIPTS:
            self._copy_files(core_analysis_dir, destination, script)

        tseshared_conf_dir = os.path.join(self._configuration.apps_dir, 'tseshared')
        for item in TSE_FILES:
            self._copy_files(tseshared_conf_dir, destination, item)

    def _copy_ldctools_files(self):
        '''
        Copy necessary files for ldctools
        '''
        logger.info("Processing [ldctools]")

        LDC_SCRIPTS = ['cachetest.sh', 'dumpkeys.sh', 'package.sh', 'testnotify.sh', 'cacheNotify.xml', 'config_functions.sh']
        LDC_FILES = ['*']

        destination = os.path.join(self._configuration.tmp_pkg_storage, "ldctools")
        self._create_directory(destination)

        for binary in self._configuration.ldc_binaries_paths:
            self._copy_file(binary, destination)

        for script in LDC_SCRIPTS:
            self._copy_files(self._configuration.ldctools_dir, destination, script)

        ldctools_conf_dir = os.path.join(self._configuration.apps_dir, "ldctools")
        for item in LDC_FILES:
            self._copy_files(ldctools_conf_dir, destination, item)

    def _copy_symbols_files(self):
        '''
        Copy necessary symbols files for debugging
        '''
        if (self._configuration.drop_symbols):
            logger.debug("Copy symbols files canceled by request")
            return

        logger.info("Copy symbols files")

        destination = os.path.join(self._configuration.tmp_symbols_pkg_storage, 'tseshared')
        self._create_directory(destination)
        self._copy_file(self._configuration.binary_path + self._configuration._SYMBOLS_SUFFIX,
                        destination,
                        False,
                        True)

        for lib in self._configuration.libraries_paths:
            self._copy_file(lib + self._configuration._SYMBOLS_SUFFIX,
                            destination,
                            False,
                            True)

    def _copy_files(self, src, dst, pattern, excl_pattern="", copysymlinks=False, usehardlink=False):
        '''
        Copy all files from src into dst using provided pattern
        For files additional options applies:
         - copysymlink - allows to copy symbolic links
         - usehardlink - create hardlink instead of copy file
        '''
        try:
            srcfiles = os.path.join(src, pattern)

            for srcfile in glob.glob(srcfiles):
                if not fnmatch.fnmatch(srcfile, excl_pattern):
                    if os.path.isdir(srcfile):
                        shutil.copytree(srcfile, os.path.join(dst, os.path.basename(srcfile)), True)
                        continue
                    dstfile = os.path.join(dst, os.path.basename(srcfile))
                    if copysymlinks and os.path.islink(srcfile):
                        linkto = os.readlink(srcfile)
                        os.symlink(linkto, dstfile)
                    elif usehardlink:
                        srcfile = os.path.realpath(srcfile)
                        os.link(srcfile, dstfile)
                    else:
                        shutil.copy(srcfile, dstfile)
        except (shutil.Error, OSError) as exc:
            raise PackageCreatorError('Cannot copy files from [%s] into [%s] using pattern [%s]:\n%s' % (src, dst, pattern, str(exc)))

    def _copy_file(self, srcfile, dstpath, copysymlinks=False, usehardlink=False):
        '''
        Copy srcfile into dstpath
         - copysymlink - allows to copy symbolic links
         - usehardlink - create hardlink instead of copy file
        '''
        if not os.path.isfile(srcfile):
            raise PackageCreatorError('Attempt to copy [%s] failed' % srcfile)

        try:
            dstfile = os.path.join(dstpath, os.path.basename(srcfile))
            if copysymlinks and os.path.islink(srcfile):
                linkto = os.readlink(srcfile)
                os.symlink(linkto, dstfile)
            elif usehardlink:
                srcfile = os.path.realpath(srcfile)
                os.link(srcfile, dstfile)
            else:
                shutil.copy(srcfile, dstfile)
        except (shutil.Error, OSError) as exc:
            raise PackageCreatorError('Cannot copy file [%s] into [%s]\n%s' % (srcfile, dstpath, str(exc)))

    def _compose_csf_command(self, source_directory, version):
        '''
        Prepare CSF command
        '''
        csf_cmd = []
        csf_cmd.append(self._configuration._CSF_PKGCREATOR)
        csf_cmd.append('-family')
        csf_cmd.append(self._configuration.family)
        csf_cmd.append('-release')
        csf_cmd.append(version)
        csf_cmd.append('-tarloc')
        csf_cmd.append(self._configuration.local_pkg_storage)

        apps = self._collect_sub_directories(source_directory)

        csf_cmd.append('-app')
        csf_cmd.extend(apps)

        csf_cmd.append('-dir')
        for app in apps:
            csf_cmd.append(os.path.join(source_directory, app))

        csf_cmd.append('-tmpdir')
        csf_cmd.append(source_directory)

        if self._configuration.stage:
            csf_cmd.append('-msr')
        return csf_cmd

    def _call_csf(self, csf_cmd):
        '''
        Call CSF script
        '''
        logger.info("Preparing package ...")
        self._create_directory(self._configuration.local_pkg_storage)
        logger.debug("CSF cmd [%s]" % csf_cmd)

        if not self._cmd_call(csf_cmd, logger_csf):
            raise PackageCreatorError('Failed CSF call')

    def _backup_package(self):
        '''
        Copy package(s) into backup storage
        '''
        logger.info("Backup package ...")
        self._create_directory(self._configuration.pkg_backup_location)

        srcfile = os.path.join(self._configuration.local_pkg_storage, self._configuration.pkgname)
        self._copy_file(srcfile, self._configuration.pkg_backup_location)

        if (self._configuration.strip):
            srcfile = os.path.join(self._configuration.local_pkg_storage,
                                   self._configuration.pkgname_symbols)
            self._copy_file(srcfile, self._configuration.pkg_backup_location)

    def _cmd_call(self, cmd, cmd_logger):
        '''
        Call external command
        '''
        logger.debug("Call cmd [%s]" % cmd)
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in iter(process.stdout.readline, ''):
            cmd_logger.info(line.strip())
        if not process.wait() == 0:
            return False
        return True

    def _strip_file_symbols(self, source, destination):
        '''
        Detach debug symbols from main binaries
         - Copy debug symbols command
         - Strip debug symbols command
         - Attaching debug link command
        '''
        logger.info("Stripping symbols")

        if (self._configuration.drop_symbols):
            logger.debug("Detaching debug symbols canceled by request")
        else:
            logger.debug("Detaching debug symbols from %s into %s" % (source, destination))
            dump_cmd = []
            dump_cmd.append(self._configuration._OBJCPY)
            dump_cmd.append('--only-keep-debug')
            dump_cmd.append(source)
            dump_cmd.append(destination)
            if not self._cmd_call(dump_cmd, logger):
                raise PackageCreatorError('Failed to copy debug symbols')

        logger.debug("Stripping debug symbols from %s" % source)
        strip_cmd = []
        strip_cmd.append(self._configuration._OBJCPY)
        strip_cmd.append('--strip-debug')
        strip_cmd.append('--strip-unneeded')
        strip_cmd.append(source)
        if not self._cmd_call(strip_cmd, logger):
            raise PackageCreatorError('Failed to strip debug symbols')

        if (self._configuration.drop_symbols):
            logger.debug("Adding link to debug symbols canceled by request")
        else:
            logger.debug("Adding link to debug symbols from %s into %s" % (destination, source))
            attach_cmd = []
            attach_cmd.append(self._configuration._OBJCPY)
            attach_cmd.append('--add-gnu-debuglink=' + destination)
            attach_cmd.append(source)
            if not self._cmd_call(attach_cmd, logger):
                raise PackageCreatorError('Failed to attach debug symbols')

    def _strip_symbols(self):
        '''
        Strip symbols from libraries and executable
        '''
        for libfile in self._configuration.libraries_paths:
            self._strip_file_symbols(libfile, libfile + self._configuration._SYMBOLS_SUFFIX)
        self._strip_file_symbols(self._configuration.binary_path,
                                 self._configuration.binary_path + self._configuration._SYMBOLS_SUFFIX)

    def _cleanup(self):
        '''
        Do some cleanup by removing intermediate objects
        '''
        logger.info("Cleanup ...")
        try:
            if os.path.isdir(self._configuration.tmp_pkg_storage):
                shutil.rmtree(self._configuration.tmp_pkg_storage)
            if os.path.isdir(self._configuration.tmp_symbols_pkg_storage):
                shutil.rmtree(self._configuration.tmp_symbols_pkg_storage)
        except OSError as err:
            raise PackageCreatorError('Cleanup operation failed [%s]' % str(err.args))


    def build_package(self):
        '''
        Take all necessary steps to build package
        '''
        logger.info("**************************************")
        logger.info("ATSEv2 baseline package generating")
        logger.info("**************************************")

        self._check_package_exists()
        self._copy_app_files()

        if (self._configuration.strip):
            self._strip_symbols()
            self._copy_symbols_files()

        self._copy_tseshared_files()
        self._copy_ldctools_files()
        self._call_csf(self._compose_csf_command(self._configuration.tmp_pkg_storage,
                                                 self._configuration.version))

        if (self._configuration.strip and not self._configuration.drop_symbols):
            version = self._configuration.version + self._configuration._SYMBOLS_SUFFIX
            self._call_csf(self._compose_csf_command(self._configuration.tmp_symbols_pkg_storage,
                                                     version))
        if self._configuration.stage:
            self._backup_package()


class PackageCreatorManager:
    '''
    Manager which drives package creation process
    '''
    def __init__(self, configuration):
        self._configuration = configuration

    def process(self):
        creator = PackageCreator(self._configuration)
        try:
            creator.build_package()
            return True
        except PackageCreatorError as exc:
            logger.critical(str(exc.args))
        except KeyboardInterrupt:
            logger.critical('Process interrupted ...')
        except Exception as e:
            logger.critical(str(e))
        finally:
            creator._cleanup()
        return False


def main(argv=None):
    try:
        parser = ArgumentParser(description='ATSEv2 baseline package generator')
        parser.add_argument('--silent', action="store_true",
                            help="Supress printing info messages (default: false)")
        parser.add_argument('--debug', action="store_true",
                            help="Print debug messages (default: false)")
        parser.add_argument('--baseline',  type=str, required=True,
                            help='Baseline <family>.<release> Ex. atsev2.2015.00.00')
        parser.add_argument('--build-directory', type=str, default='sc_release',
                            help='Build directory (default: sc_release)')
        parser.add_argument('--stage', action="store_true",
                            help="Stage package (default: false)")
        parser.add_argument('--strip', action="store_true",
                            help="Strip debug symbols info separate package (default: false)")
        parser.add_argument('--drop-symbols', action="store_true",
                            help="Do not create symbols package (default: false). Could be used with --strip")
        parser.add_argument('--compatibility-make', action="store_true",
                            help="Package Make build results (default: false)")
        parser.add_argument('--binary', type=str, default='tseserver.static',
                            help='Server binary (default: tseserver.static)')
        args = parser.parse_args()

        logger_mode = 'INFO'
        if args.debug:
            logger_mode = 'DEBUG'
        elif args.silent:
            logger_mode = 'CRITICAL'
        logging.basicConfig(level=getattr(logging, logger_mode),
                            format="%(asctime)s %(message)s",
                            datefmt='%Y-%m-%d %H:%M:%S',
                            stream=sys.stdout)

        hdlr = logging.StreamHandler()
        hdlr.setFormatter(logging.Formatter('%(message)s'))
        logger_csf.addHandler(hdlr)
        logger_csf.propagate = False

        logger.debug("Done with options parsing")
        logger.debug(args)

        baseline = args.baseline
        build_subdirectory = args.build_directory
        stage = args.stage
        strip = args.strip
        drop_symbols = args.drop_symbols
        binary = args.binary

        if args.compatibility_make:
            logger.debug("Make compatibility mode")
            configuration = MakeConfiguration(baseline,
                                              build_subdirectory,
                                              stage,
                                              strip,
                                              drop_symbols,
                                              binary)
        else:
            logger.debug("Scons compatibility mode")
            configuration = SconsConfiguration(baseline,
                                               build_subdirectory,
                                               stage,
                                               strip,
                                               drop_symbols,
                                               binary)

        logger.debug(configuration)

        configuration.collect()
        configuration.verify()

        if PackageCreatorManager(configuration).process():
            return 0

    except PackageCreatorError as exc:
        logger.critical(str(exc.args))
    except Exception, e:
        logger.critical(str(e))

    return 1


if __name__ == "__main__":
    sys.exit(main())
