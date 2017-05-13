#!/bin/bash

myname=`basename $0`
RELEASE=

if [ z"$BUILD" = z ]; then
  RELEASE=debug
else
  RELEASE=$BUILD
fi

IOR=${HOME}/tseserver.ior

profiler=
valgrindCmd=/opt/valgrind/bin/valgrind
tseServerCmd=./tseserver
tseServerArgs=

function usage()
{
  cat <<EOF

Usage: $myname [-v] [-o prof-options] [-h] -- [tseserver passthrough opts]

  -v   Run server under valgrind simulator.
  -o   Use prof-options as command line options for either Purify, Quantify,
       or Valgrind. It is recommended to enclose prof-options in quotes to
       ensure correct grouping of arguments.
        (NOTE: This option must be specified for Valgrind in order to specify
         the Valgrind skin to use.)
  -h   Display this help message.

Ex:
  $myname -v -o "--skin=memcheck --logfile=valgrind.log" -- -i abc123

EOF
}

# Process our options
while getopts ":vo:h" flag
do
  case $flag in
    v)
      # Valgrind
      profiler=valgrind
      OPTIND=1
      shift
      ;;
    o)
      args=$OPTARG
      shift; shift
      OPTIND=1
      ;;
    h)
      usage
      exit 0
      ;;
    ?)
      echo "Invalid option: $OPTARG"
      usage
      exit 1
      ;;
  esac
done

#echo "Profiler: $profiler"
#echo "Args: $args"

if [ "$profiler" = "" ]; then
  echo "Error: The -p, -q, or -v option must be specified."
  usage
  exit 1
fi

if [ "$1" = "--" ]; then
  shift
fi

# Intercept tseserver args so we can set the IOR file
OPTIND=1
while getopts ":i:" flag
do
  case $flag in
    i)
      # IOR option
      LINE=`echo $OPTARG | tr [:lower:] [:upper:]`
      IOR=${HOME}/tseserver$LINE.ior
      rm -f $IOR
      shift; shift
      ;;
  esac
done

echo 'Setting release to '$RELEASE
echo 'Setting iorfile to '$IOR
tseServerArgs="-D TO_MAN.IOR_FILE=$IOR $*"

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/lib/$RELEASE
LD_LIBRARY_PATH=/opt/log4cxx/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/asap/lib:/opt/asap-wrapper/lib

case $profiler in
  valgrind)
    cmd="$valgrindCmd $args $tseServerCmd $tseServerArgs"
    echo $cmd
    exec $cmd
    ;;
esac

