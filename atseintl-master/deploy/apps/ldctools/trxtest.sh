#!/bin/sh

# ensure we are running within the same directory
# where we are located
cd $(dirname $(which ${0}))

declare ENV_VARS_LOCATION=""
if [ -e ./env.vars ] ; then
  . ./env.vars
fi

if [ -z "${APP}" ] ; then
  echo "SORRY!  The APP environment variable is not set"
  exit 1
fi

if [ -z "${ENV_VARS_LOCATION}" ] ; then
  ENV_VARS_LOCATION="."
fi

declare -i limit=0
declare -i conc=0
declare    file=""
declare    port=${REQPORT}
declare    host=${HOST}

while [ -n "${1}" ] ; do
  case "${1}" in
    "-c"    ) shift
              conc="${1}"
              ;;
    "-f"    ) shift
              file="${1}"
              ;;
    "-host" ) shift
              host="${1}"
              ;;
    "-port" ) shift
              port="${1}"
              ;;
    "-h"    ) echo
              echo "usage: trxtest.sh [options] [limit]"
              echo
              echo "  where [limit] is an optional numeric limit on "
              echo "                the number of transactions to process"
              echo
              echo "    and [options] is one or more of the following:"
              echo
              echo "      -c    <num>   number of concurrent requests"
              echo "      -f    <file>  the transaction file to process"
              echo "      -h            show usage information and exit"
              echo "      -host <host>  host where tseserver is running"
              echo "      -port <port>  port for tseserver requests"
              echo
              exit 0
              ;;
    *       ) limit=${1}
              ;;
  esac
  shift
done

if [ -z "${file}" ] ; then
  file=${ENV_VARS_LOCATION}/${APP}.5000.tserequest.log.gz
fi

if [ ${limit} -eq 0 ] ; then
  limit=5000
fi

if [ ${conc} -eq 0 ] ; then
  conc=12
fi

if [ ! -e "${file}" ] ; then
  echo "SORRY!  ${file} was not found."
  exit 1
fi

rm  -f "${APPDIR}/hammer.txn"

declare tmpfile=""

if [ -d "${file}" ] ; then
  tmpfile="/tmp/$(whoami).trxtest.tmp"
  rm -f ${tmpfile}
  cat ${file}/* > ${tmpfile}
  file=${tmpfile}
fi

declare ext=${file##*.}
declare cmd=""

if [ "${ext}" = "gz" ] ; then
  cmd="/bin/zcat"
else
  cmd="/bin/cat"
fi

cmd="${cmd} ${file} | hammer_tseserver.sh -app ${APP} -host ${host} -port ${port} -concurrent ${conc} -retry -limit ${limit} -resume"
echo $cmd
eval $cmd

if [ -n "${tmpfile}" ] ; then
  rm -f ${tmpfile}
fi
