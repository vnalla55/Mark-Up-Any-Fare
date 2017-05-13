#!/bin/sh
# ----------------------------------------------------------------
#
#  bigipsvc.sh
#
#  BigIP registration/deregistration routines
#
# ----------------------------------------------------------------

# save the current directory of the one who called us - we will
# write temp and log files there

BIGIPSVC_CALLER_DIR=$(pwd)

# ensure we are running within the directory where
# this script is located
cd $(dirname $(which ${0}))

# set environment for finding additional perl libraries
export PERL5LIB=".:./perl:${PERL5LIB}"

# set path to find bigip.pl
export PATH=.:./perl:${PATH}

# set shell variables
BIGIPSVC_SCRIPTNAME=$(basename ${0})
BIGIPSVC_LOGFILE=${BIGIPSVC_CALLER_DIR}/${BIGIPSVC_SCRIPTNAME}.log
BIGIPSVC_TEMPFILE=${BIGIPSVC_CALLER_DIR}/${BIGIPSVC_SCRIPTNAME}.$$.tmp
BIGIPSVC_STDERR=${BIGIPSVC_CALLER_DIR}/${BIGIPSVC_SCRIPTNAME}.$$.stderr
BIGIPSVC_STDOUT=${BIGIPSVC_CALLER_DIR}/${BIGIPSVC_SCRIPTNAME}.$$.stdout
BIGIPSVC_LASTERROR=${BIGIPSVC_CALLER_DIR}/${BIGIPSVC_SCRIPTNAME}.lasterror
BIGIPSVC_OSFAMILY=$(uname -s)
BIGIPSVC_HARDWARE=$(uname -m)
BIGIPSVC_REDHAT_RELEASE=$(cat /etc/redhat-release | grep -o -E "release [0-9]*" | awk '{print $2}')
BIGIPSVC_PLATFORM="${BIGIPSVC_OSFAMILY}-GNU-${BIGIPSVC_HARDWARE}"
BIGIPSVC_USERID=$(whoami)
BIGIPSVC_NODE=$(hostname -s)
BIGIPSVC_MEMBER_NODE=""
BIGIPSVC_NODE_FULL=$(hostname -f)
BIGIPSVC_NODE_IP_ADDR=$(host ${BIGIPSVC_NODE_FULL} | awk '{print $4}')
BIGIPSVC_RC=0
BIGIPSVC_USE_ICONTROL=1
BIGIPSVC_BIGIP_PL="bigip.pl"
BIGIPSVC_HOSTSPEC=""
BIGIPSVC_CMD_PREFIX=""
BIGIPSVC_SSH_CMD="/usr/bin/ssh"
BIGIPSVC_SSH_OPTIONS="-q -o BatchMode=yes -o StrictHostKeyChecking=no"
BIGIPSVC_IS_CHECKPOINT_POOL=0
BIGIPSVC_REMOVE_ON_DEREG=0
BIGIPSVC_VERBOSE=0
BIGIPSVC_PREVIEW=0
BIGIPSVC_JUST_A_QUERY=0
BIGIPSVC_WAIT_NO_CONN=0
BIGIPSVC_SUPPRESS_SHOWCMD=0
BIGIPSVC_NO_SHOW_STAMP=0
BIGIPSVC_KEEP_LOG=0
declare -a -i BIGIPSVC_PORTS
declare -a -i BIGIPSVC_ACTUAL_PORTS

function mysay
{
  local logline=""
  local showline=""
  local stamp=""

  if [ ${BIGIPSVC_PREVIEW} -eq 1 ] ; then
    stamp="(PREVIEW) "
  fi
  stamp="${stamp}[${BIGIPSVC_NODE}] $(date '+%Y-%m-%d %H:%M:%S') - ${BIGIPSVC_SCRIPTNAME}[$$]: "

  if [ ${BIGIPSVC_NO_SHOW_STAMP} -eq 0 ] ; then
    showline="${stamp}${*}"
  else
    showline="${*}"
  fi
  echo ${showline}

  logline="${stamp}${*}"
  echo ${logline} >> ${BIGIPSVC_LOGFILE}
}

function exitError
{
  local errorCode=${1}
  shift
  mysay "ERROR ${errorCode}: $*"
  echo "${BIGIPSVC_SCRIPTNAME} [${errorCode}] - $*" > ${BIGIPSVC_LASTERROR}
  exit ${errorCode}
}

function say_transcript
{
  local line=""
  if [ -s ${BIGIPSVC_STDOUT} -o -s ${BIGIPSVC_STDERR} ] ; then
    mysay "+ ---------- BIGIP TRANSCRIPT ----------"
    if [ -s ${BIGIPSVC_STDOUT} ] ; then
      while read line ; do
        mysay "+ stdout: ${line}"
      done < ${BIGIPSVC_STDOUT}
    fi
    if [ -s ${BIGIPSVC_STDERR} ] ; then
      while read line ; do
        mysay "+ stderr: ${line}"
      done < ${BIGIPSVC_STDERR}
    fi
    mysay "+ ------- END OF BIGIP TRANSCRIPT ------"
  fi
}

function do_bigip_command
{
  local rc=0
  local line=""

  rm -f ${BIGIPSVC_STDOUT} ${BIGIPSVC_STDERR} > /dev/null 2>&1 < /dev/null

  if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
    if [ ${BIGIPSVC_SUPPRESS_SHOWCMD} -eq 0 ] ; then
      mysay "BigIP command: ${*}"
    else
      BIGIPSVC_SUPPRESS_SHOWCMD=0
    fi
  fi

  if [ ${BIGIPSVC_JUST_A_QUERY} -eq 1 -o ${BIGIPSVC_PREVIEW} -eq 0 ] ; then
    ${*} 1> ${BIGIPSVC_STDOUT} 2> ${BIGIPSVC_STDERR} </dev/null
    rc=$?
  else
    rc=0
  fi
  BIGIPSVC_JUST_A_QUERY=0

  if [ ${rc} -ne 0 ] ; then
    mysay "Bigip command returned non-zero exit code [${rc}] !!!"
    if [ -s ${BIGIPSVC_STDOUT} -o -s ${BIGIPSVC_STDERR} ] ; then
      say_transcript
    else
      mysay "No further BigIP information available."
    fi
  elif [ ${BIGIPSVC_USE_ICONTROL} -eq 0 ] ; then
    if [ -s ${BIGIPSVC_STDERR} ] ; then
      say_transcript
      rc=1
    fi
  fi

  return ${rc}
}

function verify_pool_name
{
  local poolname=${1}
  local cmd=${BIGIPSVC_CMD_PREFIX}
  local rc=0

  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
    cmd="${cmd} verifyPoolExists ${poolname}"
  else
    cmd="${cmd} pool ${poolname} lb_method show"
  fi

  BIGIPSVC_JUST_A_QUERY=1
  do_bigip_command ${cmd}
  rc=$?

  if [ ${rc} -eq 0 ] ; then
    if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
      mysay "Pool [${poolname}] verified successfully."
    fi
  else
    mysay "Pool [${poolname}] could not be verified!"
  fi

  return ${rc}
}

function verify_monitor_template
{
  local name=${1}
  local cmd=${BIGIPSVC_CMD_PREFIX}
  local rc=0

  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
    cmd="${cmd} verifyMonitorTemplateExists ${name}"
  else
    cmd="${cmd} monitor ${name} show"
  fi

  BIGIPSVC_JUST_A_QUERY=1
  do_bigip_command ${cmd}
  rc=$?

  if [ ${rc} -eq 0 ] ; then
    if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
      mysay "Monitor template [${name}] was found."
    fi
  else
    mysay "Monitor template [${name}] was not found and wll be created."
    rc=0

    cmd=${BIGIPSVC_CMD_PREFIX}

    if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
      if [ ${BIGIPSVC_IS_CHECKPOINT_POOL} -eq 0 ] ; then
        cmd="${cmd} createMonitorTemplateFromTCP ${name}"
      else
        cmd="${cmd} createMonitorTemplateFromTCP ${name} 60 181"
      fi
    else
      if [ ${BIGIPSVC_IS_CHECKPOINT_POOL} -eq 0 ] ; then
        cmd="${cmd} monitor ${name} '{ use tcp }'"
      else
        cmd="${cmd} monitor ${name} '{ use tcp interval 60 timeout 181 }'"
      fi
    fi

    do_bigip_command ${cmd}
    rc=$?

    if [ ${rc} -eq 0 ] ; then
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Monitor template [${name}] was created."
      fi
    else
      mysay "Coult not create monitor template [${name}]!"
    fi

  fi

  return ${rc}
}

function get_actual_ports
{
  local poolname=${1}
  local cmd=${BIGIPSVC_CMD_PREFIX}
  local token=""
  local host=""
  local port=""
  local -i rc=0
  local -i portidx=0
  local -i found=0
  local -i actual_count=0

  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
    cmd="${cmd} listPool ${poolname}"
  else
    cmd="${cmd} pool ${poolname} member show"
  fi

  BIGIPSVC_JUST_A_QUERY=1
  do_bigip_command ${cmd}
  rc=$?

  if [ ${rc} -eq 0 ] ; then
    actual_count=0
    if [ -s ${BIGIPSVC_STDOUT} ] ; then
      for token in $(cat ${BIGIPSVC_STDOUT}) ; do
        case "${token}" in
          "pool"|"${poolname}"|"members") ;;
          *) host=$(echo ${token} | awk -F: '{print $1}')
             if [ "${host}" = "${BIGIPSVC_MEMBER_NODE}" ] ; then
               port=$(echo ${token} | awk -F: '{print $2}')
               found=0
               portidx=0
               while [ ${portidx} -lt ${#BIGIPSVC_PORTS[*]} ] ; do
                 if [ "${BIGIPSVC_PORTS[${portidx}]}" = "${port}" ] ; then
                   found=1
                   break
                 fi
                 portidx=$(expr ${portidx} + 1)
               done
               if [ ${found} -eq 1 ] ; then
                 BIGIPSVC_ACTUAL_PORTS[${actual_count}]=${port}
                 actual_count=$(expr ${actual_count} + 1)
               fi
             fi
             ;;
        esac
      done
    fi
    if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
      mysay "Actual ports retrieved from BigIP for pool [${poolname}].  Count=${#BIGIPSVC_ACTUAL_PORTS[*]}"
    fi
  else
    mysay "Unable to retrieve actual ports from BigIP for pool [${poolname}]!"
  fi

  return ${rc}
}

function register
{
  local bigip_host=${1}
  local poolname=${2}
  local -i connlmit=${3}

  local bigip_member_list=""
  local member=""
  local cmd=${BIGIPSVC_CMD_PREFIX}
  local curr_monitor=""
  local -i idx=0
  local -i rc=0
  local -i priority=0

  # make sure our pool is defined in BigIP

  verify_pool_name ${poolname}
  rc=$?

  # get the actual list of ports for this pool from BigIP

  if [ ${rc} -eq 0 ] ; then
    get_actual_ports ${poolname}
    rc=$?
  fi

  # figure out which ports actually need to be added, then add them

  if [ ${rc} -eq 0 ] ; then
    idx=0
    while [ ${idx} -lt ${#BIGIPSVC_PORTS[*]} ] ; do
      priority=$(expr ${priority} + 1)
      local curr_port=${BIGIPSVC_PORTS[${idx}]}
      local a_idx=0
      local a_found=0
      while [ ${a_idx} -lt ${#BIGIPSVC_ACTUAL_PORTS[*]} ] ; do
        if [ ${curr_port} -eq ${BIGIPSVC_ACTUAL_PORTS[${a_idx}]} ] ; then
          a_found=1
          break
        fi
        a_idx=$(expr ${a_idx} + 1)
      done
      if [ ${a_found} -eq 0 ] ; then
        if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
          bigip_member_list="${bigip_member_list}${BIGIPSVC_MEMBER_NODE}:${curr_port} "
        else
          bigip_member_list="${bigip_member_list}member ${BIGIPSVC_MEMBER_NODE}:${curr_port} priority ${priority} "
        fi
      fi
      idx=$(expr ${idx} + 1)
    done

    if [ -n "${bigip_member_list}" ] ; then
      if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
        cmd="${cmd} addNodesToPool ${poolname} ${bigip_member_list}"
      else
        cmd="${cmd} pool ${poolname} add {${bigip_member_list}}"
      fi

      do_bigip_command ${cmd}
      rc=$?

      if [ ${rc} -eq 0 ] ; then
        if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
          mysay "Ports added successfully."
        fi
      else
        mysay "Ports could not be added!"
      fi
    else
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Don't need to add any ports because they already exist."
      fi
    fi
  fi

  # build the complete list of members

  bigip_member_list=""
  if [ ${rc} -eq 0 ] ; then
    idx=0
    while [ ${idx} -lt ${#BIGIPSVC_PORTS[*]} ] ; do
      local curr_port=${BIGIPSVC_PORTS[${idx}]}
      bigip_member_list="${bigip_member_list}${BIGIPSVC_MEMBER_NODE}:${curr_port} "
      idx=$(expr ${idx} + 1)
    done
  fi

  # make sure their conn limits are set

  if [ ${rc} -eq 0 -a -n "${bigip_member_list}" ] ; then
    cmd=${BIGIPSVC_CMD_PREFIX}
    if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
      cmd="${cmd} changeNodeConnectionLimit ${connlimit} ${bigip_member_list}"
    else
      cmd="${cmd} node ${bigip_member_list} limit ${connlimit}"
    fi

    do_bigip_command ${cmd}
    rc=$?

    if [ ${rc} -eq 0 ] ; then
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Connection limit set to [${connlimit}] successfully."
      fi
    else
      mysay "Connection limit could not be set to [${connlimit}]!"
    fi
  fi

  # make sure they are enabled

  if [ ${rc} -eq 0 -a -n "${bigip_member_list}" ] ; then
    cmd=${BIGIPSVC_CMD_PREFIX}
    if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
      cmd="${cmd} enableNodes ${bigip_member_list}"
    else
      cmd="${cmd} node ${bigip_member_list} enable"
    fi

    do_bigip_command ${cmd}
    rc=$?

    if [ ${rc} -eq 0 ] ; then
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Ports enabled successfully."
      fi
    else
      mysay "Ports could not be enabled!"
    fi
  fi

  # make sure their priorities are properly set (if iControl interface)

  if [ ${rc} -eq 0 -a -n "${bigip_member_list}" -a ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
    cmd="${BIGIPSVC_CMD_PREFIX} setPoolAscendingNodePriorities ${poolname} 1 ${bigip_member_list}"

    do_bigip_command ${cmd}
    rc=$?

    if [ ${rc} -eq 0 ] ; then
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Node priorities updated in ascending order."
      fi
    else
      mysay "Node priorities could not be updated!"
    fi
  fi

  # see if there are monitors for them, and print warnings if not.

  if [ ${rc} -eq 0 ] ; then
    for member in $(echo "${bigip_member_list}") ; do
      cmd=${BIGIPSVC_CMD_PREFIX}
      if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
        cmd="${cmd} listMonitorAssociationOfNodes ${member}"
      else
        cmd="${cmd} node ${member} monitor show"
      fi

      BIGIPSVC_JUST_A_QUERY=1
      do_bigip_command ${cmd}
      rc=$?

      if [ ${rc} -eq 0 -a -s ${BIGIPSVC_STDOUT} ] ; then

        if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
          curr_monitor=$(cat ${BIGIPSVC_STDOUT})
        else
          curr_monitor=$(cat ${BIGIPSVC_STDOUT} | grep -E '^MONITOR ' | awk '{print $2}')
        fi

        if [ "${curr_monitor}" = "tcp" ] ; then
          mysay "WARNING: Monitor association for [${member}] uses default tcp template!  This will create problems if this is a \"Switchable\" application!"
        else
          if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
            mysay "Monitor associated with [${member}] is [${curr_monitor}]."
          fi
        fi

      elif [ ${rc} -ne 0 -o ! -s ${BIGIPSVC_STDOUT} ] ; then

        mysay "WARNING: No monitor association for [${member}] was found!"

      fi

      rc=0

    done
  fi

  return ${rc}
}

function deregister
{
  local bigip_host=${1}
  local poolname=${2}
  local -i connlmit=${3}
  local rc=0
  local bigip_member_list=""
  local cmd=${BIGIPSVC_CMD_PREFIX}

  # make sure our pool is defined in BigIP

  verify_pool_name ${poolname}
  rc=$?

  # get the actual list of ports for this pool from BigIP

  if [ ${rc} -eq 0 ] ; then
    get_actual_ports ${poolname}
    rc=$?
  fi

  # figure out which ports actually need to be disabled, then disabled them

  if [ ${rc} -eq 0 ] ; then
    idx=0
    while [ ${idx} -lt ${#BIGIPSVC_PORTS[*]} ] ; do
      local curr_port=${BIGIPSVC_PORTS[${idx}]}
      local a_idx=0
      local a_found=0
      while [ ${a_idx} -lt ${#BIGIPSVC_ACTUAL_PORTS[*]} ] ; do
        if [ ${curr_port} -eq ${BIGIPSVC_ACTUAL_PORTS[${a_idx}]} ] ; then
          a_found=1
          break
        fi
        a_idx=$(expr ${a_idx} + 1)
      done
      if [ ${a_found} -eq 1 ] ; then
        bigip_member_list="${bigip_member_list}${BIGIPSVC_MEMBER_NODE}:${curr_port} "
      fi
      idx=$(expr ${idx} + 1)
    done

    if [ -n "${bigip_member_list}" ] ; then
      if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
        cmd="${cmd} disableNodes ${bigip_member_list}"
      else
        cmd="${cmd} node ${bigip_member_list} disable"
      fi

      do_bigip_command ${cmd}
      rc=$?

      if [ ${rc} -eq 0 ] ; then
        if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
          mysay "Ports disabled successfully."
        fi
      else
        mysay "Ports could not be disabled!"
      fi
    else
      if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
        mysay "Don't need to disable any ports because they don't exist."
      fi
    fi
  fi

  # wait for connections to drop to zero (if option was specified)
  if [ ${rc} -eq 0 -a ${BIGIPSVC_WAIT_NO_CONN} -gt 0 ] ; then
    set -o noglob
    idx=0
    bigip_member_list=""
    while [ ${idx} -lt ${#BIGIPSVC_ACTUAL_PORTS[*]} ] ; do
      local curr_port=${BIGIPSVC_PORTS[${idx}]}
      bigip_member_list="${bigip_member_list}${BIGIPSVC_MEMBER_NODE}:${curr_port} "
      idx=$(expr ${idx} + 1)
    done

    if [ -n "${bigip_member_list}" ] ; then
      mysay "Waiting up to ${BIGIPSVC_WAIT_NO_CONN} seconds for pool member connections to quiesce..."
      local some_conns_in_progress=1
      local total_secs=0
      while [ ${some_conns_in_progress} -ne 0 ] ; do
        sleep 1
        total_secs=$(expr ${total_secs} + 1)

        if [ ${total_secs} -gt ${BIGIPSVC_WAIT_NO_CONN} ] ; then
          mysay "WARNING: Timed out while waiting for pool member connections to quiesce!"
          break
        fi

        some_conns_in_progress=0
        cmd=${BIGIPSVC_CMD_PREFIX}
        if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
          cmd="${cmd} getCurrentConnectionsFromPoolMembers ${poolname} ${bigip_member_list}"
        else
          cmd="${cmd} node ${bigip_member_list} show"
        fi
        if [ ${total_secs} -gt 1 ] ; then
          BIGIPSVC_SUPPRESS_SHOWCMD=1
        fi
        do_bigip_command ${cmd}
        rc=$?
        if [ ${rc} -eq 0 -a -s ${BIGIPSVC_STDOUT} ] ; then
          local token=""
          local fieldnum=1
          if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
            #
            # EXAMPLE:
            #
            # 0 10.19.51.15:53501 0 10.19.51.15:53502 0 10.19.51.15:53503
            #
            for token in $(cat ${BIGIPSVC_STDOUT}) ; do
              if [ ${fieldnum} -eq 1 ] ; then
                if [ ${token} -gt 0 ] ; then
                  some_conns_in_progress=1
                  break
                fi
                fieldnum=2
              else
                fieldnum=1
              fi
            done
          else
            #
            # EXAMPLE:
            #
            # NODE 10.19.51.15          UP             CHECKED
            # |        gateway = 172.28.139.1
            # |        gateway ether = 00:01:96:23:d7:21
            # |        VLAN = poss_lab
            # |        (cur, max, limit, tot) = (0, 0, 0, 0)
            # |        (pckts,bits) in = (0, 0), out = (0, 0)
            # +-    SERVICE 53501       DOWN           CHECKED
            #          (cur, max, limit, tot) = (0, 0, 0, 0)
            #          (pckts,bits) in = (0, 0), out = (0, 0)
            #
            # NODE *                    UNCHECKED
            # |        (cur, max, limit, tot) = (0, 0, 0, 0)
            # |        (pckts,bits) in = (0, 0), out = (2, 0)
            # +-    SERVICE 53502       DOWN           CHECKED
            #          (cur, max, limit, tot) = (0, 0, 0, 0)
            #          (pckts,bits) in = (0, 0), out = (0, 0)
            #
            # NODE *                    UNCHECKED
            # |        (cur, max, limit, tot) = (0, 0, 0, 0)
            # |        (pckts,bits) in = (0, 0), out = (0, 0)
            # +-    SERVICE 53503       DOWN           CHECKED
            #          (cur, max, limit, tot) = (0, 0, 0, 0)
            #          (pckts,bits) in = (0, 0), out = (0, 0)
            #
            for token in $(cat ${BIGIPSVC_STDOUT} | grep '(cur, max, limit, tot)' \
                                                  | grep -v '|'                   \
                                                  | awk -F'=' '{print $2}'        \
                                                  | sed s/\(//                    \
                                                  | awk -F',' '{print $1}'        \
                          ) ; do
              if [ ${token} -gt 0 ] ; then
                some_conns_in_progress=1
                break
              fi
            done
          fi
        fi
      done
    fi
    set +o noglob
  fi

  # now remove them

  if [ ${rc} -eq 0 -a ${BIGIPSVC_REMOVE_ON_DEREG} -eq 1 ] ; then
    idx=0
    bigip_member_list=""
    cmd=${BIGIPSVC_CMD_PREFIX}
    while [ ${idx} -lt ${#BIGIPSVC_ACTUAL_PORTS[*]} ] ; do
      local curr_port=${BIGIPSVC_PORTS[${idx}]}
      if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
        bigip_member_list="${bigip_member_list}${BIGIPSVC_MEMBER_NODE}:${curr_port} "
      else
        bigip_member_list="${bigip_member_list}member ${BIGIPSVC_MEMBER_NODE}:${curr_port} "
      fi
      idx=$(expr ${idx} + 1)
    done

    if [ -n "${bigip_member_list}" ] ; then

      if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
        cmd="${cmd} deleteNodesFromPool ${poolname} ${bigip_member_list}"
      else
        cmd="${cmd} pool ${poolname} delete {${bigip_member_list}}"
      fi

      do_bigip_command ${cmd}
      rc=$?

      if [ ${rc} -eq 0 ] ; then
        if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
          mysay "Ports removed successfully."
        fi
      else
        mysay "Ports could not be removed!"
      fi
    fi
  fi

  return ${rc}
}

function revert_to_ssh
{
  local why=${1}
  BIGIPSVC_USE_ICONTROL=0
  mysay "WARNING: Unable to use iControl interface to BigIP.  Reason: ${why}."
  mysay "         Will attempt using deprecated SSH/bigpipe interface."
}

function usage
{
  local cv=""
  local env=""

  echo ""
  echo "Usage: ${BIGIPSVC_SCRIPTNAME} [options] <systype> <bigip_host> <action> <poolname> <port> [<port>...]"
  echo ""
  echo "  where <systype>     is one of the following:"
  echo "                        ProdHybrid"
  echo "                        CertHybrid"
  echo "                        TestHybrid"
  echo "                        DevHybrid"
  for cv in $(find . -maxdepth 1 -type f -name 'config.systype.*' 2> /dev/null) ; do
    env="${cv##*.}"
    case "${env}" in
      "ProdHybrid"|"CertHybrid"|"TestHybrid"|"DevHybrid") ;;
      *) echo "                        ${env}"
    esac
  done
  echo "    and <bigip_host>  is the address of the BigIP host"
  echo "    and <action>      is one of the following:"
  echo "                        register"
  echo "                        deregister"
  echo "    and <poolname>    is the pool to operate upon"
  echo "    and <port>        is a port to operate upon"
  echo ""
  echo "  [options] may be one or more of the following:"
  echo ""
  echo "    -connlimit <n>    set connection limit upon registration"
  echo "    -h                show usage information"
  echo "    -help             show usage information"
  echo "    -keeplog          keep previous log file intact - do not truncate"
  echo "    -logfile <file>   override log file path/name with <file>"
  echo "    -node <name>      override member node with <name>"
  echo "    -noshowstamp      do not include time/date/header info in stdout"
  echo "    -preview          run in preview mode (don't actually"
  echo "                      change BigIP configuration)"
  echo "    -remove           remove port(s) upon deregistration"
  echo "                      (otherwise they are simply disabled)"
  echo "    -ssh              force usage of SSH (instead of iControl)"
  echo "    -v                verbose output"
  echo "    -verbose          verbose output"
  echo "    -waitnoconn[:n]   if deregistering, wait for all connections"
  echo "                      to drop to zero before returning; timeout"
  echo "                      after <n> seconds (default is 10)"
  echo ""
}

function main
{
  local systype=""
  local bigip_host=""
  local action=""
  local poolname=""
  local pools=""
  local thisPool=""
  local tempstr=""
  local save_cmd_args="${*}"
  local -i portcount=0
  local -i tempnum=0
  local -i rc=0
  local -i pool_rc=0
  local -i forceSSH=0
  local -i connlimit=0
  local arg=""

  while [ -n "${1}" ] ; do
    arg="${1}"
    if [ "${arg:0:1}" = "-" ] ; then
      if [ "${arg:0:12}" = "-waitnoconn:" ] ; then
        BIGIPSVC_WAIT_NO_CONN="${arg:12}"
      else
        case "${arg}" in
          "-connlimit"    ) shift ; connlimit=${1}            ;;
          "-h"|"-help"    ) usage ; exit 0                    ;;
          "-keeplog"      ) BIGIPSVC_KEEP_LOG=1               ;;
          "-logfile"      ) shift ; BIGIPSVC_LOGFILE=${1}     ;;
          "-node"         ) shift ; BIGIPSVC_MEMBER_NODE=${1} ;;
          "-noshowstamp"  ) BIGIPSVC_NO_SHOW_STAMP=1          ;;
          "-preview"      ) BIGIPSVC_PREVIEW=1                ;;
          "-remove"       ) BIGIPSVC_REMOVE_ON_DEREG=1        ;;
          "-ssh"          ) forceSSH=1                        ;;
          "-v"|"-verbose" ) BIGIPSVC_VERBOSE=1                ;;
          "-waitnoconn"   ) BIGIPSVC_WAIT_NO_CONN=10          ;;
          *               ) mysay "NOTE: Unrecognized switch [${arg}] ignored." ;;
        esac
      fi
    elif [ -z "${systype}"    ] ; then systype="${arg}"
    elif [ -z "${bigip_host}" ] ; then bigip_host=$(echo "${arg}" | tr "[:upper:]" "[:lower:]")
    elif [ -z "${action}"     ] ; then action=$(echo "${arg}" | tr "[:upper:]" "[:lower:]")
    elif [ -z "${poolname}"   ] ; then poolname="${arg}"
    else
      BIGIPSVC_PORTS[${portcount}]=${arg}
      portcount=$(expr ${portcount} + 1)
    fi
    shift
  done

  if [ ${BIGIPSVC_KEEP_LOG} -eq 0 ] ; then
    rm -f ${BIGIPSVC_LOGFILE} > /dev/null 2>&1 < /dev/null
  fi

  # validate systype

  if [ -z "${systype}" ] ; then
    exitError 7 "No systype specified."
  fi

  # validate bigip_host

  if [ -z "${bigip_host}" ] ; then
    usage
    exitError 1 "No BigIP host specified."
  else
    tempstr=$(host ${bigip_host} 2> /dev/null < /dev/null)
    if [ -z "${tempstr}" ] ; then
      usage
      exitError 2 "Specified BigIP host [${bigip_host}] was not found."
    fi
  fi
  local sec_host=${bigip_host##*@}

  # validate action

  case "${action}" in
    "register"|"deregister") ;;
    "") usage ; exitError 3 "No action specified." ;;
    *) usage ; exitError 3 "Invalid action [${action}]." ;;
  esac

  # validate poolname

  if [ -z "${poolname}" ] ; then
    usage
    exitError 4 "No pool name specified."
  fi

  # validate ports

  if [ ${portcount} -eq 0 ] ; then
    usage
    exitError 5 "No ports specified."
  else
    tempnum=0
    while [ ${tempnum} -lt ${portcount} ] ; do
      tempstr=$(echo ${BIGIPSVC_PORTS[${tempnum}]} | grep -E ^[0-9]*$ 2> /dev/null)
      if [ -z "${tempstr}" ] ; then
        exitError 6 "Port [${BIGIPSVC_PORTS[${tempnum}]}] is not numeric."
      fi
      tempnum=$(expr ${tempnum} + 1)
    done
  fi

  # validate connection limit
  tempstr=$(echo ${connlimit} | grep -E ^[0-9]*$ 2> /dev/null)
  if [ -z "${tempstr}" ] ; then
    exitError 15 "Connection limit [${connlimit}] is not numeric."
  fi

  # figure out whether we will use iControl or SSH

  if [ ${forceSSH} -eq 1 ] ; then
    revert_to_ssh "Requested by user"
  fi

  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then

    /usr/bin/perl -MLWP::Protocol::https -le'print LWP::Protocol::https::Socket->can("new")' >/dev/null 2>&1 </dev/null
    tempnum=$?
    if [ ${tempnum} -ne 0 ] ; then
      revert_to_ssh "SOAP::Lite prerequisites are not installed"
    fi

  fi

  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then

    # we may not necessarily need a host file, if the $bigip_host variable is
    # set up with BOTH userid AND password, so see if that's the case
    # before we get too fancy

    local uid=""
    local psswrd=""
    local uidp=$(echo ${bigip_host} | awk -F@ '{print $1}')

    if [ "${uidp}" != "${sec_host}" ] ; then
      # there was a '@' delimiter, so try to find a userid or password.
      # With the bigip.pl script, password must be present if userid is specified.
      uid=$(echo ${uidp} | awk -F: '{print $1}')
      psswrd=$(echo ${uidp} | awk -F: '{print $2}')
    fi

    if [ -n "${uid}" -a -n "${psswrd}" ] ; then
      BIGIPSVC_HOSTSPEC="${uid}:${psswrd}@"
    fi

    if [ -z "${psswrd}" ] ; then

      if [ -e ~/.bigip/hosts/${sec_host} ] ; then
        # if we have a key file, do an authorization check to make sure it
        # hasn't be come invalid; if it has, then we'll delete it so we
        # can *attempt* pull a new one down in the next step
        do_bigip_command "${BIGIPSVC_BIGIP_PL} ${sec_host} new"
        rc=$?
        if [ ${rc} -ne 0 ] ; then
          mysay "WARNING: Keyfile [~/.bigip/hosts/${sec_host}] is no longer valid. Will attempt to update from admin box."
          rm -f ~/.bigip/hosts/${sec_host}
        fi
      fi

      if [ ! -e ~/.bigip/hosts/${sec_host} ] ; then

        local admin_box=""
        local admin_user=""

        case ${systype} in
          ProdHybrid) admin_box=pinhpa02.sabre.com   ; admin_user=hybfunc ;;
          CertHybrid) admin_box=pinlc101.sabre.com   ; admin_user=hybfunc ;;
          TestHybrid) admin_box=pinli001.sabre.com   ; admin_user=snpfunc ;;
          DevHybrid ) admin_box=hybd05.dev.sabre.com ; admin_user=snpfunc ;;
        esac

        if [ -e ./config.systype.${systype} ] ; then
          . ./config.systype.${systype}
          if [ -n "${COMMON_ADMIN_NODE}" -a -n "${COMMON_DOMAIN}" ] ; then
            admin_box="${COMMON_ADMIN_NODE}.${COMMON_DOMAIN}"
            admin_user=${COMMON_ADMIN_USER}
          fi
        fi

        if [ -n "${admin_box}" ] ; then

          mkdir -p ~/.bigip/hosts

          local sshtest_script=""
          if [ -x ./sshtest ] ; then
            sshtest_script="./sshtest"
          elif [ -x ../nodeagent/ext/sshtest ] ; then
            sshtest_script="../nodeagent/ext/sshtest"
          elif [ -x /opt/atse/nodeagent/ext/sshtest ] ; then
            sshtest_script="/opt/atse/nodeagent/ext/sshtest"
          fi

          if [ -n "${sshtest_script}" ] ; then
            local sshtest_command="${sshtest_script} -user=${BIGIPSVC_USERID} ${admin_box}"
            ${sshtest_command}
            rc=$?
            if [ ${rc} -eq 0 ] ; then
              local scp_command="scp -q ${BIGIPSVC_USERID}@${admin_box}:~${admin_user}/.bigip/hosts/${sec_host} ~/.bigip/hosts/${sec_host}"
              eval "${scp_command} 2> /dev/null < /dev/null"
              rc=$?
              if [ ${rc} -ne 0 ] ; then
                revert_to_ssh "Bad exit code [${rc}] returned from scp command: ${scp_command}"
              fi
            else
              revert_to_ssh "Bad exit code [${rc}] returned from sshtest command: ${sshtest_command}"
            fi
          else
            revert to ssh "Script ${sshtest_script} does not exist or is not executable."
          fi
        else
          revert to ssh "No admin box in this environment [${systype}]"
        fi
      fi
    fi
  fi

  # set up some COMMON variables

  if [ "$(echo ${poolname} | grep -o -E _ckpt\$)" = "_ckpt" ] ; then
    BIGIPSVC_IS_CHECKPOINT_POOL=1
  fi

  BIGIPSVC_HOSTSPEC="${BIGIPSVC_HOSTSPEC}${sec_host}"
  if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
    BIGIPSVC_CMD_PREFIX="${BIGIPSVC_BIGIP_PL} ${BIGIPSVC_HOSTSPEC}"
  else
    BIGIPSVC_CMD_PREFIX="${BIGIPSVC_SSH_CMD} ${BIGIPSVC_SSH_OPTIONS} ${BIGIPSVC_HOSTSPEC} b"
  fi

  if [ -z "${BIGIPSVC_MEMBER_NODE}" ] ; then
    if [ ${BIGIPSVC_USE_ICONTROL} -eq 1 ] ; then
      BIGIPSVC_MEMBER_NODE=${BIGIPSVC_NODE_IP_ADDR}
    elif [ "${bigip_host}" = "redball.dev.sabre.com" ] ; then
      BIGIPSVC_MEMBER_NODE=${BIGIPSVC_NODE_IP_ADDR}
    else
      BIGIPSVC_MEMBER_NODE=${BIGIPSVC_NODE}
    fi
  fi

  # give some info to the user, if verbose

  if [ ${BIGIPSVC_VERBOSE} -eq 1 ] ; then
    mysay "Script location: $(dirname $(which ${0}))"
    mysay "Arguments: ${save_cmd_args}"
    mysay "BIGIPSVC_SCRIPTNAME         = ${BIGIPSVC_SCRIPTNAME}"
    mysay "BIGIPSVC_LOGFILE            = ${BIGIPSVC_LOGFILE}"
    mysay "BIGIPSVC_TEMPFILE           = ${BIGIPSVC_TEMPFILE}"
    mysay "BIGIPSVC_STDERR             = ${BIGIPSVC_STDERR}"
    mysay "BIGIPSVC_STDOUT             = ${BIGIPSVC_STDOUT}"
    mysay "BIGIPSVC_LASTERROR          = ${BIGIPSVC_LASTERROR}"
    mysay "BIGIPSVC_OSFAMILY           = ${BIGIPSVC_OSFAMILY}"
    mysay "BIGIPSVC_HARDWARE           = ${BIGIPSVC_HARDWARE}"
    mysay "BIGIPSVC_REDHAT_RELEASE     = ${BIGIPSVC_REDHAT_RELEASE}"
    mysay "BIGIPSVC_PLATFORM           = ${BIGIPSVC_PLATFORM}"
    mysay "BIGIPSVC_USERID             = ${BIGIPSVC_USERID}"
    mysay "BIGIPSVC_NODE               = ${BIGIPSVC_NODE}"
    mysay "BIGIPSVC_MEMBER_NODE        = ${BIGIPSVC_MEMBER_NODE}"
    mysay "BIGIPSVC_NODE_FULL          = ${BIGIPSVC_NODE_FULL}"
    mysay "BIGIPSVC_NODE_IP_ADDR       = ${BIGIPSVC_NODE_IP_ADDR}"
    mysay "BIGIPSVC_RC                 = ${BIGIPSVC_RC}"
    mysay "BIGIPSVC_USE_ICONTROL       = ${BIGIPSVC_USE_ICONTROL}"
    mysay "BIGIPSVC_BIGIP_PL           = ${BIGIPSVC_BIGIP_PL}"
    mysay "BIGIPSVC_HOSTSPEC           = ${BIGIPSVC_HOSTSPEC}"
    mysay "BIGIPSVC_CMD_PREFIX         = ${BIGIPSVC_CMD_PREFIX}"
    mysay "BIGIPSVC_SSH_CMD            = ${BIGIPSVC_SSH_CMD}"
    mysay "BIGIPSVC_SSH_OPTIONS        = ${BIGIPSVC_SSH_OPTIONS}"
    mysay "BIGIPSVC_IS_CHECKPOINT_POOL = ${BIGIPSVC_IS_CHECKPOINT_POOL}"
    mysay "BIGIPSVC_REMOVE_ON_DEREG    = ${BIGIPSVC_REMOVE_ON_DEREG}"
    mysay "BIGIPSVC_VERBOSE            = ${BIGIPSVC_VERBOSE}"
    mysay "BIGIPSVC_PREVIEW            = ${BIGIPSVC_PREVIEW}"
    mysay "BIGIPSVC_SUPPRESS_SHOWCMD   = ${BIGIPSVC_SUPPRESS_SHOWCMD}"
    mysay "BIGIPSVC_NO_SHOW_STAMP      = ${BIGIPSVC_NO_SHOW_STAMP}"
    mysay "BIGIPSVC_KEEP_LOG           = ${BIGIPSVC_KEEP_LOG}"
    mysay "PATH                        = ${PATH}"
  fi

  # now do what we're told to do

  if [ -e ./perl/sh_tokenizer.pl ] ; then
    pools=$(./perl/sh_tokenizer.pl " :" "${poolname}")
  elif [ -e ./sh_tokenizer.pl ] ; then
    pools=$(./sh_tokenizer.pl " :" "${poolname}")
  else
    pools=${poolname}
  fi

  for thisPool in ${pools} ; do
    tempstr="${action} ${bigip_host} ${thisPool} ${connlimit}"
    mysay "Function: ${tempstr}"
    eval ${tempstr}
    pool_rc=$?
    if [ ${pool_rc} -ne 0 ] ; then
      BIGIPSVC_RC=$?
    fi
  done

  # clean up
  rm -rf ${BIGIPSVC_STDOUT} ${BIGIPSVC_STDERR} > /dev/null 2>&1 < /dev/null
}

main ${*}
mysay "Exit code: ${BIGIPSVC_RC}"
exit ${BIGIPSVC_RC}


