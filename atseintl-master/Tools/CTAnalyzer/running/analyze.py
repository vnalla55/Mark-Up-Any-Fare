import CTAnalyzer
import getopt
import sys

def usage():
    print """
    Usage:
    analyze.py [-d dir | --directory=dir] inputfile_prefix outputfile

     dir - directory where input files are located
     inputfile_prefix - prefix of input file names
     outputfile - name of output file
    """

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hd:", ["help", "directory="])
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        sys.exit(2)
    directory = './'
    for o,a in opts:
        if o in ['-h', '--help']:
            usage()
            sys.exit()
        if o in ['-d', '--directory']:
            directory = a
    if len(args) < 2:
        usage()
        sys.exit(2)

    c = CTAnalyzer.CTAnalyzer(args[0], directory)
    x = CTAnalyzer.XMLExport(c)

    x.exportTo(args[1])


if __name__ == '__main__':
    main()
