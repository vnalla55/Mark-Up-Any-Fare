#!/bin/bash

#
# Convert ATSEv2 ini file parameters to a "config.vars"
# one-line-per item format
#

declare -a FILES
declare -i FILECOUNT=0
declare -a KEYS=()
declare -a VALS=()
declare -i VARCOUNT=0
declare    ALL="[--all--]"

declare -i NPOS=-1

function find_key
{
  local key="${1}"
  local -i pos=${NPOS}
  local -i idx=0

  while [ ${idx} -lt ${VARCOUNT} ] ; do
    if [ "${KEYS[${idx}]}" = "${key}" ] ; then
      pos=${idx}
      break
    else
      idx=$(( idx + 1 ))
    fi
  done

  echo ${pos}
}

function find_first_of
{
  local st="${1}"
  local ch="${2}"

  local -i pos=${NPOS}
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

function processFile
{
  local f="${1}"
  local section="${2}"
  local param="${3}"
  local line=""
  local saveIFS=${IFS}
  local currGroup=""
  local -i skipGroup=0
  local key=""
  local val=""
  local -i pos=0

  IFS="
"

  for line in $(cat ${f}) ; do
    line=${line// /}
    if [ -n "${line}" ] ; then
      if [ "${line:0:1}" != "#" ] ; then
        if [ "${line:0:1}" = "[" ] ; then
          skipGroup=0
          currGroup=${line//]/}
          currGroup=${currGroup//[/}
          if [ "${section}" != "${ALL}" ] ; then
            if [ "${section}" != "${currGroup}" ] ; then
              skipGroup=1
            fi
          fi
        elif [ ${skipGroup} -eq 0 ] ; then
          pos=$(find_first_of "${line}" "=")
          if [ ${pos} -ne ${NPOS} ] ; then
            key="${line:0:${pos}}"
            if [ "${param}" = "${ALL}" -o "${param}" = "${key}" ] ; then
              key="${currGroup}:${key}"
              pos=$(( pos + 1 ))
              val="${line:${pos}}"
              if [ ${FILECOUNT} -eq 1 ] ; then
                pos=${VARCOUNT}
              else
                pos=$(find_key "${key}")
                if [ ${pos} -eq ${NPOS} ] ; then
                  pos=${VARCOUNT}
                fi
              fi
              KEYS[${pos}]="${key}"
              VALS[${pos}]="${val}"
              if [ ${pos} -eq ${VARCOUNT} ] ; then
                VARCOUNT=$(( VARCOUNT + 1 ))
              fi
            fi
          fi
        fi
      fi
    fi
  done

  IFS=${saveIFS}
}

function main
{
  local -i i=0
  local    section=""
  local    param=""
  local -i showValueOnly=0

  while [ -n "${1}" ] ; do
    if [ "${1}" = "-s" ] ; then
      shift
      section="${1}"
    elif [ "${1}" = "-p" ] ; then
      shift
      param="${1}"
    elif [ "${1}" = "-ov" ] ; then
      showValueOnly=1
    elif [ -e "${1}" ] ; then
      FILES[${FILECOUNT}]="${1}"
      FILECOUNT=$(( FILECOUNT + 1 ))
    fi
    shift
  done

  if [ -z "${section}" ] ; then
    section="${ALL}"
  fi

  if [ -z "${param}" ] ; then
    param="${ALL}"
  fi

  i=0
  while [ ${i} -lt ${FILECOUNT} ] ; do
    processFile "${FILES[${i}]}" "${section}" "${param}"
    i=$(( $i + 1 ))
  done

  i=0
  while [ ${i} -lt ${VARCOUNT} ] ; do
    if [ ${showValueOnly} -eq 1 ] ; then
      echo "${VALS[${i}]}"
    else
      echo "${KEYS[${i}]} = ${VALS[${i}]}"
    fi
    i=$(( $i + 1 ))
  done
}

main $*
