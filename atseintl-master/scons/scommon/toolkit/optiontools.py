#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2013

import logging
log = logging.getLogger(__name__)

from SCons.Script import *


def gen_current_variables(vars, env):
    '''
    yields key, value, default
    for each option
    '''
    try:
        for o in vars.options:
            yield (o.key, env[o.key], o.default)
    except Exception, e:
        log.error('Error while reading variables: %s' % e)

class Optreporter(object):

    def __init__(self):
        self.history = []

    def set_option(self, name, val, setter=SetOption, retriever=GetOption):
        setter(name, val)
        actual = retriever(name)
        log.debug('setting option %s to %s (actual %s)' % (name, val, actual))
        self.history.append((name, val, actual))

    def gen_options(self):
        '''
        Yields triples: (name, set_value, actual_value).
        If set_value != actual_value, it means that
        the set value was overridden.
        '''
        for (name, val, retrieved) in self.history:
            yield (name, val, retrieved)

_reporter = Optreporter()

def optreporter():
    return _reporter

def set_option(name, val):
    optreporter().set_option(name, val)

def log_summary(env, vars, sconsflags=None, optrep=None):
    if sconsflags is None:
        sconsflags = os.environ.get('SCONSFLAGS', '')
    if optrep is None:
        optrep = optreporter()

    if sconsflags:
        log.info('SCONSFLAGS env variable: %s' % sconsflags)
    else:
        log.debug('no SCONSFLAGS set')

    for (k, v, default) in gen_current_variables(vars, env):
        if v != default:
            log.info('variable %s set to %s (default %s)' % (k, v, default))
        else:
            log.debug('variable %s set to %s (same as default)' % (k, v))

    for name, set_value, actual_value in optrep.gen_options():
        if set_value != actual_value:
            log.info('option %s value %s overridden by %s' % (name, set_value, actual_value))
        else:
            log.info('option %s set to %s' % (name, set_value))

