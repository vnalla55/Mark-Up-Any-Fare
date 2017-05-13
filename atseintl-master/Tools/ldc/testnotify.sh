#!/bin/bash

declare    MYFULLPATH=$(which ${0})
declare    MYHOMEDIR=$(dirname ${MYFULLPATH})
declare    ENVVARS=${MYHOMEDIR}/env.vars
declare    CACHE=""
declare -i LONGEST=0
declare -i VERBOSE=0
declare -i ERRORCOUNT=0
declare    INJECTTYPE="D"
declare -a LOUS=()
declare -a NAMES=()
declare -a KEYS=()
declare -a NOTIFYS=()
declare -a NAS=()
declare    TEMP_SCRIPT="/tmp/$(whoami).testnotify.tmp"

if [ ! -e ${ENVVARS} ] ; then
  ENVVARS=/vobs/atseintl/deploy/apps/ldctools/env.vars
fi

. ${ENVVARS}

# ensure we are running within the the directory
# where we live
cd ${MYHOMEDIR}

function print_result
{
  local -i isError=${1}
  local    cacheName="${2}"
  local    tag="${3}"

  if [ ${isError} -eq 0 ] ; then
    if [ ${VERBOSE} -eq 0 ] ; then
      return 0
    fi
  else
    ERRORCOUNT=$(expr ${ERRORCOUNT} + 1)
  fi

  if [ ${isError} -eq 1 ] ; then
    echo -n "*** "
  else
    echo -n "    "
  fi

  while [ ${#cacheName} -lt ${LONGEST} ] ; do
    cacheName="${cacheName} "
  done

  echo -n "${cacheName}"

  if [ ${isError} -eq 1 ] ; then
    echo -n " [-ERROR-] "
  else
    echo -n " [-OK-]    "
  fi

  echo ${tag} | sed 's/ /_/g'
}

function test_numeric
{
  local rc=0
  case ${1} in
    *[0-9] ) ;;
         * ) rc=1 ;;
  esac
  return ${rc}
}

function checkLoadOnUpdate
{
  local -i bool=0
  local    name=${1}
  local -i idx=0
  while [ ${idx} -lt ${#LOUS[*]} ] ; do
    if [ "${LOUS[${idx}]}" = "${name}" ] ; then
      bool=1
      break
    fi
    idx=$(expr ${idx} + 1)
  done
  return ${bool} ;
}

function checkExistence
{
  local -i existsIsError=${1}
  shift

  local -i notExistsIsError=0
  local -i i=0
  local    cacheName=""
  local    cacheKey=""
  local    notifyKey=""
  local    x=""
  local    ignore=""
  local    loc=""
  local    stat=""

  if [ ${existsIsError} -eq 0 ] ; then
    notExistsIsError=1
  fi

  rm -f ${TEMP_SCRIPT}
  while [ ${i} -lt ${#NAMES[*]} ] ; do
    cacheName="${NAMES[${i}]}"
    cacheKey="${KEYS[${i}]}"
    notifyKey="${NOTIFYS[${i}]}"
    echo "cache_exact ${cacheName}" >> ${TEMP_SCRIPT}
    echo "exist ${cacheKey}" >> ${TEMP_SCRIPT}
    i=$(expr ${i} + 1)
  done

  temp=$(cachetest.sh $* -cmd run ${TEMP_SCRIPT} | grep -o -E '(Memory|LDC) *= *(TRUE|FALSE)' | sed 's/[][]//g')

  i=1
  for x in ${temp} ; do
    case ${i} in
      1) loc=${x}    ;;
      2) ignore=${x} ;;
      3) stat=${x}   ;;
    esac
    i=$(expr ${i} + 1)
    if [ ${i} -gt 3 ] ; then
      if [ "${stat}" = "TRUE" ] ; then
        if [ ${existsIsError} -eq 1 ] ; then
          checkLoadOnUpdate ${cacheName}
          if [ $? -eq 1 ] ; then
            print_result 0 ${cacheName} "key exists in ${loc} (loadOnUpdate=TRUE)"
          else
            print_result ${existsIsError} ${cacheName} "key exists in ${loc}"
          fi
        else
          print_result ${existsIsError} ${cacheName} "key exists in ${loc}"
        fi
      else
        print_result ${notExistsIsError} ${cacheName} "key does not exist in ${loc}"
      fi
      i=1
    fi
  done
}

function test_cache_update
{
  local -i idx=0
  local -i naidx=0
  local    passOnArgs=""
  local    answer=""
  local    tok=""
  local    cacheName=""
  local    cacheKey=""
  local    cacheSetting=""
  local    notifyKey=""
  local    saveIFS=${IFS}
  local    result=""
  local    cache_arg=""
  local    all_arg="all"

  local -i i=0
  local    temp=""
  local    buf=""
  local    cName=""
  local    nName=""
  local    opstat=""

  echo ""

  while [ -n "${1}" ] ; do
    case "${1}" in
      "-app"    ) shift ; APP=${1}   ;;
      "-host"   ) shift ; HOST=${1}  ;;
      "-port"   ) shift ; PORT=${1}  ;;
      "-ior"    ) shift ; IOR=${1}   ;;
      "-cache"  ) shift ; CACHE=${1} ; cache_arg="-cache ${CACHE}" ; all_arg="" ;;
      "-v"      ) VERBOSE=1 ;;
      "-h"      ) echo "Usage: testnotify.sh [options]"
                  echo ""
                  echo "  where [options] is one of more of the following:"
                  echo ""
                  echo "    -cache <name>   specific cache name to test; if"
                  echo "                    omitted, ALL eligible caches will"
                  echo "                    be tested"
                  echo "    -host <host>    tseserver host; overrides any other"
                  echo "                    method of host determination"
                  echo "    -port <port>    tseserver AppConsole port; overrides"
                  echo "                    any other method of port determination"
                  echo "    -ior <filename> tseserver.ior file (by default the "
                  echo "                    script looks for ${HOME}/tseserver.ior);"
                  echo "                    meaningless if -host and -port are"
                  echo "                    specified"
                  echo "    -v              verbose output"
                  echo "    -h              print usage and exit"
                  echo ""
                  exit 1
                  ;;
    esac
    shift
  done

  while [ -z "${HOST}" ] ; do
    echo ""
    echo -n "Please specify the host where the [${APP}] server is running: "
    read answer
    if [ -n "${answer}" ] ; then
      HOST=${answer}
    else
      echo "SORRY!  You must specify an application host."
    fi
  done

  while [ -z "${PORT}" ] ; do
    echo ""
    echo -n "Please specify the port where the [${APP}] server on host [${HOST}] is listening for AppConsole requests: "
    read answer
    if [ -n "${answer}" ] ; then
      PORT=${answer}
    else
      echo "SORRY!  You must specify an application port."
    fi
  done

  if [ -n "${APP}"  ] ; then passOnArgs="${passOnArgs} -app  ${APP}"  ; fi
  if [ -n "${HOST}" ] ; then passOnArgs="${passOnArgs} -host ${HOST}" ; fi
  if [ -n "${PORT}" ] ; then passOnArgs="${passOnArgs} -port ${PORT}" ; fi

  echo ""
  echo "Generating dummy key(s)..."
  i=0
  idx=0
  naidx=0
  for tok in $(cachetest.sh ${passOnArgs} ${cache_arg} -cmd dummy ${all_arg} | grep NotifyKey | sed 's/\[//g' | sed 's/\]//g' )
  {
    case ${idx} in
      0) cacheName="${tok}" ; idx=1 ;;
      1) cacheKey="$(echo ${tok} | sed 's/CacheKey\=//g')" ; idx=2 ;;
      2) notifyKey="$(echo ${tok} | sed 's/NotifyKey\=//g')"
         idx=0
         if [ ${#cacheName} -gt ${LONGEST} ] ; then
           LONGEST=${#cacheName}
         fi
         if [ "${notifyKey}" = "N/A" ] ; then
           NAS[${naidx}]="${cacheName}"
           naidx=$(expr ${naidx} + 1)
         else
           NAMES[${i}]="${cacheName}"
           KEYS[${i}]="${cacheKey}"
           NOTIFYS[${i}]="${notifyKey}"
           i=$(expr ${i} + 1)
         fi
         ;;
    esac
  }

  if [ ${#NAMES[*]} -eq 0 ] ; then
    echo "ERROR: No dummy KEYS were generated."
    echo "Are you sure there's a tseserver listening on host [${HOST}] at port [${PORT}]?"
    exit 1
  fi

  i=0
  while [ ${i} -lt ${#NAMES[*]} ] ; do
    cacheName="${NAMES[${i}]}"
    cacheKey="${KEYS[${i}]}"
    print_result 0 ${cacheName} "dummy key [${cacheKey}]"
    i=$(expr ${i} + 1)
  done

  if [ ${#NAS[*]} -gt 0 ] ; then
    echo ""
    echo "NOTE: The following LDC-enabled caches are not configured for notifications in cacheNotify.xml and will be skipped:"
    naidx=0
    while [ ${naidx} -lt ${#NAS[*]} ] ; do
      echo "      ${NAS[${naidx}]}"
      naidx=$(expr ${naidx} + 1)
    done
  fi

  echo ""
  echo "Waiting 25 seconds to make sure LDC action queue is processed..."
  sleep 25

  echo ""
  echo "Obtaining CacheParm struct information..."
  for temp in $(cachetest.sh ${passOnArgs} -cmd parm all | grep -o -E '.*\.loadOnUpdate=.*') ; do
    cacheName=$(echo "${temp}" | awk -F'.' '{print $1}')
    cacheSetting=$(echo "${temp}" | awk -F'=' '{print $2}')
    if [ "${cacheSetting}" = "1" ] ; then
      i=${#LOUS[*]}
      LOUS[${i}]="${cacheName}"
    fi
  done

  echo ""
  echo "Making sure dummy keys exist in both Memory and LDC..."
  checkExistence 0 ${passOnArgs}

  echo ""
  echo "Injecting cache notifications..."
  i=0
  rm -f ${TEMP_SCRIPT}
  while [ ${i} -lt ${#NAMES[*]} ] ; do
    cacheName="${NAMES[${i}]}"
    cacheKey="${KEYS[${i}]}"
    notifyKey="${NOTIFYS[${i}]}"
    echo "cache_exact ${cacheName}" >> ${TEMP_SCRIPT}
    echo "inject ${INJECTTYPE} ${notifyKey}" >> ${TEMP_SCRIPT}
    i=$(expr ${i} + 1)
  done

  if [ ${VERBOSE} -eq 1 ] ; then
    result=$(cachetest.sh ${passOnArgs} -cmd run ${TEMP_SCRIPT} | grep "key string")
  else
    result=$(cachetest.sh ${passOnArgs} -cmd run ${TEMP_SCRIPT} | grep "key string" | grep ERROR)
  fi

  if [ -n "${result}" ] ; then
    IFS='
'
    for temp in ${result} ; do

      temp=$(echo ${temp} | sed 's/\[//g' | sed 's/\]//g' | sed 's/\.//g') ;
      cName=$(echo ${temp} | awk -F' ' '{print $1}')
      opstat=$(echo ${temp} | grep ERROR)

      if [ -z "${opstat}" ] ; then
        opstat="[-OK-]    injected"
      else
        opstat="[-ERROR-] failed  "
      fi

      while [ ${#cName} -lt ${LONGEST} ] ; do
        cName="${cName} "
      done
      buf="    ${cName} ${opstat} "
      nName=$(echo "${temp}" | awk -F' ' '{print $11}')
      while [ ${#nName} -lt ${LONGEST} ] ; do
        nName="${nName} "
      done
      buf="${buf}${nName} => "
      buf="${buf}"$(echo "${temp}" | awk -F' ' '{print $5}')
      buf=$(echo "${buf}" | tr -d '\n')
      echo "${buf}"
    done
    IFS=${saveIFS}
  fi

  echo ""
  echo "Waiting 90 seconds to give the server a chance to process..."
  sleep 90

  echo ""
  echo "Making sure dummy keys no longer exist in either Memory or LDC..."
  checkExistence 1 ${passOnArgs}

  return ${ERRORCOUNT}
}

function main
{
  time test_cache_update $*

  echo ""
  echo "Test finished."
  if [ ${ERRORCOUNT} -eq 0 ] ; then
    echo "NO errors found."
  elif [ ${ERRORCOUNT} -eq 1 ] ; then
    echo "1 error found !!!"
  else
    echo "${ERRORCOUNT} errors found !!!"
  fi

  return ${ERRORCOUNT}
}

main $*

