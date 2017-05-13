#!/bin/bash

declare    APP=""
declare    HOST="$(hostname -s)"
declare    PORT=""
declare -a CACHENAMES=""
declare -a CACHEKEYS=""
declare -i CACHECOUNT=0
declare    MYHOMEDIR=""
declare -i NOSTOP=0

function get_cache_names
{
  local n=""
  for n in $(./cachetest.sh -host ${HOST} -port ${PORT} -cmd "type" | grep '[0-9]*\. .*' | awk '{print $2}') ; do
    CACHENAMES[${CACHECOUNT}]=${n}
    CACHECOUNT=$(expr ${CACHECOUNT} + 1)
  done
  echo "LDC is turned on for [${CACHECOUNT}] caches."
  if [ ${CACHECOUNT} -eq 0 ] ; then
    return 1
  else
    return 0
  fi
}

function dump_keys
{
  local -i idx=0
  local -i count=0
  while [ ${idx} -lt ${CACHECOUNT} ] ; do
    ./cachetest.sh $* -cache ${CACHENAMES[${idx}]} -cmd memkeys | grep -v '^INFO:' > testcacheupdate.tmp
    count=$(cat testcacheupdate.tmp | grep -E 'Received [0-9]* items\.' | awk '{print $3}')
    if [ ${count} -gt 0 ] ; then
      cat testcacheupdate.tmp
      return 1
    else
      CACHEKEYS[${idx}]=""
    fi
    idx=$(expr ${idx} + 1)
  done
  return 0
}

function wait_for_zero
{
  local prev_num="0"
  local num="0"
  local line=""

  while [ true  ] ; do
    line=$(grep "all queues" tseserver.log | tail -1)
    if [ -n "${line}" ] ; then
      num=$(echo ${line} | awk -F'[][]' '{print $2}')
      if [ "${num}" != "${prev_num}" ] ; then
        prev_num=${num}
        echo "LDC Queue Entries = ${num}"
      fi
      if [ "${num}" = "0" ] ; then
        break
      fi
    fi
    sleep 3
  done

  return 0
}

function start_server
{
  local savedir=$(pwd)
  local readyText="' TseServer is running '"

  echo "Starting tseserver..."

  if [ "${MYHOMEDIR}" = "/vobs/atseintl/Tools/ldc" ] ; then
    cd /vobs/atseintl/bin/debug
    export COLUMNS=800
    pids=$(ps -fu $(whoami) --no-header | grep /vobs/atseintl/bin/debug/tseserver | grep -v grep | awk '{print $2}')
    if [ -z "${pids}" ] ; then
      rm -f tseserver.log*
      (server.sh -D APPLICATION_CONSOLE.PORT=${PORT} > server.sh.stdout 2> server.sh.stderr) &
    else
      echo "Server already running."
    fi
  else
    cd /opt/atseintl/${APP}
    rm -f tseserver.log*
    istart.sh
  fi

  while [ ! -e tseserver.log ] ; do
    sleep 1
  done

  echo "Waiting for tseserver to be ready..."
  while [ -z "$(grep ' TseServer is running ' tseserver.log)" ] ; do
    sleep 1
  done

  echo "Waiting for tseserver LDC action queue to subside..."
  wait_for_zero

  echo "The tseserver is ready."

  cd ${savedir}
  return 0
}

function stop_server
{
  if [ ${NOSTOP} -eq 0 ] ; then

    local savedir=$(pwd)
    local pids=""
    local p=""

    echo "Stopping tseserver..."

    if [ "${MYHOMEDIR}" = "/vobs/atseintl/Tools/ldc" ] ; then
      cd /vobs/atseintl/bin/debug
      export COLUMNS=800
      pids=$(ps -fu $(whoami) --no-header | grep /vobs/atseintl/bin/debug/tseserver | grep -v grep | awk '{print $2}')
      for p in ${pids} ; do
        echo "Killing tseserver process [${p}]..."
        kill ${p}
        while [ -n "$(ps -p ${p} --no-headers)" ] ; do
          sleep 1
        done
      done
    else
      cd /opt/atseintl/${APP}
      istop.sh
    fi

    echo "The tseserver is stopped."

    cd ${savedir}

  fi

  return 0
}

function test_cache_update
{
  local -i rc=0
  local    answer=""

  # ensure we are running within the the directory
  # where we live
  local fullPathToMe=$(which ${0})
  MYHOMEDIR=$(dirname ${fullPathToMe})
  cd ${MYHOMEDIR}

  while [ -n "${1}" ] ; do
    case "${1}" in
      "-app"    ) shift ; APP=${1}    ;;
      "-host"   ) shift ; HOST=${1}   ;;
      "-port"   ) shift ; PORT=${1}   ;;
      "-nostop" ) NOSTOP=1            ;;
    esac
    shift
  done

  if [ "${MYHOMEDIR}" != "/vobs/atseintl/Tools/ldc" ] ; then
    while [ -z "${APP}" ] ; do
      echo ""
      echo -n "Please specify what V2 application we are testing with: "
      read answer
      if [ -n "${answer}" ] ; then
        APP=${answer}
      else
        echo "SORRY!  You must specify an application ID."
      fi
    done
  fi

  while [ -z "${HOST}" ] ; do
    echo ""
    echo -n "Please the host where the [${APP}] server is running: "
    read answer
    if [ -n "${answer}" ] ; then
      HOST=${answer}
    else
      echo "SORRY!  You must specify an application host."
    fi
  done

  while [ -z "${PORT}" ] ; do
    echo ""
    echo -n "Please the port where the [${APP}] server on host [${HOST}] is listening for AppConsole requests: "
    read answer
    if [ -n "${answer}" ] ; then
      PORT=${answer}
    else
      echo "SORRY!  You must specify an application port."
    fi
  done

  if [ ${rc} -eq 0 ] ; then
    start_server
    rc=$?
  fi

  if [ ${rc} -eq 0 ] ; then
    get_cache_names
    rc=$?
  fi

  if [ ${rc} -eq 0 ] ; then
    dump_keys
    rc=$?
  fi

  if [ ${rc} -eq 0 ] ; then
    stop_server
    rc=$?
  fi

  echo "Exiting with code [${rc}]."
  return ${rc}
}

test_cache_update $*

