#!/bin/bash

#
# Functions to convert tseserver config file to a flat file and
# grep for values
#
declare -i CFGSHELL_INMEM=1
declare -a CFGSHELL_LINES=()
declare -i CFGSHELL_LINE_COUNT=0
declare -i CFGSHELL_NPOS=-1
declare -i CFGSHELL_DOING_UNIT_TESTS=0
declare -i CFGSHELL_VERBOSE=0
declare    CFGSHELL_FLATFILE=/tmp/$(whoami).cfgshell.$$.tmp

function cfgshell_verbose_say
{
  if [ ${CFGSHELL_VERBOSE} -eq 1 ] ; then
    echo "cfgshell: $*"
  fi
}

function cfgshell_find_first_of
{
  local st="${1}"
  local ch="${2}"

  local -i pos=${CFGSHELL_NPOS}
  local -i sz=${#st}
  local -i idx=0

  while [ ${idx} -lt ${sz} ] ; do
    if [ "${st:${idx}:1}" = "${ch}" ] ; then
      pos=${idx}
      break
    else
      idx=$(( idx + 1 ))
    fi
  done

  echo ${pos}
}

function cfgshell_write_entry
{
  if [ ${CFGSHELL_INMEM} -eq 1 ] ; then
    CFGSHELL_LINES[${CFGSHELL_LINE_COUNT}]="${1}"
  else
    echo "${1}" >> ${CFGSHELL_FLATFILE}
  fi
  CFGSHELL_LINE_COUNT=$(( CFGSHELL_LINE_COUNT + 1 ))
}

function cfgshell_process_cfg
{
  local filenum=${1}
  local f=${2}
  local line=""
  local saveIFS=${IFS}
  local currGroup=""
  local key=""
  local val=""
  local -i pos=0
  local setcmd=""
  local temp=""

  cfgshell_verbose_say "Processing file [${f}]."

  IFS="
"

  if [ ${filenum} -gt 0 ] ; then
    cfgshell_write_entry ""
  fi

  for line in $(cat ${f}) ; do
    line=${line// /}
    if [ -n "${line}" ] ; then
      if [ "${line:0:1}" != "#" ] ; then
        if [ "${line:0:1}" = "[" ] ; then
          currGroup=${line//]/}
          currGroup=${currGroup//[/}
        else
          pos=$(cfgshell_find_first_of "${line}" "=")
          if [ ${pos} -ne ${CFGSHELL_NPOS} ] ; then
            key="${currGroup}/${line:0:${pos}}"
            pos=$(( pos + 1 ))
            val=${line:${pos}}
            cfgshell_write_entry "${key}=${val}"
          fi
        fi
      fi
    fi
  done

  IFS=${saveIFS}
}

function cfgshell_reset
{
  cfgshell_verbose_say "Resetting cfgshell..."
  rm -f ${CFGSHELL_FLATFILE}
  CFGSHELL_LINES=()
  CFGSHELL_LINE_COUNT=0
}

function cfgshell_flatten_from_file_list
{
  local -a files=()
  local -i filecount=0
  local -i idx=0

  cfgshell_reset
  cfgshell_verbose_say "Loading cfgshell from config files..."

  while [ -n "${1}" ] ; do
    if [ -e "${1}" ] ; then
      files[${filecount}]="${1}"
      filecount=$(( filecount + 1 ))
    else
      cfgshell_verbose_say "File [${1}] not found - skipping."
    fi
    shift
  done

  while [ ${idx} -lt ${filecount} ] ; do
    cfgshell_process_cfg ${idx} ${files[${idx}]}
    idx=$(( idx + 1 ))
  done
}

function cfgshell_flatten_from_tseserver_args
{
  local tse_cmd_line="$*"

  # Tokenize the command line.  A typical
  # set of command line tokens might look like this:
  #
  #  ./tseserver.pricing.1
  #  -baseline=atsev2.2010.03.04
  #  -adminapp=pricingv2
  #  -adminservice=PRICING
  #  -admininstance=1
  #  -appconsolegroup=Pricing
  #  -readyfile=./.PRICING.1.ready
  #  -appconsoleport=5000
  #  -appconsoleportfile=./.PRICING.1.appconsoleport
  #  -ipport=53701
  #  -ipportfile=./.PRICING.1.ipport
  #  -bigip=pincelb02.sabre.com
  #  -poolname=pricing_int_a
  #  -systype=TestHybrid
  #  -D
  #  TO_MAN.IOR_FILE=tseserver.pricing.ior
  #  -c
  #  ./tseserver.acms.cfg
  #  -D
  #  TSE_SERVER.OVERRIDE_CFGS=tseserver.cfg.node|tseserver.cfg.piili001
  #  -acport=5000
  #
  local -a cmdTokens=( ${tse_cmd_line} )

  # Iterate through the command line tokens and extract the config files.
  # Keep the first slot open in case for some odd reason the "-c" option
  # is missing or comes after OVERRIDE_CFGS.  Also, if "-c" is missing,
  # we assume 'tseserver.acms.cfg' in the first slot.

  local -i argIdx=0
  local    paramString=""
  local    paramKey="" ;
  local    paramVal=""
  local    mainConfigFile="tseserver.acms.cfg"
  local    otherConfigs=""

  while [ -n "${cmdTokens[${argIdx}]}" ] ; do
    case "${cmdTokens[${argIdx}]}" in
      "-c") argIdx=$(( argIdx + 1 ))
            mainConfigFile=${cmdTokens[${argIdx}]}
            ;;
      "-D") argIdx=$(( argIdx + 1 ))
            paramString=${cmdTokens[${argIdx}]}
            paramKey=$(echo ${paramString} | awk -F'=' '{print $1}')
            if [ "${paramKey}" = "TSE_SERVER.OVERRIDE_CFGS" ] ; then
              paramVal=$(echo ${paramString} | awk -F'=' '{print $2}')
              if [ -n "${otherConfigs}" ] ; then
                otherConfigs="${otherConfigs} "
              fi
              otherConfigs="${otherConfigs}$(echo ${paramVal} | tr '|' ' ' )"
            fi
            ;;
      *   ) ;;
    esac
    argIdx=$(( argIdx + 1 ))
  done

  cfgshell_flatten_from_file_list ${mainConfigFile} ${otherConfigs}
}

function cfgshell_show_entries
{
  if [ ${CFGSHELL_INMEM} -eq 1 ] ; then
    local -i idx=0
    while [ ${idx} -lt ${CFGSHELL_LINE_COUNT} ] ; do
      echo ${CFGSHELL_LINES[${idx}]}
      idx=$(( idx + 1 ))
    done
  else
    cat ${CFGSHELL_FLATFILE}
  fi
}

function cfgshell_lookup
{
  local group=${1}
  local key=${2}
  local default=${3}
  local value=""

  value=$(cfgshell_show_entries | grep "${1}/${2}=" 2> /dev/null | tail -1 | awk -F= '{print $2}')
  if [ -z "${value}" ] ; then
    value=${default}
  fi

  echo ${value}
}

function cfgshell_unit_test
{
  CFGSHELL_DOING_UNIT_TESTS=1
  local files=""
  local group=""
  local key=""
  local default=""
  local val=""
  local -i before=0
  local -i after=0
  local -i loadsecs=0
  local -i lookupsecs=0

  while [ -n "${1}" ] ; do
    case "${1}" in
      "-g") shift ; group=${1} ;;
      "-k") shift ; key=${1} ;;
      "-d") shift ; default=${1} ;;
         *) files="${files} ${1}" ;;
    esac
    shift
  done

  before=$(date +%s)
  cfgshell_flatten_from_file_list ${files}
  after=$(date +%s)
  loadsecs=$(( after - before ))

  echo ""
  cfgshell_show_entries

  echo ""
  echo "Flattening the tseserver config file(s) took ${loadsecs} seconds."

  echo ""
  if [ ${CFGSHELL_INMEM} -eq 1 ] ; then
    echo "Used in-memory array for entries"
  else
    echo "CFGSHELL_FLATFILE=${CFGSHELL_FLATFILE}"
  fi

  if [ -n "${group}" -a -n "${key}" ] ; then
    before=$(date +%s)
    val=$(cfgshell_lookup ${group} ${key} ${default})
    after=$(date +%s)
    lookupsecs=$(( after - before ))
    echo ""
    echo "Lookup: ${group}/${key}=${val}"
    echo "Looking up the value took ${lookupsecs} seconds."
  fi

  cfgshell_reset

  echo ""
  echo "_____________________________________________________________"
  echo "Unit test for $(basename ${0}) is complete."
  echo "If you want to use the functions in your script, source it in!"

  echo ""
  CFGSHELL_DOING_UNIT_TESTS=0
  return 0
}

