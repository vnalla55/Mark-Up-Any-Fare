#!/usr/bin/python

SOURCE_REPO = '/atse_git/sg217418/atseintl4rnt'
TARGET_REPO = '/atse_git/sg217418/atseintl4rnt_prev_release'
DIR = 'Releases'


FIRST_IN_YEAR = 0
LAST_IN_YEAR = 12

import os
import re
import filecmp
import datetime

rel_re = re.compile(r'release_(\d{1,4})_(\d{1,2})')

def full_source_file_name(release_file):
   return '%s/%s/%s' % (SOURCE_REPO, DIR, release_file)

def full_target_file_name(release_file):
   return '%s/%s/%s' % (TARGET_REPO, DIR, release_file)


class BadNameError:
   pass


class ReleaseFile:

   def __init__(self, file_name):
      match = rel_re.match(file_name)
      if not match:
         raise BadNameError()
      self._first_num, self._second_num = match.groups()

   def __str__(self):
      return "release_%s_%s" % (self._first_num, self._second_num)

   def __lt__(self, other):
      if self._first_num != other._first_num:
         return self._first_num < other._first_num
      return self._second_num < other._second_num

   def branch_name(self):
      first, second = int(self._first_num), int(self._second_num)
      return "atsev2.%d.%02d" % (first, second)

   def previous(self):
      first, second = int(self._first_num), int(self._second_num)
      if second == 0:
         first -= 1
         second = LAST_IN_YEAR
      else:
         second -= 1
      return ReleaseFile("release_%d_%02d" % (first, second))


def get_newest_release_file():
   newest_file = ReleaseFile("release_0_0")
   for file_name in os.listdir(SOURCE_REPO + '/' + DIR):
      try:
         release_file = ReleaseFile(file_name)
         if newest_file < release_file:
            newest_file = release_file
      except BadNameError:
         continue
   return newest_file

def get_prepare_cmds():
   prev = get_newest_release_file().previous()
   return ['cd "%s" && git pull --rebase && git checkout %s' % (TARGET_REPO, prev.branch_name())]

def get_cmds_to_run():
   cmds_to_run = []

   prev = get_newest_release_file().previous()

   full_current = full_source_file_name(prev)
   full_prev = full_target_file_name(prev)

   if filecmp.cmp(full_current, full_prev, shallow=True):
      return cmds_to_run

   cmds_to_run.append('cp "%s" "%s"' % (full_current, full_prev))
   cmds_to_run.append('cd "%s" && git commit -a -m "Sync release notes" && git push origin %s' % (TARGET_REPO, prev.branch_name()))
   return cmds_to_run

def run_cmds(cmds):
   for cmd in cmds:
      print 'Running ' + cmd
      ret = os.system(cmd)
      print 'Returned %d' % ret


run_cmds(get_prepare_cmds())
run_cmds(get_cmds_to_run())

print 'newest', str(get_newest_release_file())
print datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'), "\n"
