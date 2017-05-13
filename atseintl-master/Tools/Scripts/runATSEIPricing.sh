#!/bin/sh

USAGE="Usage: `basename $0` < Server | Client >"

SVRDIR=/opt/atseintl/TseMultiplexServer
NORSFILE=${SVRDIR}/noTESTrestart

if [ ! -f ${NORSFILE} ]; then
	. /etc/profile
	# Temporarily add the current directory, VisiBroker executables directory, and
	# the FFC executables directory to the PATH.

	PATH=.:$FFCDIR/bin:$PATH
	LD_LIBRARY_PATH=/opt/atseintl/TseMultiplexServer/lib:$LD_LIBRARY_PATH
	MYHOST=`/bin/hostname`.`/bin/domainname`

	# Only allow this to run on atsela01
	if [ ! "$MYHOST" = "atsela05.dev.sabre.com" ] ; then
	   /bin/echo Cannot run on $MYHOST
	   exit 1
	fi

	#Only allow starting the server if one is not already running.
	Server=`/bin/ps -auxww | /bin/grep runATSEIPricing.sh | grep -v grep | grep -v ${MYPID} | grep -v atseintl`

	if [ ! "${Server}x" = "x" ] ; then
	  echo
	  echo !!!DON'T START THE SERVER!!! There's another already running.
	  echo
	  exit 1
	fi

	# Check which mode is to be tested: Client or Server mode.

	if [ "$1" = "Server" ] ; then
	   RUNCMD="TseMultiplexerServer -service ATSEIPricing -FFCConfig=ATSEIPricing.properties -ORBInitRef NameService=corbaloc:iiop:directorycrt.sabre.com:27000/NameService -ORBEndpoint iiop://${MYHOST}:27090"
	elif [ "$1" = "Client" ] ; then
	   RUNCMD="TseMultiplexerClient -service ATSEIPricing -FFCConfig=ATSEIPricing.properties -ORBInitRef NameService=corbaloc:iiop:directorycrt.sabre.com:27000/NameService"
	else
	   echo "$USAGE"
	   exit 1
	fi

	# Check that the properties used to direct the test program are available
	# in a file called testFFCDir.properties in the current directory.

	if [ ! -r ATSEIPricing.properties ] ; then
	   echo "Properties file 'ATSEIPricing.properties' not found!"
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
else
    sleep 60
fi

exit 0
