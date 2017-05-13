#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import os
import logging
log = logging.getLogger(__name__)
import imp

import pytools
import pytools.command
import pprint

from .exceptions import NoTestsError

def pformat(x):
    pretty = pprint.pformat(x)
    if '\n' in pretty:
        return pretty.join(['\n', '\n'])
    return pretty

def list_summary(a, threshold=40, prefixlen=5):
    la = len(a)
    if la <= threshold:
        return str(a)
    return '<list with %s elements, summary: %s>' % (la, str(a[:prefixlen] + ['...'] + a[-prefixlen:]))

# Alternative implementation of merging a dictionary
# to a Scons environment object.

def merge(env, d, tcheck=pytools.typecheck):
    '''
    Merges a dictionary "d" to a Scons environment "env".
    '''

    for k, v in d.iteritems():
        t = tcheck(v)
        if t == 'l':
            env.append_unique(k, v)
        else:
            env[k] = v


def _exclude(relative_paths, excludes):
    if excludes:
        for excluded_rel_path in excludes:
            relative_paths.remove(excluded_rel_path)

def glob(env, expression, excludes=None):
    relative_paths = env.Glob(expression, source=True, strings=True)
    _exclude(relative_paths, excludes)
    return relative_paths

def multiglob(env, *expressions, **kwargs):
    relative_paths = []
    for e in expressions:
        relative_paths.extend(env.Glob(e, source=True, strings=True))
    _exclude(relative_paths, kwargs.get('excludes', None))
    return relative_paths



def strshell(text):
    '''
    For text returns '"text"'
    Used to place c-strings in gcc definitions, e.g. -DMYPATH='"/atse/tmp/myfile1.txt"')
    '''
    return "'" + '"' + text + '"' + "'"


class NodeCollector(object):
    def __init__(self, storage, nodetokey=None):
        self.storage = storage
        self.nodetokey = nodetokey or self.default_nodetokey

    def default_nodetokey(self, node):
        return os.path.splitext(str(node))[0]

    def __call__(self, node):
        key = self.nodetokey(node)
        if key in self.storage:
            raise ValueError('object duplication: %s' % key)
        self.storage[key] = node
        log.debug('node %s put under key %s' % (str(node), key))

    def retrieve_nodes(self, filenames, excludes=None):
        excludes = excludes or []
        res = []
        for f in filenames:
            key = self.nodetokey(f)
            if f in excludes or key in excludes:
                log.debug('dropping %s' % f)
                continue
            res.append(self.storage[key])
        return res



class Testbank(object):

    def __init__(self, bld_root, run_root):
        self.bld_root = bld_root.strip(os.sep)
        self.run_root = run_root.strip(os.sep)

        # {path: [tsrecord1, tsrecord2, ...]}
        self.tsrecords = {}

    def _simplify_path(self, path):
        try:
            return pytools.subtract_paths(path, self.bld_root)
        except pytools.PathSubtractionError:
            return pytools.subtract_paths(path, self.run_root)

    def add_tsrecord(self, bld_path, tsrecord):
        path = self._simplify_path(bld_path)
        path_records = self.tsrecords.setdefault(path, [])
        if tsrecord.name in [r.name for r in path_records]:
            raise ValueError('Suite with name %s already exists, choose different name' % tsrecord.name)
        log.debug('inserting %s tests for path %s, executor %s, name %s' % (len(tsrecord.tests), path, tsrecord.executor, tsrecord.name))
        path_records.append(tsrecord)

    def get_tsrecords(self, run_path):
        path = self._simplify_path(run_path)
        log.debug('returning tests for path %s' % path)
        try:
            return self.tsrecords[path]
        except KeyError:
            raise NoTestsError('No tests for path %s' % path)


class TaskToggles(object):
    def __init__(self, command_factory=None):
        self.tasks = []
        self.command_factory = command_factory or pytools.command.command

    def task(self, callable, *args):
        return self.command_factory(callable, *args, logging_func=log.debug)

    def add_task(self, task, enabled=True):
        self.tasks.append(self._task_entry(task, enabled))

    def replace_task(self, replaced_task_name, task, enabled=True):
        swaphere = self._task_index(replaced_task_name)
        self.tasks[swaphere] = self._task_entry(task, enabled)

    def enable_task(self, task_name, enabled=True):
        sethere = self._task_index(task_name)
        self.tasks[sethere][1] = enabled

    def clear_tasks(self):
        self.tasks = []

    def run(self):
        for task, enabled in self.tasks:
            if enabled:
                task()

    def _task_entry(self, task, enabled):
        return [task, enabled]

    def _task_index(self, task_name):
        return [self._taskname(t) for (t, ena) in self.tasks].index(task_name)

    def _taskname(self, task):
        return str(task)

