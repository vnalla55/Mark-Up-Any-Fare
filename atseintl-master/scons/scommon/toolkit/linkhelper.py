#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

from .exceptions import NoLibsError, DuplicateKeyError

class Linkhelper(object):

    WHOLE_ARCHIVE_START_PAIR = ('WHOLE_ARCHIVE_START', '-Wl,-whole-archive')
    WHOLE_ARCHIVE_END_PAIR = ('WHOLE_ARCHIVE_END', '-Wl,-no-whole-archive')
    WHOLE_ARCHIVE_LIBS_VARIABLE = 'WHOLE_ARCHIVE_LIBS'
    NO_WHOLE_ARCHIVE_LIBS_VARIABLE = 'NO_WHOLE_ARCHIVE_LIBS'
    LINK_COMMAND_TEMPLATE = ' '.join([
            "$LINK -o $TARGET $SOURCES $LINKFLAGS $_LIBDIRFLAGS",
            "${wastart} ${walibs} ${waend} ${nowalibs}"])

    LIBLINK_TEMPLATE = '${%s}%s${%s}'
    LIBLINKPREFIX_VAR = 'LIBLINKPREFIX'
    LIBLINKSUFFIX_VAR = 'LIBLINKSUFFIX'
    LINKER_COMMAND_VAR = 'LINKCOM'

    def __init__(self, env,
            whole_archive_start_pair=None,
            whole_archive_end_pair=None,
            whole_archive_libs_variable=None,
            no_whole_archive_libs_variable=None):

        self.env = env
        self.libnames = []
        self.libnames_wa = []

        self.whole_archive_start_pair = self.WHOLE_ARCHIVE_START_PAIR \
                if whole_archive_start_pair is None else whole_archive_start_pair
        self.whole_archive_end_pair = self.WHOLE_ARCHIVE_END_PAIR \
                if whole_archive_end_pair is None else whole_archive_end_pair
        self.whole_archive_libs_variable = self.WHOLE_ARCHIVE_LIBS_VARIABLE \
                if whole_archive_libs_variable is None else whole_archive_libs_variable
        self.no_whole_archive_libs_variable = self.NO_WHOLE_ARCHIVE_LIBS_VARIABLE \
                if no_whole_archive_libs_variable is None else no_whole_archive_libs_variable

    def add_libnames(self, *libnames):
        self.libnames.extend(libnames)

    def add_libnames_whole_archive(self, *libnames):
        self.libnames_wa.extend(libnames)

    def create_link_command_dict(self):
        d = {}
        d[self.whole_archive_start_pair[0]] = self.whole_archive_start_pair[1]
        d[self.whole_archive_end_pair[0]] = self.whole_archive_end_pair[1]
        d[self.whole_archive_libs_variable] = [self._add_liblink_prefix_suffix(x) for x in self.libnames_wa]
        d[self.no_whole_archive_libs_variable] = [self._add_liblink_prefix_suffix(x) for x in self.libnames]
        d[self.LINKER_COMMAND_VAR] = self.LINK_COMMAND_TEMPLATE.format(
                wastart=self.whole_archive_start_pair[0],
                walibs=self.whole_archive_libs_variable,
                waend=self.whole_archive_end_pair[0],
                nowalibs=self.no_whole_archive_libs_variable)
        return d

    def update_env(self):
        if not self._all_libs():
            raise NoLibsError('No input library names')

        self.env.add_libs(*(self._all_libs()))
        if self.libnames_wa:
            self._assert_no_name_clashes()
            for k, v in self.create_link_command_dict().iteritems():
                self.env[k] = v
            import pprint
            pprint.pprint(self.create_link_command_dict())

    def _all_libs(self):
        return self.libnames + self.libnames_wa

    def _get_injected_variables(self):
        return [self.whole_archive_libs_variable, self.no_whole_archive_libs_variable,
                self.whole_archive_start_pair[0], self.whole_archive_end_pair[0]]

    def _assert_no_name_clashes(self):
        for key in self._get_injected_variables():
            if key in self.env:
                raise DuplicateKeyError('duplicate key %s in env' % key)

    def _add_liblink_prefix_suffix(self, libname):
        return self.LIBLINK_TEMPLATE % (self.LIBLINKPREFIX_VAR, libname, self.LIBLINKSUFFIX_VAR)

