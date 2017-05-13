#!/opt/python-2.6.1/bin/python
import os
import sys
import re
import getopt
import shutil
import traceback
import filecmp
import subprocess
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

from release_config import ConfigBuilder
from release_parser import ReleaseNoteFileParser, ReleaseNoteFileDiffer
from release_html import HtmlOutputProcessor
from release_validator import HTMLDecoratingValidationHandler, ReleaseNoteValidator

RELEASES_DIR = '/vobs/atseintl/Releases/others'
#PREV_RELEASES_DIR = '/vobs/atseintl/Releases/.previous_files'
PREV_RELEASES_DIR = '/vobs/atseintl/Tools/irelease/previous_files'
EMAIL_LIST_FILE = '/vobs/atseintl/Tools/irelease/subscribers.txt'


def gen_html_diff(old_file, new_file):
    scriptpath = ""
    
    if sys.argv[0] == "notify_on_changes.py":
        scriptpath = os.getcwd()
    else:
        scriptpath = sys.argv[0].replace(os.sep + 'notify_on_changes.py','') 

    configfile = scriptpath + os.sep + "irel.xml"
    configuration = ConfigBuilder(configfile).build()

    with open(old_file) as old:
        oldContent = ReleaseNoteFileParser().parseFile(old)

    with open(new_file) as new:
        newContent = ReleaseNoteFileParser().parseFile(new)

    diffContent = ReleaseNoteFileDiffer().compare(oldContent, newContent)

    handler = HTMLDecoratingValidationHandler()
    validator = ReleaseNoteValidator(handler)
    validator.validate(diffContent, configuration)

    validatedContent = handler.getResult()
    errors = handler.getErrors()

    htmlDiffContent = HtmlOutputProcessor().decorate(validatedContent, configuration, errors)
    return htmlDiffContent


def send_message(subject, message_body, to_address):
    from_address = 'nobody@sabre.com'
    #to_address = 'marcin.wrochniak.ctr@sabre.com'

    msg = MIMEText(message_body, 'html')
    msg['Subject'] = subject
    msg['From'] = from_address
    msg['To'] = to_address

    s = smtplib.SMTP('smtp.sabre.com', 25)
    s.sendmail(from_address, [to_address], msg.as_string())
    s.quit()


def distribute_message(subject, message_body):
   with open(EMAIL_LIST_FILE, 'r') as file:
      for line in file:
         to_address = line.strip()
         if to_address:
            send_message(subject, message_body, to_address)


def run_cmd(cmd):
    "Returns cmd's output"
    popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
    popen.wait()
    return '\n'.join(popen.stdout.readlines())


class SCM:

    def __init__(self):
        self.checkedout_files = []

    def _run_cleartool_cmd(self, cmd):
        return run_cmd('/usr/atria/bin/cleartool ' + cmd)

    def _is_file_checkedout(self, filename):
        "ClearCase-specific"
        if filename in self.checkedout_files:
            return True
        if os.path.isdir(filename):
            parent_dir = os.path.dirname(filename.rstrip('/'))
            ct_ls_output = self._run_cleartool_cmd("ls '%s'" % parent_dir)
            for output_line in ct_ls_output.split('\n'):
                if output_line.startswith(filename.rstrip('/') + '@@'):
                    return "Rule: CHECKEDOUT" in output_line
            return False
        else:
           return "Rule: CHECKEDOUT" in self._run_cleartool_cmd("ls '%s'" % filename)

    def _checkout_file(self, filename):
        "ClearCase-specific"
        self._run_cleartool_cmd("co -nc '%s'" % filename)
        self.checkedout_files.append(filename)

    def _checkin_file(self, filename):
        "ClearCase-specific"
        self._run_cleartool_cmd("ci -c 'Automatic irelease check-in' '%s'" % filename)

    def create_file(self, filename):
        if not self._is_file_checkedout(os.path.dirname(filename)):
            self._checkout_file(os.path.dirname(filename))
        self._run_cleartool_cmd("mkelem -nc '%s'" % filename)
        self.checkedout_files.append(filename)

    def copy_file(self, from_filename, to_filename):
        if not os.path.isfile(to_filename):
            self.create_file(to_filename)
        elif not self._is_file_checkedout(to_filename):
            self._checkout_file(to_filename)
        run_cmd("cp '%s' '%s'" % (from_filename, to_filename))

    def commit(self):
        print 'Files to commit:', self.checkedout_files
        for filename in sorted(self.checkedout_files):
            self._checkin_file(filename)
        self.checkedout_files = []

def full_rel_filename(filename):
    return '%s/%s' % (RELEASES_DIR, filename)

def full_old_rel_filename(filename):
    return '%s/%s' % (PREV_RELEASES_DIR, filename)

def send_notifications(scm):
    modified_files = []
    for filename in [f for f in os.listdir(RELEASES_DIR)
                     if os.path.isfile(full_rel_filename(f)) and f.startswith('release_')]:

        current_full_filename = full_rel_filename(filename)
        previous_full_filename = full_old_rel_filename(filename)

        if not os.path.isfile(previous_full_filename):
            scm.create_file(previous_full_filename)
 
        if not filecmp.cmp(previous_full_filename, current_full_filename):
            html_message = gen_html_diff(previous_full_filename, current_full_filename)
            print 'file differs', filename          
            distribute_message('New deliveries in ' + filename, html_message)
            modified_files.append(filename)
    return modified_files

def save_copies(scm, modified_files):
    #for filename in [f for f in os.listdir(RELEASES_DIR)
    #                 if os.path.isfile(full_rel_filename(f)) and f.startswith('release_')]:
    print 'Modified files:', modified_files
    for filename in modified_files:
        scm.copy_file(full_rel_filename(filename), full_old_rel_filename(filename))

def main(argv):
    #print gen_html_diff('a.txt', 'b.txt')
    scm = SCM()
    modified_files = send_notifications(scm)
    save_copies(scm, modified_files)
    scm.commit()


if __name__ == '__main__':
    main(sys.argv)

