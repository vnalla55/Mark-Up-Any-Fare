#!/bin/sh

CALLER_DIR=$(pwd)
MY_PARENT=$(dirname $(which ${0}))

COMMON_LOGFILE="${CALLER_DIR}/bigIp.log"
COMMON_SYSTYPE="TestHybrid"
COMMON_BIG_IP="pincelb02.sabre.com"
COMMON_SSH_OPTIONS="-q -o BatchMode=yes -o StrictHostKeyChecking=no"
COMMON_NODE=$(hostname -s)
COMMON_SCRIPTNAME=${0}

function say
{
  local line="[${COMMON_NODE}] $(date '+%Y-%m-%d %H:%M:%S') - ${COMMON_SCRIPTNAME}: ${*}"
  echo ${line} >> ${COMMON_LOGFILE}
}

function exitError
{
  local errorCode=${1}
  shift
  say "ERROR ${errorCode}: $*"
  say "Aborted with exit code ${errorCode}."
  exit ${errorCode}
}

function main
{
  local service=${1}
  local action=${2}
  local -i instances=${3}
  local poolname=${4}
  local nodename=${5}
  local port=${6}

  local rc=0

  local common_bigip_script="${MY_PARENT}/bigipsvc.sh"
  if [ ! -e ${common_bigip_script} ] ; then
    common_bigip_script="/opt/atse/common/bigipsvc.sh"
    if [ ! -e ${common_bigip_script} ] ; then
      common_bigip_script=""
    fi
  fi

  if [ -n "${common_bigip_script}" ] ; then
    local cmd="register"
    if [ "${action}" = "stop" ] ; then
      cmd="deregister"
    fi
    ${common_bigip_script} -v -keeplog -connlimit 0 ${COMMON_SYSTYPE} ${COMMON_BIG_IP} ${cmd} ${poolname} ${port} > ${COMMON_LOGFILE} 2>&1 < /dev/null
    rc=$?
    if [ ${rc} -ne 0 ] ; then
      exitError ${rc} "The bigipsvc.sh script returned bad exit code [${rc}]!"
    fi
  else
    exitError 1 "Unable to locate bigipsvc.sh script!"
  fi

  return ${rc}
}

rm -rf ${COMMON_LOGFILE}
say "Entering bigIp.sh script."
main ${*}
RC=$?
say "Completed with exit code ${RC}."
exit ${RC}



