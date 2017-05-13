#!/bin/sh

SCRIPT_FILE=$(which ${0})
SCRIPT_DIR=$(dirname ${SCRIPT_FILE})
SEPARATOR="#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
DBHOSTPORT=""
DATABASE=""
PORT=""

function compareType
{
  local type=${1}
  shift

  echo ""
  echo ${SEPARATOR}
  echo " Checking ${type}..."
  local cmd="./cacheCompare -z -h ${DATABASE} -P ${PORT} -d ${LDCLOC} -t ${type} $*"
  echo " ${cmd}"
  eval ${cmd}
  echo " Finished checking ${type}."
}

cd ${SCRIPT_DIR}
. ./env.vars
cd ${SCRIPT_DIR}
. ./wcfg_functions.sh
cd ${SCRIPT_DIR}

if [ -z "${ATSEDIR}" ] ; then
  ATSEDIR="$(dirname ${SCRIPT_DIR})"
fi

if [ -z "${APP}" ] ; then
  APP=pricingv2
fi

DBACCESS=${ATSEDIR}/${APP}/dbaccess.ini
DBHOSTPORT="$(wcfg_get_dbaccess_host_port ${DBACCESS})"
DATABASE=$(echo ${DBHOSTPORT} | awk '{print $1}')
PORT=$(echo ${DBHOSTPORT} | awk '{print $2}')

if [ -z "${LDCLOC}" ] ; then
  if [ -e ${ATSEDIR}/ldc ] ; then
    LDCLOC="${ATSEDIR}/ldc"
  elif [ -e /tmp/$(whoami)/ldc ] ; then
    LDCLOC="/tmp/$(whoami)/ldc"
  fi
fi

compareType FARECACHENOTIFY $*
compareType ROUTINGCACHENOTIFY $*
compareType SUPPORTCACHENOTIFY $*
compareType INTLCACHENOTIFY $*
compareType HISTORICALCACHENOTIFY $*
compareType RULECACHENOTIFY $*

echo ""
echo ${SEPARATOR}
echo " PROCESSING COMPLETE."
echo ""

