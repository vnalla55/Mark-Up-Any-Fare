#!/usr/bin/env python

import subprocess
import sys
import os
import getpass

CONTEXT_FILE='acms-bisect-context'
STATUS_FILE='acms-bisect-status'
CONFIG_FILE='acms-bisect.cfg'

# command
def fetch_baseline_configuration(baseline, output):
    user = getpass.getuser()
    host = os.uname()[1]

    command = "/opt/atse/common/acmsquery/acmsquery.sh \
                 -svchost 172.30.247.217 -svcport 8181 \
                 -baseline {baseline} \
                 -user {user} \
                 -FAM atsev2 \
                 -APP shopping \
                 -ENV dev \
                 -NOD {host} \
                 -exportv2 {output}"\
            .format(user=user, host=host, baseline=baseline, output=output)

    print 'saving configuration to', output
    subprocess.call(command.split())

def acms_console(acms_command):
    command = ['/opt/atse/acmsconsole/acmsconsole.sh',
               '-host', 'hybd05.dev',
               '-password', 'guest',
               '-exec', acms_command]

    return subprocess.check_output(command)

def load_baselines():
    raw_baseline_listing = acms_console("lsbl -p atsev2 -d")
    baselines = raw_baseline_listing.splitlines()[1:]

    i = 0
    with open(CONTEXT_FILE, 'w') as ctx:
        for baseline in baselines:
            ctx.write(str(i)+'  '+baseline+'\n')
            i += 1

    return i-1

def initialize_context(good_date):
    last = load_baselines()

    good=0
    baselines = load_context()
    for baseline in baselines:
        if baseline[2]<=good_date:
            good += 1

    bad=last

    print 'good', good
    print 'bad', bad

    if (good==bad):
        print 'GOOD and BAD are the same... nothing to bisect'
        sys.exit(1)

    head=(good+bad)/2

    checkout_revision(head)
    save_status(good, bad, head)


def save_status(good, bad, head):
    with open(STATUS_FILE, 'w') as stat:
        stat.write('good={0}\n'.format(good))
        stat.write('bad={0}\n'.format(bad))
        stat.write('HEAD={0}\n'.format(head))

def load_status():
    with open(STATUS_FILE, 'r') as stat:
        content = map(lambda x: x.strip().split('='), stat.readlines())
        return (int(content[0][1]), int(content[1][1]), int(content[2][1]))

def load_context():
    with open(CONTEXT_FILE, 'r') as ctx:
        baselines = map(lambda x: x.strip().split('  '), ctx.readlines())
        return baselines

def checkout_revision(number):
    baseline = load_context()[number]
    print 'checkout revision:', ' '.join(baseline)
    fetch_baseline_configuration(baseline[1], CONFIG_FILE)

def report_offending_revision(rev):
    baseline = load_context()[rev]
    print 'bug introduced in revision:', rev, 'date:', baseline[2]
    print 'all activities for that baseline:'
    print acms_console('lsact -b {0}'.format(baseline[3]))
    print '---'
    checkout_revision(rev)

def mark_good():
    (good, bad, head) = load_status()
    new_head = (bad+head)/2
    new_good = head

    if (new_good+1 == bad):
        report_offending_revision(bad)
        save_status(new_good, bad, bad)
    else:
        checkout_revision(new_head)
        save_status(new_good, bad, new_head)

def mark_bad():
    (good, bad, head) = load_status()

    new_head = (good+head)/2
    new_bad = head

    if (good+1 == new_bad):
        report_offending_revision(new_bad)
        save_status(good, bad, new_bad)
    else:
        checkout_revision(new_head)
        save_status(good, new_bad, new_head)

def clean_files():
    print 'removing',CONTEXT_FILE
    os.remove(CONTEXT_FILE)

    print 'removing',STATUS_FILE
    os.remove(STATUS_FILE)

    print 'removing',CONFIG_FILE
    os.remove(CONFIG_FILE)

def print_usage():
    print 'usage acms-bisect.py <command>'
    print ''
    print 'Valid commands are:'
    print ''
    print 'start YYYY-MM-DD'
    print '  mark today as bad, the given date as good and grab the config'
    print '  in the middle'
    print ''
    print 'good'
    print '  mark current position as good and move towards the bad one'
    print ''
    print 'bad'
    print '  mark current position as bad and move towards the good one'
    print ''
    print 'reset'
    print '  remove all files create to track context and the configuration'

def main():
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)

    cmd = sys.argv[1]
    if cmd == 'start':
        if len(sys.argv)==3:
            initialize_context(sys.argv[2])
        else:
            print 'usage acms.bisect.py start <last-known-good-date>'
            sys.exit(1)

    elif cmd == 'good':
        mark_good()

    elif cmd == 'bad':
        mark_bad()

    elif cmd == 'reset':
        clean_files()

if __name__ == "__main__":
    main()
