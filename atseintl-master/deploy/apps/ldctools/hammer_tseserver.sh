#!/bin/bash
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# This script reads input from <STDIN>. Use it to replay
# tserequest.log transaction logs and send them to your
# development tseserver. You also need hammer.pl.
#


# ensure we are running within the same directory
# where we are located
cd $(dirname $(which ${0}))

. ./env.vars

CONCURRENT=1
LIMIT=0
RETRY=0
declare -a PIDS
LAST_SLOT_USED=0
TXNCOUNT=0
RESUME=0
TERMINATING=0
LASTTXNFILE=""
REQRSPFILE=""
LOGFILE=""
ROOT=""

declare PERL585="${ENV_VARS_LOCATION}/pms/lib64/perl5/5.8.5/x86_64-linux-thread-multi"
declare PERL588="${ENV_VARS_LOCATION}/pms/lib64/perl5/5.8.8/x86_64-linux-thread-multi"

export LD_LIBRARY_PATH="${PERL585}/auto/Time/HiRes:${LD_LIBRARY_PATH}"
export PERL5LIB="${ENV_VARS_LOCATION}:${PERL585}/Time:${PERL588}:${PERL588}/XML:${PERL588}/XML/Parser:${PERL5LIB}"

HAMMER_PL=$(which hammer.pl)

function terminate
{
  TERMINATING=1
  echo "Terminating..."
  ${HOME}/bin/killfamily.sh $$ > /dev/null 2>&1 /dev/null < /dev/null
}

function clear_pids
{
  local -i idx=0
  while [ ${idx} -lt ${CONCURRENT} ] ; do 
    PIDS[${idx}]=0 
    idx=$(expr ${idx} + 1)
  done
}

function wait_for_slot 
{
  local -i waiting=1
  local -i next_try=0

  next_try=$(expr ${LAST_SLOT_USED} + 1)

  while [ ${waiting} -eq 1 ] ; do

    if [ ${next_try} -eq ${CONCURRENT} ] ; then
      next_try=0
    fi

    if [ ${PIDS[${next_try}]} -eq 0 ] ; then
      PIDS[${next_try}]=99999
      LAST_SLOT_USED=${next_try}
      waiting=0 
    elif [ -z "$(ps -p ${PIDS[${next_try}]} --no-header)" ] ; then 
      PIDS[${next_try}]=99999
      LAST_SLOT_USED=${next_try}
      waiting=0 
    else
      next_try=$(expr ${next_try} + 1)
    fi 

  done
}

function request_thread
{
  local -i slot=${1}
  shift

  # send the request
  eval "$*"

  # let go of our slot
  PIDS[${slot}]=0
}

function spawn_request
{
  local REQUEST=$*
  local REQUEST_XML=""

  # Strip out the junk before the XML request.
  #
  REQUEST_XML=$(echo "${REQUEST}" | sed 's/^200[0-9].*TseManagerUtil Request Received - //')
  #echo "${REQUEST_XML}"
  
  local COMMAND="echo '${REQUEST_XML}' | perl ${HAMMER_PL} --noavail -h ${HOST} -p ${PORT} --stdin --map ${REQRSPFILE}"
  if [ ${RETRY} -eq 1 ] ; then
    COMMAND="${COMMAND} --retry"
  fi
  #echo "Executing command: ${COMMAND}"

  #wait for a slot in our pid list
  wait_for_slot

  #start thread
  request_thread ${LAST_SLOT_USED} ${COMMAND} >> ${LOGFILE} 2>&1 < /dev/null &
  PIDS[${LAST_SLOT_USED}]=$! 
}

## -=-=-=-=-
## MAIN LINE
## -=-=-=-=-

trap 'terminate' SIGINT

while [ -n "${1}" ] ; do
  case "${1}" in 
    "-app"       ) shift ; APP=${1}        ;;
    "-host"      ) shift ; HOST=${1}       ;;
    "-port"      ) shift ; PORT=${1}       ;;
    "-concurrent") shift ; CONCURRENT=${1} ;;
    "-limit"     ) shift ; LIMIT=${1}      ;;
    "-retry"     ) RETRY=1                 ;;
    "-resume"    ) RESUME=1                ;;
  esac
  shift
done

APP=$(echo ${APP} | tr "[:upper:]" "[:lower:]")

if [ "${APP}" = "pricingv2" ] ; then
  ROOT="PricingRequest"
elif [ "${APP}" = "shoppingis" ] ; then
  ROOT="ShoppingRequest"
elif [ "${APP}" = "historical" ] ; then
  ROOT="PricingRequest"
else
  echo "Unsupported app [${APP}]."
  exit 1
fi

LASTTXNFILE="${APPDIR}/hammer.txn"
LOGFILE="${APPDIR}/hammer.log"
REQRSPFILE=${APPDIR}/hammer.reqrsp.log

if [ ${CONCURRENT} -lt 1 ] ; then
  CONCURRENT=1
fi

if [ ${LIMIT} -lt 0 ] ; then
  LIMIT=0
fi

# There's something funky about running the client and server
# both on atseitst1. The client can't connect using the fully
# qualified host name from the IOR file.
if [ "${HOST}" = "atseitst1.dev.sabre.com" ] ; then
  HOST=${HOST%%.*}
fi

echo "App        : ${APP}"
echo "Host       : ${HOST}"
echo "Port       : ${PORT}"
echo "Concurrent : ${CONCURRENT}"
echo "Log File   : ${LOGFILE}"

rm -f ${LOGFILE}

declare -i begin_at=1 
if [ ${RESUME} -eq 1 -a -e ${LASTTXNFILE} ] ; then
  begin_at=$(cat ${LASTTXNFILE})
fi

clear_pids

if [ ${RESUME} -eq 1 ] ; then
  if [ ${begin_at} -gt 1 ] ; then
    echo "Waiting to begin at ${begin_at}..."
  fi
fi

rm -f ${REQRSPFILE}

declare REQUEST=""
while read REQUEST ; do
  #echo "${REQUEST}"
  if [ ${TERMINATING} = 1 ] ; then
    break
  fi

  TXNCOUNT=$(expr ${TXNCOUNT} + 1)
  if [ ${RESUME} -eq 1 ] ; then
    if [ ${TXNCOUNT} -lt ${begin_at} ] ; then
      continue
    else
      echo "Resuming at #${begin_at}"
      RESUME=0
    fi
  fi

  spawn_request "${REQUEST}"

  echo ${TXNCOUNT} > ${LASTTXNFILE}

  if [ ${LIMIT} -gt 0 -a ${TXNCOUNT} -ge ${LIMIT} ] ; then
    echo "Transaction limit [${LIMIT}] reached.  Quitting."
    break
  elif [ $(expr ${TXNCOUNT} % 500) -eq 0 ] ; then
    echo "Processed ${TXNCOUNT} transactions..." 
  fi

done

