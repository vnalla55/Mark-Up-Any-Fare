#! /usr/bin/env python2.5
import os
import sys
import re
import getopt
import shutil
import traceback

from release_config import ConfigBuilder
from release_parser import ReleaseNoteFileParser, ReleaseNoteFileDiffer
from release_html import HtmlOutputProcessor
from release_validator import HTMLDecoratingValidationHandler, ReleaseNoteValidator

def usage():
    print """
====================================================
===          Release Notes Differ                ===
====================================================

Usage:
    $ python ireldiff.py -o Release/release_2012_09_old -n Release/release_2012_09

Options:
    -o, --oldfile   - Old file name
    -n, --newfile   - New file name
    -c, --conf      - Configuration file (default is ./irel.xml)
    -h, --help      - Shows this message

"""

def main(argv):
    try:
        #command line check and parse
        old_file = ""
        new_file = ""
        scriptpath = ""
        
        if sys.argv[0] == "ireldiff.py":
            scriptpath = os.getcwd()
        else:
            scriptpath = sys.argv[0].replace(os.sep + 'ireldiff.py','') 

        configfile = scriptpath + os.sep + "irel.xml"

        try:                                
            opts, args = getopt.getopt(argv, "o:n:c:h", ["oldfile=", "newfile=", "conf=", "help"])
        except getopt.GetoptError, msg:
            print "Wrong parameters! Exiting!";
            print msg;
            print "Try -h for options."
            sys.exit(2) 
            
        for opt, arg in opts:
            if opt in ("-o","--oldfile"):
                old_file = arg
            elif opt in ("-n","--newfile"):
                new_file = arg
            elif opt in ("-h","--help"):
                usage()
                sys.exit(0)
            elif opt in ("-c", "--conf"):
                configfile = arg

        if old_file == "" or new_file == "":
            print "Wrong parametes. Try -h for help!";
            sys.exit(2)
            
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

        sys.stdout.write(htmlDiffContent)

    except Exception as e:
        print r"""<html><body>
            <h1> An error occured in HTML generator during validating and parsing release note file. </h1>
         <pre>"""

        print r"<br>----------------------------------------------"
        print r"</pre><h2> Traceback: </h2><pre>"
        print e
        for line in traceback.format_exc().splitlines():
            print line
        
        print r"</pre></body><html>"

    # success
    # 
    # errors in validation arent enough to exit with different status, 
    # user will be notified about errors when he get a mail with this HTML
    # so it's one of normal script workflow
    sys.exit(0)
    
if __name__ == "__main__":
    main(sys.argv[1:])
