#!/bin/bash
createLog4CXX()
{
echo -e "<?xml version=\"1.0\" encoding=\"UTF-8\" ?> 
<!DOCTYPE log4j:configuration SYSTEM \"log4j.dtd\">

<log4j:configuration xmlns:log4j='http://jakarta.apache.org/log4j/' >

    <appender name=\"ASYNC_STDOUT\" class=\"org.apache.log4j.AsyncAppender\">
        <appender-ref ref=\"STDOUT\"/>
    </appender>

    <appender name=\"ASYNC_TSE_SERVER\" class=\"org.apache.log4j.AsyncAppender\">
        <appender-ref ref=\"TSE_SERVER\"/>
    </appender>

    <appender name=\"ASYNC_METRICS\" class=\"org.apache.log4j.AsyncAppender\">
        <appender-ref ref=\"METRICS_LOGGER\"/>
    </appender>

    <appender name=\"ASYNC_REQUEST\" class=\"org.apache.log4j.AsyncAppender\">
        <appender-ref ref=\"REQUEST_LOGGER\"/>
    </appender>

    <appender name=\"ASYNC_SAX_PARSER\" class=\"org.apache.log4j.AsyncAppender\">
        <appender-ref ref=\"SAX_PARSER\"/>
    </appender>

    <appender name=\"STDOUT\" class=\"org.apache.log4j.ConsoleAppender\">
        <layout class=\"org.apache.log4j.PatternLayout\">
            <param name=\"TimeZone\" value=\"US/Central\"/>
            <param name=\"ConversionPattern\" value=\"%d: %t %-5p %c{2} - %m%n\"/>
        </layout>
    </appender>

    <appender name=\"TSE_SERVER\" class=\"org.apache.log4j.RollingFileAppender\">
        <param name=\"File\"   value=\"tseserver.log\" />
        <param name=\"Append\" value=\"false\" />
        <!-- Create 5 backup logfiles. -->
        <param name=\"MaxBackupIndex\" value=\"5\" />
        <param name=\"MaxFileSize\" value=\"50MB\" />
        <layout class=\"org.apache.log4j.PatternLayout\">
            <param name=\"TimeZone\" value=\"US/Central\"/>
            <param name=\"ConversionPattern\" value=\"%d: %t %-5p %c{2} - %m%n\"/>
        </layout>
    </appender>

    <appender name=\"METRICS_LOGGER\" class=\"org.apache.log4j.RollingFileAppender\">
        <param name=\"File\"   value=\"tsemetrics.log\" />
        <param name=\"Append\" value=\"false\" />
        <!-- Create 5 backup logfiles. -->
        <param name=\"MaxBackupIndex\" value=\"5\" />
        <param name=\"MaxFileSize\" value=\"50MB\" />
        <layout class=\"org.apache.log4j.PatternLayout\">
            <param name=\"TimeZone\" value=\"US/Central\"/>
            <param name=\"ConversionPattern\" value=\"%d: %t %-5p %c{2} - %m%n\"/>
        </layout>
    </appender>

    <appender name=\"REQUEST_LOGGER\" class=\"org.apache.log4j.RollingFileAppender\">
        <param name=\"File\"   value=\"tserequest.log\" />
        <param name=\"Append\" value=\"false\" />
        <!-- Create 5 backup logfiles. -->
        <param name=\"MaxBackupIndex\" value=\"5\" />
        <param name=\"MaxFileSize\" value=\"50MB\" />
        <layout class=\"org.apache.log4j.PatternLayout\">
            <param name=\"TimeZone\" value=\"US/Central\"/>
            <param name=\"ConversionPattern\" value=\"%d: %t %-5p %c{2} - %m%n\"/>
        </layout>
    </appender>

    <appender name=\"SAX_PARSER\" class=\"org.apache.log4j.RollingFileAppender\">
        <param name=\"File\"   value=\"SAXParser.log\" />
        <param name=\"Append\" value=\"false\" />
        <!-- Create 5 backup logfiles. -->
        <param name=\"MaxBackupIndex\" value=\"5\" />
        <param name=\"MaxFileSize\" value=\"50MB\" />
        <layout class=\"org.apache.log4j.PatternLayout\">
            <param name=\"TimeZone\" value=\"US/Central\"/>
            <param name=\"ConversionPattern\" value=\"%d: %t %-5p %c{2} - %m%n\"/>
        </layout>
    </appender>

    <category name=\"atseintl.Server\">
        <priority value=\"info\" />
    </category>

    <category name=\"atseintl.Service.TransactionOrchestrator\">
        <priority value=\"info\" />
    </category>
<!--
    <category name=\"atseintl.Rules.RuleUtil.TSI\">
        <priority value=\"error\" />
    </category>
-->
<!--    <logger name=\"atseintl.Metrics\" additivity=\"false\">
        <level value=\"info\" />
        <appender-ref ref=\"ASYNC_METRICS\" />
    </logger> 
    <logger name=\"atseintl.Request\" additivity=\"false\">
        <level value=\"info\" />
        <appender-ref ref=\"ASYNC_REQUEST\" />
    </logger>
    <logger name=\"atseintl.Xform.XformClientXML\" additivity=\"false\">
        <level value=\"info\" />
        <appender-ref ref=\"ASYNC_SAX_PARSER\" />
    </logger>
-->
    <logger name=\"atseintl.Request\" additivity=\"false\">
      <level value=\"info\" />
      <appender-ref ref=\"ASYNC_REQUEST\" />
    </logger>
    
    <logger name=\"atseintl.Xform.XformClientShoppingXML\" additivity=\"false\">
      <level value=\"info\" />
      <appender-ref ref=\"ASYNC_SAX_PARSER\" />
    </logger>

    <root>
        <priority value =\"error\" />
<!--        <appender-ref ref=\"ASYNC_STDOUT\" /> -->
       <appender-ref ref=\"TSE_SERVER\" /> 
    </root>

</log4j:configuration> 
" > log4cxx.xml

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
##################################################################


#Ensure that this new core directory exists
mkdir -p $COREDIR &> /dev/null

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

#Ensure that the current storage locations are not eating up
#to much disk space
getDiskUsage "$COREDIR/*$APPNAME*"
getNumFiles "$COREDIR/*$APPNAME*"

if ([ "$NUM_FILES_RET" -gt 0 ] && [ "$DISK_USAGE_FILES" -gt "$DISKUSAGE_LIMIT_KBYTES" ])
then
  deleteNumberFiles "$NUM_FILES_LS_RESULTS" "$(($NUM_FILES_RET/2))"
fi
    
#Change to core directory
cd $COREDIR

#Ensure there are no core files left ungzip'd 
getNumFiles "core.*.$APPNAME"

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
export LD_LIBRARY_PATH=/opt/atseintl/lib:/opt/FFC/lib:/opt/sfc/lib:/opt/eo/lib:/opt/ACE_wrappers/lib:/opt/boost/lib:/opt/xercesc-2.4/lib:/opt/log4cxx/lib
# setup our environment for file permissions and core dump size
umask 002
ulimit -c unlimited

#Change into the app specific directory 
cd /opt/atseintl/$APPNAME

# perform log maintenance 
outputFileAfterLastStringOccurrence 'atseintl.Request' 'tserequest.log'
LASTREQUEST="<ShoppingRequest $AWKTOPLINE $SEARCHRESULTS"

# Email data to users on email list
EMAIL_LIST=`cat email.lst`

if (! [ -e $NOEMAILFILE ])
then
  TRANS_LOG="Transactions:\n `cat Transactions.log` \nLast Request:\n $LASTREQUEST\n Last 100 Lines Of tseserver Log:\n `tail --lines=100 tseserver.log`\n TseServer Error Log:\n `cat /tmp/tseserver-error.log`\n"
  for i in $EMAIL_LIST
  do
    echo -e "$TRANS_LOG" | /bin/mail -s "$APPNAME has cored - XML request" "$i" &
  done
  wait
fi

#Ensure that log4cxx.xml for shopping remains what it was
createLog4CXX

# run the tseserver.
exec /opt/atseintl/bin/tseserver -c /opt/atseintl/$APPNAME/tseserver.cfg > /dev/null 2> /tmp/tseserver-error.log 
