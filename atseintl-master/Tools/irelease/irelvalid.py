#! /opt/python-2.6.1/bin/python

from release_config import ConfigBuilder
from release_validator import ReleaseNoteValidator, HTMLDecoratingValidationHandler
from release_parser import ReleaseNoteFileParser

import getopt
import os
import sys


def usage():
    print """
====================================================
===          Release Notes Validator             ===
====================================================

Usage:
    $ python irelvalid.py < Release/release_2012_09
    $ python irelvalid.py -f Release/release_2012_09

Options:
    -f, --file  - releases file to validate (default is stdin)
    -c, --conf  - configuration file (default is ./irel.xml)
    -h, --help  - shows this usage message

"""

def main(argv):
    filename = "";

    if sys.argv[0] == "irelvalid.py":
        scriptpath = os.getcwd()
    else:
        scriptpath = sys.argv[0].replace(os.sep + 'irelvalid.py','') 

    configfile = scriptpath + os.sep + "irel.xml"

    try:                                
        opts, args = getopt.getopt(argv, "f:c:h", ["file=", "conf=", "help"])
    except getopt.GetoptError, msg:
        print "Wrong parameters! Exiting!";
        print msg;
        print "Try -h for options."
        sys.exit(2) 
        
    for opt, arg in opts:
        if opt in ("-f","--file"):
            filename = arg
        elif opt in ("-h","--help"):
            usage()
            sys.exit(0)   
        elif opt in ("-c", "--conf"):
            configfile = arg   

    configuration = ConfigBuilder(configfile).build()

    if filename != "":
        f = open(filename, 'r')
    else:
        f = sys.stdin

    content = ReleaseNoteFileParser().parseFile(f)

    if filename != "":
        f.close()

    handler = HTMLDecoratingValidationHandler()
    validator = ReleaseNoteValidator(handler)
    validator.validate(content, configuration)

    errors = handler.getErrors()

    if not errors:
        print 'OK'

        #success
        sys.exit(0)

    else:
        print 'Validation failed:'
        for i,error in enumerate(errors):
            print "{0}: {1}".format(i+1,error)

        #failure
        sys.exit(1)

if __name__ == "__main__":
    main(sys.argv[1:])

        
