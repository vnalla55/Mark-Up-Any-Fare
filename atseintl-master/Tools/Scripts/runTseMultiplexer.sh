#!/bin/sh

USAGE="Usage: `basename $0` < Server | Client >"

# Temporarily add the current directory, VisiBroker executables directory, and
# the FFC executables directory to the PATH.

PATH=.:$FFCDIR/bin:$PATH
LD_LIBRARY_PATH=$FFCDIR/lib:/vobs/atseintl/lib/debug:/vobs/atseintl/Tools/sfc/lib:$LD_LIBRARY_PATH
MYHOST=`hostname`.`domainname`

# Check which mode is to be tested: Client or Server mode.

if [ "$1" = "Server" ] ; then
   RUNCMD="TseMultiplexerServer -service TseMultiplexerService -FFCConfig=TseMultiplexer.properties -ORBInitRef NameService=corbaloc:iiop:directorycrt.sabre.com:27000/NameService -ORBEndpoint iiop://${MYHOST}:27099"
elif [ "$1" = "Client" ] ; then
   RUNCMD="TseMultiplexerClient -service TseMultiplexerService -FFCConfig=TseMultiplexer.properties -ORBInitRef NameService=corbaloc:iiop:directorycrt.sabre.com:27000/NameService"
else
   echo "$USAGE"
   exit 1
fi

# Check that the properties used to direct the test program are available
# in a file called testFFCDir.properties in the current directory.

if [ ! -r TseMultiplexer.properties ] ; then
   echo "Properties file 'TseMultiplexer.properties' not found!"
   exit 1
fi

# Check that the properties used by the VisiBroker ORB are available
# in a file called orb.properties in the current directory.

if [ ! -r svc.conf ] ; then
   echo "ORB properties file 'svc.conf' not found!"
   exit 1
fi

# Check that the FFC shared library is available in the FFC executables
# directory.

if [ ! -r $FFCDIR/bin/libffc.so ] ; then
   if [ ! -r $FFCDIR/bin/libffc.sl ] ; then
      echo "FFCDIR=$FFCDIR"
      echo "   environment variable FFCDIR must be set to the correct directory."
      exit 1
   fi
fi

# Run the test program.

echo $RUNCMD
$RUNCMD

exit 0
