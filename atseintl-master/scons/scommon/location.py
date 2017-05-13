#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import os

SCONS_ROOT = '#'

def path(scons_node):
    return Node(scons_node).dest

def abspath(scons_node):
    return Node(scons_node).absdest

class Node(object):
    def __init__(self, scons_node):
        self.node = scons_node

    @property
    def orig(self):
        return self.node.srcnode().path

    @property
    def absorig(self):
        return self.node.srcnode().abspath

    @property
    def dest(self):
        return self.node.path

    @property
    def absdest(self):
        return self.node.abspath

    def __str__(self):
        return self.dest

    @property
    def basename(self):
        return os.path.basename(self.dest)

