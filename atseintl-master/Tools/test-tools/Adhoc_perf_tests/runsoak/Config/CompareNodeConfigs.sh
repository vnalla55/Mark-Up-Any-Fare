#!/bin/sh

NAME=$0

function usage
{
   echo -e >&2 "\n** Error: Missing/Incorrect parameters **\n"
   echo >&2 "Usage: $NAME application hostname1:port1 hostname2:port2"
   echo -e >&2 "\nApplication must be one of pricing, faredisplay, tax or shopping"
   echo >&2 "Use 'pricing' for pricing & historical pricing servers"
   echo >&2 "Use 'faredisplay' for faredisplay servers"
   echo -e >&2 "Use 'shopping' for shopping is, mip, esv and historical shopping\n"
   echo -e >&2 "Examples:"
   echo >&2 "$NAME pricing piili001:53701 piili002:53701"
   echo >&2 "$NAME shopping pimli006:53601 pimli007:53601"
   echo >&2 "$NAME faredisplay piili001:53901 piili002:53901"
   echo >&2 "$NAME tax piili001:53501 piili002:53501"
   echo -e >&2 "\n**** IMPORTANT: DO NOT USE MISMATCHED APPLICATION NAME. IT MAY KILL THE SERVER!!! ****\n"

   exit 1
}

# *** MAIN ***

if [[ $# -ne 3 ]]; then
   usage
fi

APPLICATION=$1
HOST_PORT1=$2
HOST_PORT2=$3

if [[ "$APPLICATION" != "shopping" && "$APPLICATION" != "tax" && "$APPLICATION" != "pricing" && "$APPLICATION" != "faredisplay" ]] ; then
   usage
fi

GETCONFIG=`dirname $0`/GetNodeConfig.sh
if [ ! -s $GETCONFIG ]; then
    echo "$GETCONFIG not found. Set a view to continue."
    exit 1
fi

TMPDIR=/tmp/gc$$
IOR_FILE1=$TMPDIR/ior1
IOR_FILE2=$TMPDIR/ior2
OUT1=$TMPDIR/out1
OUT2=$TMPDIR/out2
DIFF=$TMPDIR/diff.txt
mkdir $TMPDIR

echo $HOST_PORT1 > $IOR_FILE1
echo $HOST_PORT2 > $IOR_FILE2

$GETCONFIG $APPLICATION $HOST_PORT1 > $OUT1
if [ $? -ne 0 ]; then
	echo "ERROR! Could not get configuration from $HOST_PORT1"
	rm -rf $TMPDIR
	exit 1
fi

$GETCONFIG $APPLICATION $HOST_PORT2 > $OUT2
if [ $? -ne 0 ]; then
    echo "ERROR! Could not get configuration from $HOST_PORT2"
    rm -rf $TMPDIR
    exit 1
fi

sed -i '1,4d' $OUT1
sed -i '1,4d' $OUT2

/usr/bin/diff $OUT1 $OUT2 > $DIFF
if [ -s $DIFF ]; then
    echo -e "\n*** Differences detected between $HOST_PORT1 & $HOST_PORT2 ***"
	cat $DIFF
else
    echo -e "\nNo differences detected between $HOST_PORT1 & $HOST_PORT2."
fi

rm -rf $TMPDIR
