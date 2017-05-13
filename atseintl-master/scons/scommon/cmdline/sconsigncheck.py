#!/usr/bin/env python

# Author: Piotr Bartosik (piotr.bartosik@sabre.com)
# Copyright Sabre 2014

import argparse
import hashlib
import os
import subprocess

def parse_args():
    INFO = '''Check if .sconsign.dblite contains correct md5 sums.
        Please call it from the SCons repository directory.'''
    parser = argparse.ArgumentParser(
        description=INFO,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--sconsign',
        help='Path to the sconsign application',
        default='/login/sg218694/tools/scons/sconsign.py')
    parser.add_argument(
        '--sconsignfile',
        help='The name of SCons database file, usually .sconsign.dblite',
        default='.sconsign.dblite')
    return parser.parse_args()

def get_from_sc(sconsign, sconsignfile):
    p = subprocess.Popen([sconsign, '-c', sconsignfile],
                         stdout=subprocess.PIPE)
    parser = Parser()
    for v in p.stdout:
        parser.fetch_line(v)

    for path, csum in parser.sums.iteritems():
        try:
            realsum = calculate_checksum(path)
        except IOError as e:
            pass
        else:
            if csum != realsum:
                print '%s %s %s' % (csum, realsum, path)


def calculate_checksum(path):
    with open(path) as f:
        content = f.read()
        m = hashlib.md5()
        m.update(content)
        return m.hexdigest()



class Parser(object):
    DIR_MARKER = '==='
    PREFIXES = ['sc_debug/', 'sc_release/']
    EXCEPTIONS = ['None']

    def __init__(self):
        self.sums = {}
        self.current_dir = None

    def fetch_line(self, v):
        v = v.strip()
        if v.startswith(self.DIR_MARKER):
            self._set_new_dir(v)
        else:
            self._add_sum(v)

    def _set_new_dir(self, v):
        v = v.split(self.DIR_MARKER)[1]
        v = v.strip(' :')
        for prefix in self.PREFIXES:
            if v.startswith(prefix):
                v = v[len(prefix):]
                break
        self.current_dir = v

    def _add_sum(self, v):
        file, csum = map(lambda x: x.strip(), v.split(':'))
        if csum in self.EXCEPTIONS:
            return
        k = os.path.join(self.current_dir, file)
        self.sums[k] = csum

def main():
    args = parse_args()
    get_from_sc(args.sconsign, args.sconsignfile)

if __name__ == '__main__': main()
