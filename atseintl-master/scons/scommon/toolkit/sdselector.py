#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import collections
import logging
log = logging.getLogger(__name__)

import pytools
import os

from .exceptions import HandOver

class ScriptDirSelector(object):
    def __init__(self):
        self.head_algorithm = None
        self.tail_algorithm = None
        self.dir_groups = {}

    def add_next_algorithm(self, groups_algorithm):
        # empty: init both algorithms
        if self.head_algorithm is None:
            self.head_algorithm = groups_algorithm
            self.tail_algorithm = groups_algorithm
            return

        # at least one algo present: update tail
        self.tail_algorithm.setnext(groups_algorithm)
        self.tail_algorithm = groups_algorithm

    def add_dir_group(self, name, *dirs):
        self.dir_groups.setdefault(name, []).extend(dirs)

    def get_dirs(self, targets):
        log.debug('targets = %s' % targets)
        groups = self.head_algorithm.get_groups(targets)
        log.debug('groups = %s' % groups)
        dirs = self._dirs_for_groups(groups)
        log.debug('dirs = %s' % dirs)
        return dirs

    def _dirs_for_groups(self, groups):
        dirs = []
        for g in groups:
            dirs.extend(self.dir_groups[g])
        return dirs


class GroupsAlgorithm(object):
    def setnext(self, next):
        self.next = next

    def get_groups(self, targets):
        try:
            return self._groups(targets)
        except HandOver, e:
            log.debug('Unable to calc with reason: %s, trying next', str(e))
            return self.next.get_groups(targets)


class TargetNameAlgorithm(GroupsAlgorithm):
    def __init__(self):
        self.groups_for_targets = {}

    def add_groups_for_target(self, target, *dir_group_names):
        self.groups_for_targets.setdefault(target, []).extend(dir_group_names)

    def _group_names_for_targets(self, targets):
        groups = collections.OrderedDict()
        for t in targets:
            for name in self.groups_for_targets[t]:
                # deduplicate and maintain order
                groups[name] = None
        return list(groups)

    def _groups(self, targets):
        try:
            gg = self._group_names_for_targets(targets)
            log.debug('getting groups %s for targets %s', gg, targets)
            return gg
        except KeyError, e:
            raise HandOver('unknown target = %s' % str(e))


class PathAlgorithm(GroupsAlgorithm):
    def __init__(self, root):
        self.root = root
        self.groups_for_prefixes = {}

    def add_groups_for_prefix(self, prefix, *dir_group_names):
        self.groups_for_prefixes.setdefault(prefix, []).extend(dir_group_names)

    def _path_exists(self, path):
        return os.path.exists(os.path.join(self.root, path))

    def _get_prefixes_from_longest(self):
        return sorted(self.groups_for_prefixes.keys(),
                key=lambda prefix: len(prefix), reverse=1)

    def _calculate_prefixes(self, targets):
        pfromlongest = self._get_prefixes_from_longest()
        found_prefixes = set()
        for t in targets:
            if not self._path_exists(t):
                raise HandOver('target %s is not a valid path, cwd = %s' % (t, os.getcwd()))
            for p in pfromlongest:
                if pytools.is_subdir(t, p):
                    found_prefixes.add(p)
                    log.debug('%s is prefix of %s', p, t)
                    break
            else:
                raise HandOver('no prefix for path %s' % t)
        return found_prefixes

    def _groups(self, targets):
        prefixes = self._calculate_prefixes(targets)
        groups = collections.OrderedDict()
        for p in prefixes:
            for name in self.groups_for_prefixes[p]:
                groups[name] = None
        gg = list(groups)
        log.debug('returning groups = %s' % gg)
        return gg


class DefaultGroupsAlgorithm(GroupsAlgorithm):
    def __init__(self, *default_groups):
        self.default_groups = list(default_groups)

    def _groups(self, targets):
        return self.default_groups

