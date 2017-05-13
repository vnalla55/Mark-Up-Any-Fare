#!/bin/bash

APPNAME="shopping"
OPDIRECTORY="/opt/atseintl/$APPNAME"
NOEMAILTOUCHFILE="$OPDIRECTORY/noEmailFromScript"

makeNoTouch()
{
  touch $NOEMAILTOUCHFILE 2> /dev/null
}

removeNoTouch()
{
  rm -f $NOEMAILTOUCHFILE 2> /dev/null
}

SERVERID=0
SERVERIDLEN=0
getServerProcessId()
{
  SERVERID=`ps -ef | grep maker | grep $APPNAME | grep tseserver | awk '{print $2}'`
  SERVERIDLEN=${#SERVERID}
}  

###########################################################
## MAIN ##
###########################################################
makeNoTouch
getServerProcessId
if [ "$SERVERIDLEN" -eq 0 ]
then
  removeNoTouch
  exit
fi
kill -9 $SERVERID 2> /dev/null
./runATSEI-Integration.sh
removeNoTouch
exit
