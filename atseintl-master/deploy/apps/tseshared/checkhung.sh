#!/bin/bash
#-----------------------------------------------------------------------------
#
#         tseserver checkhung.sh script
#         Date: March 2010
#         John Watilo
#
#-----------------------------------------------------------------------------

#
# set to '1' for testing & debugging purposes
# otherwise '0'
#
declare -i VERBOSE=0

#
# ensure we are running within the app directory
#
cd $(dirname $(which ${0}))

#
# source in the common functions - this gives us the 'say' function as
# well as access to other CSF variables if we need them later
#
. ./set.atse.common.sh
. ${ATSE_COMMON}/admin.sh

. ./cfgshell_functions.sh

CFGSHELL_VERBOSE=${VERBOSE}

#
# function to see if we are hung up in startup
#
function atsev2_hung_in_startup
{
  #
  # arguments
  #
  local -i verbose=${1}
  local    pid=${2}
  local    hung_time_threshold=${3}

  #
  # other local variables
  #
  local    logfile="tseserver.log"
  local    line=""
  local    datetime_at_startup""
  local -i inside_startup=0
  local -i is_hung=0

  #
  # perform the check by looking for a line like this:
  #
  #   2010-03-09 12:39:10,757: 0x2b5c218040f0 INFO  Server.TseServer - Config initialized
  #
  # and achieving 'closure' by matching it with subsequent lines (either/or) like these:
  #
  #   2010-03-09 12:46:42,187: 0x2b1e851640f0 INFO  Server.main - ** TseServer is running **
  #   2010-03-09 12:46:47,200: 0x2b1e851640f0 INFO  Server.main - ** TseServer is terminating **
  #
  # if we are left with no closure, then we know the server is still in startup phase, so
  # we check current time against the date of the opening line to see if we've exceeded
  # out configured threshold
  #
  if [ -f ${logfile} ] ; then

    set -o noglob

    #
    # make newline our delimiter so our loop will read a line at a time
    # instead of a word at a time
    #
    local saveIFS=${IFS}
    IFS='
'
    for line in $(tail -200 ${logfile}) ; do

      if [ -n "$(echo ${line} | grep ' Config initialized')" ] ; then

        inside_startup=1
        datetime_at_startup=$(echo ${line} | awk -F',' '{print $1}')

        if [ ${verbose} -eq 1 ] ; then
          say "Found startup line - timestamp = ${datetime_at_startup}"
        fi

      elif [ -n "$(echo ${line} | grep ' ** TseServer is ')" ] ; then

        inside_startup=0

        if [ ${verbose} -eq 1 ] ; then
          say "Found closure"
        fi

      fi

    done

    #
    # restore IFS to its prior value
    #
    IFS=${saveIFS}

    #
    # now check our results and see if we're stuck
    #
    if [ ${inside_startup} -eq 1 ] ; then
      local secs_at_startup=$(date +%s --date="""${datetime_at_startup}""")
      local secs_now=$(date +%s)
      local diff=$(( ${secs_now} - ${secs_at_startup} ))

      if [ ${verbose} -eq 1 ] ; then
        say "Startup was ${diff} seconds ago"
      fi

      if [ ${diff} -gt ${hung_time_threshold} ] ; then
        is_hung=1
        /usr/bin/logger -p local0.alert -t CVGALERT "ATSEv2 process [${pid}] is hung during startup!  Restart imminent..."
        if [ ${verbose} -eq 1 ] ; then
          say "Startup time exceeds threshold of ${hung_time_threshold} seconds!"
        fi
      fi

    fi

    set +o noglob

  else

    say "ERROR: Unable to open ${logfile}"

  fi

  if [ ${verbose} -eq 1 ] ; then
    if [ ${is_hung} -eq 1 ] ; then
      say "Server is hung!"
    else
      say "Server is NOT hung."
    fi
  fi

  return ${is_hung}
}

#
# function to see if the cache udpate thread is hung
#
function atsev2_cacheupdate_hung
{
  #
  # arguments
  #
  local -i verbose=${1}
  local pid=${2}

  #
  # other local variables
  #
  local -i is_hung=0
  local -i thread_grace=600  # 10 minutes
  local -i poll_interval=60  # 1 minute
  local -i last_mod_seconds=0
  local -i seconds_now=0
  local -i delta=0
  local -i grace_seconds=0
  local    thread_alive_file="cache.thread.alive"
  local    thread_dead_restart=N

  #
  # get the command line arguments used to start the server
  #
  local runctl=$(ls -rt1 .*.*.1.runctl 2> /dev/null | tail -1)
  if [ ${verbose} -eq 1 ] ; then
    say "runctl = ${runctl}"
  fi
  if [ -z "${runctl}" ] ; then
    return ${is_hung}
  fi
  APPMON_COMMAND=""
  . ${runctl}
  if [ ${verbose} -eq 1 ] ; then
    say "APPMON_COMMAND = ${APPMON_COMMAND}"
  fi

  cfgshell_flatten_from_tseserver_args "${APPMON_COMMAND}"

  #
  # get the thread dead restart flag
  #
  thread_dead_restart=$(cfgshell_lookup CACHE_ADP THREAD_DEAD_RESTART ${thread_dead_restart})
  if [ ${verbose} -eq 1 ] ; then
    say "CACHE_ADP:THREAD_DEAD_RESTART = ${thread_dead_restart}"
  fi


  #
  # get the thread alive file name
  #
  thread_alive_file=$(cfgshell_lookup CACHE_ADP THREAD_ALIVE_FILE ${thread_alive_file})
  if [ ${verbose} -eq 1 ] ; then
    say "CACHE_ADP:THREAD_ALIVE_FILE = ${thread_alive_file}"
  fi
  if [ ! -e ${thread_alive_file} ] ; then
    if [ ${verbose} -eq 1 ] ; then
      say "File [${thread_alive_file}] not found."
    fi
    cfgshell_reset
    return ${is_hung}
  fi

  ## get the poll interval
  poll_interval=$(cfgshell_lookup CACHE_ADP POLL_INTERVAL ${poll_interval})
  if [ ${verbose} -eq 1 ] ; then
    say "CACHE_ADP:POLL_INTERVAL = ${poll_interval}"
  fi

  ## get the thread_grace
  thread_grace=$(cfgshell_lookup CACHE_ADP THREAD_GRACE ${thread_grace})
  if [ ${verbose} -eq 1 ] ; then
    say "CACHE_ADP:THREAD_GRACE = ${thread_grace}"
  fi

  last_mod_seconds=$(stat -c %Y ${thread_alive_file})
  seconds_now=$(date +%s)
  delta=$(( seconds_now - last_mod_seconds ))
  grace_seconds=$(( poll_interval + thread_grace ))

  if [ ${verbose} -eq 1 ] ; then
    say "File [${thread_alive_file}] hasn't been touched in ${delta} seconds."
  fi

  if [ ${delta} -ge ${grace_seconds} ] ; then
    if [ "${thread_dead_restart}" = "Y" ] ; then
      is_hung=1
      /usr/bin/logger -p local0.alert -t CVGALERT "ATSEv2 process [${pid}] cache update thread has died!  Restart imminent..."
    else
      /usr/bin/logger -p local0.alert -t CVGALERT "ATSEv2 process [${pid}] cache update thread has died!  Manual restart required!"
    fi
    if [ ${verbose} -eq 1 ] ; then
      say "Cache update thread has been dormant for ${delta} seconds!"
    fi
  fi

  cfgshell_reset
  return ${is_hung}
}

#
# main function
#
function atsev2_checkhung_main
{
  #
  # arguments passed in from atse/nodeagent/nodemonitor.sh
  #
  local    pid=${1:-99999}
  local    processName=${2:-TSESERVER}
  local    hung_time_threshold=${3:-600}

  #
  # local variables
  #
  local -i is_hung=0

  #
  # print variable values
  #
  if [ ${VERBOSE} -eq 1 ] ; then
    say "VALUES: verbose             => ${VERBOSE}"
    say ".       \$(pwd)             => $(pwd)"
    say ".       pid                 => ${pid}"
    say ".       processName         => ${processName}"
    say ".       hung_time_threshold => ${hung_time_threshold}"
  fi

  #
  # see if we're hung in startup
  #
  atsev2_hung_in_startup ${VERBOSE} ${pid} ${hung_time_threshold}
  is_hung=$?

  #
  # see if the cacheupdate thread is hung
  #
  if [ ${is_hung} -eq 0 ] ; then
    atsev2_cacheupdate_hung ${VERBOSE} ${pid}
    is_hung=$?
  fi

  return ${is_hung}
}

atsev2_checkhung_main $*



