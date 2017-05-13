#! /opt/python-2.6.1/bin/python

from release_config import ConfigBuilder
from release_generator import DefaultReleaseNoteGenerator

import getopt
import os
import sys


def usage():
    print """
====================================================
===          Release Notes Generator             ===
====================================================

Usage:
    $ python irelgen.py -l
    $ python irelgen.py -g ATSEv2
    $ python irelgen.py -g ATSEv2 -f Release/new_release_note
    $ python irelgen.py -g ATSEv2 -f Release/new_release_note -a

Options:
    -f, --file          - output filename (default is writing on standard output)
    -l, --list          - listing available groups instead of generating release note
    -g, --group         - group of generated release note
    -a, --attachfiles   - attach files from current activity to release note 
                                   (works only if group has 'files()' default)
    -c, --conf          - configuration file (default is ./irel.xml)
    -h, --help          - shows this usage message

"""

def main(argv):
    filename = "";
    group = ""
    
    if sys.argv[0] == "irelgen.py":
        scriptpath = os.getcwd()
    else:
        scriptpath = sys.argv[0].replace(os.sep + 'irelgen.py','') 

    configfile = scriptpath + os.sep + "irel.xml"
    listGroups = False

    attachFilesFromActivity = False

    try:                                
        opts, args = getopt.getopt(argv, "f:g:c:hla", ["file=", "help", "conf=", "group=", "list", "attachfiles"])
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
        elif opt in ("-g", "--group"):
            group = arg
        elif opt in ("-c", "--conf"):
            configfile = arg
        elif opt in ("-l", "--list"):
            listGroups = True
        elif opt in ("-a", "--attachfiles"):
            attachFilesFromActivity = True
  
    configuration = ConfigBuilder(configfile).build()

    if listGroups or group == "":
        print "\nAvailable groups: "
        for group in sorted(configuration.groups.keys()):
            print "  - {0}".format(group)
        sys.exit(0)

    if group != "" and group not in configuration.groups.keys():
        print "Wrong group name. Run script with -l option to see group list"
        sys.exit(2)

    generator = DefaultReleaseNoteGenerator(attachFilesFromActivity)
    note = generator.generate(group, configuration.groups[group])

    if filename != "":
        f = open(os.getcwd() + os.sep + filename, 'w')
        f.write(note)
        f.close()
    else:
        sys.stdout.write(note)

    #success
    sys.exit(0)

if __name__ == "__main__":
    main(sys.argv[1:])

        
