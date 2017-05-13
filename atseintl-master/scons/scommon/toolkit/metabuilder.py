#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import subprocess
import logging
log = logging.getLogger(__name__)
import os

from . import bridge
from .exceptions import MetabuilderError

class CommandExecutor(object):


    def __init__(self, cwd=None, spawn=None):
        '''
        Cmd should be a list
        '''
        self.cwd = cwd
        self.spawn = spawn or subprocess.check_call


    def __call__(self, renv, cmd):
        env = renv.get('ENV')
        log.debug('calling %s, env = %s', cmd, env)
        self.spawn(cmd, cwd=self.cwd, env=env)


class BuilderAction(object):
    def __init__(self, action, emitter=None):
        if action is None:
            raise MetabuilderError('action is None')
        self.action = action
        self.emitter = emitter


class ImplicitDepsAction(object):
    def __init__(self, act, implicit_deps):
        self.act = act
        self.implicit_deps = implicit_deps

    def emitter(self, target, source, env):
        if self.act.emitter is not None:
            target, source = self.act.emitter(target, source, env)

        self.original_source_size = len(source)
        for dep in self.implicit_deps:
            source.append(dep)
        return target, source

    def action(self, target, source, env):
        return self.act.action(target, source[:self.original_source_size], env)


class Metabuilder(object):
    BUILDER_PROPERTIES = ['src_suffix']

    def __init__(self, schema=None, builder_factory=None, executor_factory=None):

        self.builder_factory = builder_factory or bridge.builder_factory
        executor_factory = executor_factory or CommandExecutor
        self._executor = executor_factory()

        self.src_suffix = None
        self.action = None
        self.emitter = None

        self.implicit_dependencies = []

        if schema is not None:
            self.process_schema(schema)

    def add_implicit_deps(self, *dependencies):
        self.implicit_dependencies.extend(dependencies)

    def process_schema(self, schema):
        schema(self, self._executor)
        schema.executor = self._executor
        return self

    def _create_action(self):
        act = BuilderAction(self.action, self.emitter)
        if self.implicit_dependencies:
            return ImplicitDepsAction(act, self.implicit_dependencies)
        return act

    def _create_config_dict(self):
        builder_action = self._create_action()
        d = dict(action=builder_action.action)
        if  builder_action.emitter is not None:
            d['emitter'] = builder_action.emitter
        for key in self.BUILDER_PROPERTIES:
            v = getattr(self, key)
            if v is not None:
                d[key] = v
        return d

    def build(self):
        return self.builder_factory(**self._create_config_dict())

