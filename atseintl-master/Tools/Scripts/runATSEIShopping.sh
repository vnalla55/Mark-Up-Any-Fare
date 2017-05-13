#!/bin/bash
createLog4CXX()
{

cp /home/maker/shopping-log4cxx.xml log4cxx.xml

}  

server=`/bin/ps -auxww | /bin/grep maker | /bin/grep tseserver | /bin/egrep -v "grep|gdb"`
#echo SERVER: "$server"

#Save current directory
OLDDIR=`pwd`
APPNAME="shopping"
OLDCOREDIR="/opt/atseintl/cores"
COREDIR="/opt/atseintl/$APPNAME-cores"
DISKUSAGE_LIMIT_KBYTES="1000000"
NOEMAILFILE="/opt/atseintl/$APPNAME/noEmailFromScript"
LASTEMAILTIME="/opt/atseintl/$APPNAME/lastEmailTime"
EMAILTIMEWAITSECONDS=20
APPSTACKTRACEFILE="/tmp/$APPNAME-stacktrace.txt"
SCPTARGETHOST="atseibld2"
SCPTARGETDIR="/opt/atseintl/Shopping/hammer/crash"

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
  done
  return
}

##################################################################
# - Deletes the files in the file list specified by the first
#   parameter
#   The second parameter determines how many files should
#   be deleted
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
    fCnt=$(($fCnt+1))
    if [ "$fCnt" -gt "$FILECOUNT" ]
    then
      break
    fi
  done
  return
}  

##################################################################
# Outputs the last lines of a file after a string occurrence
# has been found
# SEARCHRESULTS contains the output of the file
# AWKTOPLINE is a specific return line search for 
# an ATSEI shopping request, ignore for general usage
##################################################################
SEARCHRESULTS=
AWKTOPLINE=
outputFileAfterLastStringOccurrence()
{
  #Check if the search string is valid
  if [ -z $1 ]
  then
    return
  fi
  
  #Check if the file name is valid
  if [ -z $2 ]
  then
    return
  fi

  OUTAFTERSTRING=$1
  SEARCHINFILE=$2
  
  TOTALLINESINFILE=`awk '{x++} END {print x}' $SEARCHINFILE 2> /dev/null`  
  if [ "$TOTALLINESINFILE" -eq 0 ]
  then
    return
  fi
  
  LASTLINEOFSEARCH=`grep -n $OUTAFTERSTRING $SEARCHINFILE | awk -F: '{print $1}' | tail --lines=1`
  if [ "${#LASTLINEOFSEARCH}" -eq 0 ]
  then
    return
  fi
    
  NUMLINESLASTSEARCH=$(($TOTALLINESINFILE-$LASTLINEOFSEARCH+1))
  NUMLINESLASTSEARCHMINUS1=$(($NUMLINESLASTSEARCH-1))
  
  if [ "$NUMLINESLASTSEARCH" -le 0 ]
  then
    return
  fi
  
  MODRESULTS=`tail --lines=$NUMLINESLASTSEARCH $SEARCHINFILE 2> /dev/null`
  
  if [ "${#MODRESULTS}" -eq 0 ]
  then
    return
  fi
  
  AWKTOPLINE=`echo -e $MODRESULTS | awk '{print $12, $13}'`
  SEARCHRESULTS=`tail --lines=$NUMLINESLASTSEARCHMINUS1 $SEARCHINFILE`

  return
}  
##################################################################
CURFDATE=
getCurDateInSecondsFromEpoch()
{
  CURFDATE=`date +%s 2> /dev/null`
}
##################################################################
scpFileToHost()
{
  SCPHOST=$1
  SCPDESTDIR=$2
  SCPSRCFILE=$3

  if ([ -z $SCPHOST ] || [ "${#SCPHOST}" -eq 0 ])
  then
    return
  fi
  
  if ([ -z $SCPDESTDIR ] || [ "${#SCPDESTDIR}" -eq 0 ])
  then
    return
  fi
  
  if ([ -z $SCPSRCFILE ] || [ "${#SCPSRCFILE}" -eq 0 ] || (! [ -e $SCPSRCFILE ]))
  then
    return
  fi
  
  RETCODE=`scp -Bq $SCPSRCFILE $SCPHOST:$SCPDESTDIR/$SCPSRCFILE`  
}

##################################################################
sendOutEmailAfterCore()
{
  #Get last request
  outputFileAfterLastStringOccurrence 'atseintl.Request' 'tserequest.log'
  LASTREQUEST="<ShoppingRequest $AWKTOPLINE $SEARCHRESULTS"

  # Email data to users on email list
  EMAIL_LIST=`cat email.lst`

  # If emailing is disabled, return immediately
  if [ -e $NOEMAILFILE ]
  then
    return
  fi
  
  #If there was no previous last email time file, we need to 
  #send an email
  if [ -e $LASTEMAILTIME ]
  then
    STACKTRACEOUTPUT=`cat $APPSTACKTRACEFILE 2> /dev/null`
    if [ "${#STACKTRACEOUTPUT}" -eq 0 ]
    then
      return
    fi
  
    getCurDateInSecondsFromEpoch
    CURODATE=`cat $LASTEMAILTIME 2> /dev/null`
    DIFFINTIME=$(($CURFDATE-$CURODATE))

    #If it has been less than 20 seconds
    #since the last email, do not send
    #another one out
    if [ "$DIFFINTIME" -le 20 ]
    then
      return
    fi
    
    #Create xml output name
    OUTPUTXMLFILENAME="shopping_crash_$CURFDATE.xml"
    
    #Generate output xml for scp transfer
    echo -e "$LASTREQUEST" &> $OUTPUTXMLFILENAME
    
    #Secure copy the file
    scpFileToHost $SCPTARGETHOST $SCPTARGETDIR $OUTPUTXMLFILENAME
    
    #Create the email contents
    TRANS_LOG="Last Request File Name: $SCPTARGETDIR/$OUTPUTXMLFILENAME\n Last 100 Lines Of tseserver Log:\n `tail --lines=100 tseserver.log`\n TseServer Stacktrace:\n $STACKTRACEOUTPUT\n"
    
    #Email to all recipients on the email list
    for i in $EMAIL_LIST
    do
      echo -e "$TRANS_LOG" | /bin/mail -s "$APPNAME has cored - XML request" "$i" &
    done
  fi
  
  #Output the last email time file
  getCurDateInSecondsFromEpoch
  echo $CURFDATE &> $LASTEMAILTIME  
}
##################################################################
cleanUpOldFiles()
{
  #Ensure that no files are left in the old core directory
  getNumFiles "/opt/atseintl/cores/*$APPNAME*"

  if [ "$NUM_FILES_RET" -gt 0 ]
  then
    moveFiles $NUM_FILES_LS_RESULTS $COREDIR
  fi

  #Ensure that no files are left in the newer, yet old core directory
  getNumFiles "/opt/atseintl/core-storage/*$APPNAME*"

  if [ "$NUM_FILES_RET" -gt 0 ]
  then
    moveFiles $NUM_FILES_LS_RESULTS $COREDIR
  fi  
}
##################################################################
reclaimDiskSpaceInCoreStorage()
{
  #Ensure there are no core files left ungzip'd 
  getNumFiles "$COREDIR/core.*.$APPNAME"

  if [ "$NUM_FILES_RET" -gt 0 ]
  then
    for j in $NUM_FILES_LS_RESULTS
    do
      gzip $j &> /dev/null &
    done
  fi
  
  #Ensure that the current storage locations are not eating up
  #to much disk space
  getDiskUsage "$COREDIR/*$APPNAME*"
  getNumFiles "$COREDIR/*$APPNAME*"

  if ([ "$NUM_FILES_RET" -gt 0 ] && [ "$DISK_USAGE_FILES" -gt "$DISKUSAGE_LIMIT_KBYTES" ])
  then
    deleteNumberFiles "$NUM_FILES_LS_RESULTS" "$(($NUM_FILES_RET/2))"
  fi
}
##################################################################
archiveCurrentCores()
{
  cd /opt/atseintl/$APPNAME
  
  #Move cores, if they exist, to the cores directory
  getNumFiles "core.*"
  if [ "$NUM_FILES_RET" -gt 0 ]
  then
    for i in $NUM_FILES_LS_RESULTS
    do
      IFILENAME=${i##/*/}
      COREFILENAME=${IFILENAME}.${HOSTNAME}.${APPNAME}
      /bin/mv $IFILENAME $COREDIR/$COREFILENAME
      chmod 664 $COREDIR/$COREFILENAME
      gzip $COREDIR/$COREFILENAME &> /dev/null &
    done
  fi
  
  cd $OLDDIR
}
##################################################################
##################################################################

#Ensure that this new core directory exists
mkdir -p $COREDIR &> /dev/null

#Clean up core files from previous core directory locations
cleanUpOldFiles

#Ensure we have enough disk space for proper server operation
#reclaimDiskSpaceInCoreStorage

#Get current core files if they exist from the app specific
#directory
#archiveCurrentCores

# import the profile
. /etc/profile
# set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/opt/atseintl/lib:/opt/FFC/lib:/opt/sfc/lib:/opt/eo/lib:/opt/ACE_wrappers/lib:/opt/boost/lib:/opt/xercesc-2.4/lib:/opt/log4cxx/lib
# setup our environment for file permissions and core dump size
umask 002
ulimit -c unlimited

#Change into the app specific directory 
cd /opt/atseintl/$APPNAME

#Perform email operations
sendOutEmailAfterCore

#Ensure that log4cxx.xml for shopping remains what it was
createLog4CXX

#set stack size to 2Mb
ulimit -Ss 2048

# run the tseserver.
#echo "Run the server"
exec /opt/atseintl/bin/tseserver -c /opt/atseintl/$APPNAME/tseserver.cfg > /dev/null 2> $APPSTACKTRACEFILE
