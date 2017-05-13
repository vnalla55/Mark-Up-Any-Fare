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

def send_message(subject, message_body):
    from_address = 'nobody@sabre.com'
    to_address = 'marcin.wrochniak.ctr@sabre.com'

    msg = MIMEText(message_body)
    msg['Subject'] = subject
    msg['From'] = from_address
    msg['To'] = to_address

    s = smtplib.SMTP('smtp.sabre.com', 25)
    s.sendmail(from_address, [to_address], msg.as_string())
    s.quit()


def run_cmd(cmd):
    "Returns cmd's output"
    popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
    popen.wait()
    return '\n'.join(popen.stdout.readlines())


if len(sys.argv) <= 1:
    print 'Usage: ./record_use.py filename'
    #sys.exit(-1)

with file(sys.argv[1]) as f:
    s = f.read()
    send_message("TOOLUSAGE", s)
