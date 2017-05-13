#!/usr/bin/env python

import sys
import subprocess
import logging
import shutil
import errno
import os
from argparse import ArgumentParser
from packagecreator import SconsConfiguration, PackageCreatorError, PackageCreatorManager

logger = logging.getLogger(__name__)


def compose_build_command(baseline, builddir, suffix):
    build_cmd = []
    build_cmd.append('scons')
    build_cmd.append('--allvars')
    build_cmd.append('--no-cache')
    build_cmd.append('--debug=time')
    build_cmd.append('--jobs=264')
    build_cmd.append('BUILDDIR=' + builddir)
    build_cmd.append('--userconfig=scons/.release.build.cfg' + ('.' + suffix if suffix != '' else ''))
    build_cmd.append('BUILD_LABEL=%s' % baseline)
    build_cmd.append('package')
    return build_cmd


def build_applications(configuration, suffix=''):
    cmd = compose_build_command(configuration.baseline, configuration.build_subdirectory, suffix)
    logger.info("Build cmd %s" % cmd)

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in iter(process.stdout.readline, ''):
        logger.info(line.strip())
    return process.wait() == 0


def main(argv=None):
    try:
        parser = ArgumentParser(description='ATSEv2 baseline package builder manager')
        parser.add_argument('--baseline', type=str, required=True,
                            help='Baseline <family>.<release> Ex. atsev2.2015.00.00')
        parser.add_argument('--stage', action="store_true",
                            help="Stage package (default: false)")
        parser.add_argument('--build-directory', type=str, default='sc_release',
                            help='Build directory (default: sc_release)')
        parser.add_argument('--strip', action="store_true",
                            help="Strip debug symbols info separate package (default: false)")
        parser.add_argument('--drop-symbols', action="store_true",
                            help="Do not create symbols package (default: false). Could be used with --strip")
        parser.add_argument('--add-suffixes', type=str, default='fallback',
                            help="Different suffixes for config file (for different compilers and etc.)")
        args = parser.parse_args()

        logging.basicConfig(level=getattr(logging, 'INFO'),
                            format="%(message)s",
                            datefmt='%Y-%m-%d %H:%M:%S',
                            stream=sys.stdout)

        configuration = SconsConfiguration(args.baseline,
                                           args.build_directory,
                                           args.stage,
                                           args.strip,
                                           args.drop_symbols)

        if not build_applications(configuration):
                return 1
        configuration.collect()
        logger.debug(configuration)
        configuration.verify()

        for suff in args.add_suffixes.split(','):
            if len(suff) == 0:
                continue
            curr_config = SconsConfiguration(args.baseline,
                                              args.build_directory + '.' + suff,
                                              args.stage,
                                              args.strip,
                                              args.drop_symbols)

            if not build_applications(curr_config, suff):
                return 1
            curr_config.collect()
            logger.debug(curr_config)
            curr_config.verify()
            os.link(curr_config.binary_path, configuration.binary_path + '.' + suff)

        if not PackageCreatorManager(configuration).process():
                return 1

        return 0

    except PackageCreatorError as exc:
        logger.critical(str(exc.args))
    except Exception, e:
        logger.critical(str(e))

    return 1


if __name__ == "__main__":
    sys.exit(main())
