#!/bin/bash

#
# Functions to read a WIN.INI style configuration file
# and extract values
#

declare -a WCFG_KEYARRAYS=()
declare -a WCFG_VALARRAYS=()
declare -i WCFG_SIZE=0

function wcfg_clear
{
  WCFG_KEYARRAYS=()
  WCFG_VALARRAYS=()
  WCFG_SIZE=0

  return 0
}

function wcfg_load
{
  local -i rc=0
  local filename=${1}
  local currgroup=""

  wcfg_clear

  if [ -e ${filename} ] ; then
    local saveIFS=${IFS}
    IFS="
"
    local line=""
    for line in $(cat ${filename}) ; do
      line=$(echo "${line}" | sed 's/^ *//g' | sed 's/ *$//g')
      if [ -n "${line}" ] ; then
        if [ "${line:0:1}" != '#' ] ; then
          if [ "${line:0:1}" = '[' ] ; then
            currgroup=${line}
          else
            WCFG_KEYARRAYS[${WCFG_SIZE}]=${currgroup}$(echo "${line}" | awk -F'=' '{print $1}')
            WCFG_VALARRAYS[${WCFG_SIZE}]=$(echo "${line}" | awk -F'=' '{print $2}')
            WCFG_SIZE=$(( WCFG_SIZE + 1 ))
          fi
        fi
      fi
    done
    IFS=${saveIFS}
  else
    echo "ERROR: Input file not found"
    rc=1
  fi

  return ${rc}
}

function wcfg_find
{
  local key=${1}
  local val=""

  local -i rc=0
  local -i idx=0

  while [ ${idx} -lt ${WCFG_SIZE} ] ; do
    if [ "${WCFG_KEYARRAYS[${idx}]}" = "${key}" ] ; then
      val="${WCFG_VALARRAYS[${idx}]}"
      break
    fi
    idx=$(( idx + 1 ))
  done

  echo "${val}"

  return ${rc}
}

function wcfg_get_dbaccess_host_port
{
  wcfg_load ${1}

  local -i rc=0

  local host=""
  local port=""
  local defaultConn=$(wcfg_find "[Connection]Default")
  local useIni=$(wcfg_find "[${defaultConn}]ini")
  if [ -z "${useIni}" ] ; then
    host=$(wcfg_find "[${defaultConn}]host")
    port=$(wcfg_find "[${defaultConn}]port")
  elif [ "${useIni}" = "dbconn.ini" ] ; then
    #
    # DBConnect\HYBRID=hybfunc@pimhp017.sabre.com:3306/ATSEHYB1,hybfunc@pinhpp84.sabre.com:3306/ATSEHYB1
    #
    host=$(cat /opt/db/ini/dbconn.ini | grep "DBConnect\\\HYBRID=" | grep -v "#" | awk -F'[@:]' '{print $2}')
    port=$(cat /opt/db/ini/dbconn.ini | grep "DBConnect\\\HYBRID=" | grep -v "#" | awk -F'[@:/]' '{print $3}')
  fi

  echo "${host} ${port}"

  if [ -z "${host}" -o -z "${port}" ] ; then
    rc=1
  fi

  return ${rc}
}


