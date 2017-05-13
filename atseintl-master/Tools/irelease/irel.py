#! /usr/bin/env python2.5
import os
import sys
import re
import getopt
import shutil
import traceback

from release_html          import HtmlOutputProcessor
from release_config        import ConfigBuilder
from release_parser        import ReleaseNoteFileParser
from release_validator     import ReleaseNoteValidator, HTMLDecoratingValidationHandler

def usage():
    print """
====================================================
===       Release Notes HTML Generator           ===
====================================================

Usage:
    $ python irel.py -f Release/release_2012_09

Options:
    -f, --file  - input release file
    -c, --conf  - configuration file (default is ./irel.xml)
    -h, --help  - shows this usage message

"""

def main(argv):
    #command line check and parse
    filename = "";
    
    if sys.argv[0] == "irel.py":
        scriptpath = os.getcwd()
    else:
        scriptpath = sys.argv[0].replace(os.sep + 'irel.py','') 

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

    if filename == "":
        print "Wrong parameter (filename). Try -h for help!";
        sys.exit(2)
    
    try:    
        configuration = ConfigBuilder(configfile).build()

        with open(filename) as f:
            content = ReleaseNoteFileParser().parseFile(f)
        
        handler = HTMLDecoratingValidationHandler()
        validator = ReleaseNoteValidator(handler)
        validator.validate(content, configuration)

        validatedContent = handler.getResult()
        errors = handler.getErrors()

        htmlContent = HtmlOutputProcessor().decorate(validatedContent, configuration, errors)
        sys.stdout.write(htmlContent)

    except Exception as e:
        
        print r"""<html><body>
            <h1> An error occured in HTML generator during validating and parsing release note file. </h1>
            <h2> Hereunder there are raw release notes: </h2>
         <pre>"""

        try:
            file = open(filename)
            for line in file.xreadlines():
                print line,
        except IOError:
            print "Error Reading File"


        print r"<br>----------------------------------------------"
        print r"</pre><h2> Traceback: </h2><pre>"
        print e
        for line in traceback.format_exc().splitlines():
            print line
        
        print r"</pre></body><html>"

    #success
    sys.exit(0)

if __name__ == "__main__":
    main(sys.argv[1:])
