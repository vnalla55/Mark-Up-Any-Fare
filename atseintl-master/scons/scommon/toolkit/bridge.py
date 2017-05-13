#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

# global tools like Copy etc.
import SCons.Script


def split_sources(sources):
    return SCons.Script.Split(sources)

def set_cache_directory(path):
    SCons.Script.CacheDir(path)

def builder_factory(*args, **kwargs):
    return SCons.Script.Builder(*args, **kwargs)

def always_build(nodes):
    SCons.Script.AlwaysBuild(nodes)

def no_cache(nodes):
    SCons.Script.NoCache(nodes)

def depends(what, on_what):
    SCons.Script.Depends(what, on_what)

def get_targets():
    return map(str, SCons.Script.BUILD_TARGETS)
