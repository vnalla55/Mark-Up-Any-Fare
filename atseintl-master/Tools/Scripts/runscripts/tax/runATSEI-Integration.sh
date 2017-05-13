#!/bin/bash

server=`/bin/ps -auxww | /bin/grep maker | /bin/grep tseserver | /bin/egrep -v "grep|gdb"`

OLDDIR=`pwd`
APPNAME="tax"
OLDCOREDIR="/opt/atseintl/cores"
COREDIR="/opt/atseintl/$APPNAME-cores"
DISKUSAGE_LIMIT_KBYTES="1000000"

##################################################################
# getNumFiles
# - Returns number of files specified by
#   the search string parameter passed in
# - Utilizes these global variables for data return
#   - NUM_FILES_RET for number of files found
#   - NUM_FILES_LS_RESULTS for results of the ls search command
##################################################################
NUM_FILES_RET=0
NUM_FILES_LS_RESULTS=
getNumFiles()
{
  if [ -z "$1" ]
  then
    return
  fi
  
  SEARCH_STRING=$1
  NUM_FILES_LS_RESULTS=`ls -1 --sort=time -r $SEARCH_STRING 2> /dev/null`
  LS_RESULTS_LEN=${#NUM_FILES_LS_RESULTS}
  
  if [ "$LS_RESULTS_LEN" -eq 0 ]
  then
    NUM_FILES_RET=0
    NUM_FILES_LS_RESULTS=
    return
  fi
  
  NUM_FILES_RET=`echo $LS_RESULTS_LEN | wc -l | awk '{print $1}'`
  
  #echo -e "getNumFiles found $NUM_FILES_RET with search string $NUM_FILES_LS_RESULTS"
  return
}

##################################################################
# getDiskUsage
# - Returns the amount of disk space used by
#  the files returned via the search string passed in
# - Utilizes these global variables for data return
#   - DISK_USAGE_FILES for amount of disk space used
#   - DISK_USAGE_LS_REUSLTS for results of search
##################################################################
DISK_USAGE_FILES=0
DISK_USAGE_LS_RESULTS=
getDiskUsage()
{
  if [ -z "$1" ]
  then
    return
  fi  
  
  SEARCH_STRING=$1
  DISK_USAGE_LS_RESULTS=`du -k -c $SEARCH_STRING 2> /dev/null`

  if [ "${#DISK_USAGE_LS_RESULTS}" -eq 0 ]
  then
    DISK_USAGE_FILES=0
    DISK_USAGE_LS_RESULTS=
    return
  fi
  
  DU_KEYWORD="total"
  PREVVAL=
  for h in $DISK_USAGE_LS_RESULTS
  do
    USETYPE="$h"
    if [ "$USETYPE" = "$DU_KEYWORD" ]
    then
      DISK_USAGE_FILES="$PREVVAL"
      break
    fi
    PREVVAL=$h
  done  
  
  #echo -e "getDiskUsage found that $DISK_USAGE_LS_RESULTS are using up $DISK_USAGE_FILES kilobyte(s) of disk space"
  return
}

##################################################################
# moveFiles
# - Moves the files specified by the the initial parameter passed
#   in to the directory specified by the second parameter
#
##################################################################
moveFiles()
{
  if [ -z "$1" ]
  then
    return
  fi
  
  if [ -z "$2" ]
  then
    return
  fi
  
  if [ "${#1}" -eq 0 ]
  then
    return
  fi
  
  FILELIST=$1
  MOVETODIR=$2
  for m in $FILELIST
  do
    mv $m $MOVETODIR &> /dev/null
    #echo -e "OPCOMMENT--moveFiles would have moved $m to $MOVETODIR"
  done
  return
}

##################################################################
# - Deletes the files in the file list specified by the first
#   parameter
##################################################################
deleteNumberFiles()
{
  if [ -z "$1" ]
  then
    return
  fi
  
  if [ -z "$2" ]
  then
    return
  fi

  if ([ "${#1}" -eq 0 ] || [ "${#2}" -eq 0 ])
  then
    return
  fi
  
  FILELIST=$1
  FILECOUNT=$2
  fCnt=0
  for x in $FILELIST
  do
    rm -f $x &> /dev/null
    #echo -e "OPCOMMENT--deleteNumberFiles would have deleted $x"
    fCnt=$(($fCnt+1))
    if [ "$fCnt" -gt "$FILECOUNT" ]
    then
      break
    fi
  done
  return
}  

#ensure that this dir exists
mkdir -p $COREDIR &> /dev/null

#ensure that no files are left in the old core directory
getNumFiles "/opt/atseintl/cores/*$APPNAME*"
#echo -e "Num old cores = $NUM_FILES_RET"

if [ "$NUM_FILES_RET" -gt 0 ]
then
  moveFiles $NUM_FILES_LS_RESULTS $COREDIR
fi

#ensure that no files are left in the newer, yet old core directory
getNumFiles "/opt/atseintl/core-storage/*$APPNAME*"
#echo -e "Num old cores in storage = $NUM_FILES_RET"

if [ "$NUM_FILES_RET" -gt 0 ]
then
  moveFiles $NUM_FILES_LS_RESULTS $COREDIR
fi

#Ensure that the current storage locations are not eating up
#to much disk space
getDiskUsage "$COREDIR/*$APPNAME*"
getNumFiles "$COREDIR/*$APPNAME*"
#echo -e "Number core files stored = $NUM_FILES_RET"
#echo -e "Space utilized by files  = $DISK_USAGE_FILES KBytes"

if ([ "$NUM_FILES_RET" -gt 0 ] && [ "$DISK_USAGE_FILES" -gt "$DISKUSAGE_LIMIT_KBYTES" ])
then
  deleteNumberFiles "$NUM_FILES_LS_RESULTS" "$(($NUM_FILES_RET/2))"
fi
    
#Change to core directory
cd $COREDIR

#Ensure there are no core files left ungzip'd 
getNumFiles "core.*.$APPNAME"
#echo -e "Num core files unzipped = $NUM_FILES_RET"

if [ "$NUM_FILES_RET" -gt 0 ]
then
  for j in $NUM_FILES_LS_RESULTS
  do
    gzip $j &> /dev/null &
  done
  wait
fi

#Go to proper directory
cd /opt/atseintl/$APPNAME

# move cores, if they exist, to the cores directory
getNumFiles "core.*"
#echo -e "Number core files existing in $APPNAME directory = $NUM_FILES_RET"

if [ "$NUM_FILES_RET" -gt 0 ]
then
  for i in $NUM_FILES_LS_RESULTS
  do
    COREFILENAME=${i}.${HOSTNAME}.${APPNAME}
    COREEXEFILENAME=${i}.${HOSTNAME}.tseserver_exe.${APPNAME}
    CORELIBFILENAME=${i}.${HOSTNAME}.tseserver_lib.${APPNAME}.tar.gz
    tar czvf $COREDIR/$CORELIBFILENAME /opt/atseintl/lib/* &    
    /bin/cp /opt/atseintl/bin/tseserver $COREDIR/$COREEXEFILENAME
    /bin/mv $i $COREDIR/$COREFILENAME
    chmod 664 $COREDIR/$COREFILENAME
    chmod 664 $COREDIR/$COREEXEFILENAME
    gzip $COREDIR/$COREFILENAME &
    gzip $COREDIR/$COREEXEFILENAME &
    #echo -e "OPCOMMENT--Would have created $COREFILENAME, $COREEXEFILENAME, and $CORELIBFILENAME in $COREDIR"
  done
  wait
fi

#Change back to original directory
cd $OLDDIR


# import the profile
. /etc/profile
# set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/opt/atseintl/lib:/opt/FFC/lib:/opt/sfc/lib:/opt/ACE_wrappers/lib:/opt/boost/lib:/opt/xercesc-2.4/lib:/opt/log4cxx/lib
# setup our environment for file permissions and core dump size
umask 002
ulimit -c unlimited

cd /opt/atseintl/$APPNAME
# perform log maintenance 
/bin/cat /dev/null > SAXParser.log
/bin/cat tserequest.log > Transactions.log
/usr/bin/tail -1 Transactions.log | /bin/mail -s "Tax Service XML for core" joe.yglesias@sabre.com
/usr/bin/tail -1 Transactions.log | /bin/mail -s "Tax Service XML for core" Jakub.Kubica@sabre.com
/bin/cat /dev/null > tserequest.log
/bin/cat /dev/null > tseserver.log
# run the tseserver.
exec /opt/atseintl/bin/tseserver -c /opt/atseintl/tax/tseserver.cfg > /dev/null 2>&1 

