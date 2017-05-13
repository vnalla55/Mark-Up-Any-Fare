#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import logging
from SCons.Script import *

log = logging.getLogger(__name__)

class VariableProxy(object):
    def __init__(self, key, help, default, factory=None, **kwargs):
        self.key = key
        self.help = help
        self.default = default
        self.kwargs = kwargs
        self.factory = factory

    def install(self, vars):
        if self.factory:
            v = self.factory(key=self.key, help=self.help,
                    default=self.default, **self.kwargs)
            vars.Add(v)
        else:
            vars.Add(key=self.key, help=self.help,
                    default=self.default, **self.kwargs)

    def get_defaults(self):
        return self.key, self.default


class VarAdder(object):
    def __init__(self, var_proxy_factory=None):
        self.proxies = []
        self.var_proxy_factory = var_proxy_factory or VariableProxy

    def variable(self, key, help, default, factory=None, minor=False, **kwargs):
        t = self.var_proxy_factory(key=key, help=help, default=default, factory=factory, **kwargs)
        self.proxies.append((t, minor))

    def add_variables(self, vars, major_only=False):
        d = {}
        for p, minor in self.proxies:
            if major_only and minor:
                k, v = p.get_defaults()
                d[k] = v
            else:
                p.install(vars)
        return d

