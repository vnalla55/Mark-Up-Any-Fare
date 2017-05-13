#!/bin/sh

declare    SCRIPT_FILE=$(which ${0})
declare    SCRIPT_DIR=$(dirname ${SCRIPT_FILE})
declare    LD_LIBRARY_PATH=""
declare -i VERBOSE=0
declare    RHVER="$(grep -o -E "release [0-9]*" /etc/redhat-release | awk '{print $2}')"
declare    CONFIG_FUNCTIONS=${SCRIPT_DIR}/config_functions.sh
declare    ENV_VARS=${SCRIPT_DIR}/env.vars

if [ ! -e ${CONFIG_FUNCTIONS} ] ; then
  CONFIG_FUNCTIONS=/vobs/atseintl/deploy/apps/tseshared/config_functions.sh
fi

if [ ! -e ${ENV_VARS} ] ; then
  ENV_VARS=/vobs/atseintl/deploy/apps/ldctools/env.vars
fi

. ${CONFIG_FUNCTIONS}

if [ "${COMMON_ATSEINTL_HOME}" = "/vobs/atseintl/atseintl" ] ; then
  export COMMON_ATSEINTL_HOME=/opt/atseintl
fi

. ${ENV_VARS}

cd ${SCRIPT_DIR}

function test_numeric
{
  local rc=0
  case ${1} in
    *[0-9] ) ;;
         * ) rc=1 ;;
  esac
  return ${rc}
}

function run_cachetest
{
  local -i debug=0
  local    currDir=$(pwd)
  local    vob="/vobs/atseintl"
  local    args=""
  local    iorfile="${HOME}/tseserver_appconsole.ior"
  local    host=${HOST}
  local    port=${PORT}
  local    app=${APP}
  local    exe="${currDir}/debug/cachetest"
  local    ld=0

  if [ ! -e ${exe} ] ; then
    exe="./cachetest"
  fi

  while [ -n "${1}" ] ; do
    case "${1}" in
      "-debug" ) debug=1 ;;
      "-app"   ) shift ; app=${1}  ;;
      "-host"  ) shift ; host=${1} ;;
      "-port"  ) shift ; port=${1} ;;
      "-ld"    ) ld=1 ;;
             * ) args="${args} ${1}"
    esac
    shift
  done

  args="-host ${host} -port ${port} ${args}"

  add_to_lib_var ${RHVER} "LD_LIBRARY_PATH"

  if [ -e /vobs/atseintl/lib/debug ] ; then
    LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/vobs/atseintl/lib/debug
  fi

  if [ -n "${APPDIR}" ] ; then
    LD_LIBRARY_PATH=${APPDIR}/lib:${LD_LIBRARY_PATH}
  fi

  LD_LIBRARY_PATH=$(echo ${LD_LIBRARY_PATH} | sed 's/\/vobs\/atseintl\/atseintl/\/opt\/atseintl\//g')

  export LD_LIBRARY_PATH

  if [ ${ld} -eq 1 ] ; then
    echo
    echo "COMMON_ATSEINTL_HOME:"
    echo "  ${COMMON_ATSEINTL_HOME}"
    echo "LD_LIBRARY_PATH:"
    echo "  ${LD_LIBRARY_PATH}" | sed 's/:/\n  /g' 
    echo
  fi

  if [ ${debug} -eq 1 ] ; then
    totalview ${exe} -search_path ${vob} -a ${args}
  else
    ${exe} ${args}
  fi
}

run_cachetest $*

