#!/bin/bash

#------------------------------------------------------------------------------
# make sure I'm starting out in the directory where this script is located
#
  SCRIPT_PARENT_DIR=$( builtin cd -- "${0%/*}" ; builtin pwd -P )
  cd ${SCRIPT_PARENT_DIR}

#------------------------------------------------------------------------------
# location of the executable
#
  BUILDTYPE="${BUILD}"
  if [ -z "${BUILDTYPE}" ] ; then
     BUILDTYPE=debug
  fi
  VOB_DIR="$SCRIPT_PARENT_DIR/../.."
  RUNDIR=debug
  if [ -z "${SERVERNAME}" ] ; then
     SERVERNAME=tseserver
  fi
  EXE=${VOB_DIR}/bin/${BUILDTYPE}/${SERVERNAME}

#------------------------------------------------------------------------------
# location of the IOR files to be generated
#
  IOR="${HOME}/${SERVERNAME}.ior"
  APPCONSOLE_IOR="${HOME}/${SERVERNAME}_appconsole.ior"

  while getopts "i:c:dD:hr:p:b:n:s:a:m:lwg:" flag ; do
    case ${flag} in
      i) IOR="${HOME}/${SERVERNAME}$(echo ${OPTARG} | tr '[:lower:]' '[:upper:]').ior" ;
         rm -f ${IOR} ;
         APPCONSOLE_IOR="${HOME}/${SERVERNAME}_appconsole$(echo ${OPTARG} | tr '[:lower:]' '[:upper:]').ior" ;
         rm -f ${APPCONSOLE_IOR} ;;

      b) export PATH=${VOB_DIR}/deploy/integ/bin:${PATH} ;;
      c) if [ ! -s ${OPTARG} ] ; then
             echo server.sh ERROR: Config file ${OPTARG} is missing.
             exit 1
         fi
    esac
  done

  #echo ${IOR}

  echo 'server.sh: Setting build type to '${BUILDTYPE}
  echo 'server.sh: Setting iorfile to '${IOR}
  echo 'server.sh: Setting appconsole iorfile to '${APPCONSOLE_IOR}

#------------------------------------------------------------------------------
# libraries needed by the executable
#
  . ${VOB_DIR}/Server/set_paths.sh

#------------------------------------------------------------------------------
# Make sure bigipsvc.sh is in the PATH
#
  if [ -n "${RUNTIME_BIGIP_OPTIONS}" ] ; then
    if [ -e /opt/atse/common ] ; then
      export PATH="${PATH}:/opt/atse/common"
    elif [ -e /opt/atseintl/common ] ; then
      export PATH="${PATH}:/opt/atseintl/common"
    fi
  fi

#------------------------------------------------------------------------------
# this probably has something to do with oracle connections
export NLS_LANG=AMERICAN_AMERICA.AL32UTF8
export TNS_ADMIN=.

# use JE allocator on private servers
export TRX_ALLOCATOR=${TRX_ALLOCATOR:-JE}

#------------------------------------------------------------------------------
# this file is generated out by tseserver when its ready for requests
#
  export ATSE_READYFILE=${EXE}.ready

#------------------------------------------------------------------------------
# acms support
#
  if [ "${ACMS_SUPPORT}" == "Y" ] ; then
    ACMS_PARAMS="-app=${ACMS_APP_NAME}"
    if [ -n "${ACMS_BSL}" ]; then
      ACMS_PARAMS="${ACMS_PARAMS} -bsl=${ACMS_BSL}"
    fi
    if [ -n "${ACMS_USER}" ]; then
      ACMS_PARAMS="${ACMS_PARAMS} -user=${ACMS_USER}"
    fi
    if [ -n "${ACMS_DMN_ENV}" ]; then
      ACMS_PARAMS="${ACMS_PARAMS} -env=${ACMS_DMN_ENV}"
    fi
    if [ "${NO_ACMS_DOWNLOAD}" != "Y" ]; then
      if [ -n "${ACMS_ACT_LIST}" ]; then
        ${VOB_DIR}/Server/gettseservercfg.pl ${ACMS_PARAMS} -act="$ACMS_ACT_LIST"
      else
        ${VOB_DIR}/Server/gettseservercfg.pl ${ACMS_PARAMS}
      fi
    fi
  fi

#------------------------------------------------------------------------------
# set up arguments
#
  allargs=""
  allargs="${allargs} -D TO_MAN.IOR_FILE=$IOR"
  allargs="${allargs} -D APPLICATION_CONSOLE.IOR_FILE=${APPCONSOLE_IOR}"
  allargs="${allargs} ${RUNTIME_BIGIP_OPTIONS}"

  config_file="./tseserver.acms.cfg"
  if [ "${NO_ACMS_DOWNLOAD}" == "Y" ]; then
    if [ -n "${CONFIG_FILE}" ]; then
      config_file="${CONFIG_FILE}"
    fi
  fi

  if [ "${ACMS_SUPPORT}" == "Y" ]; then
    if [ ! -s "${config_file}" ] ; then
      echo server.sh ERROR: Config file ${config_file} is missing.
      exit 1
    fi
    allargs="${allargs} -c ${config_file}"

    if [ "${NO_ACMS_DOWNLOAD}" != "Y" ]; then
      if [ "${START_NON_HISTORICAL}" == "Y" ]; then
        sed -i 's/\s*ALLOW_HISTORICAL\s*=.*/ALLOW_HISTORICAL = N/' ${config_file}
        sed -i 's/\s*SERVER_TYPE\s*=.*/SERVER_TYPE = PRICING/' ${config_file}
      elif [ "${START_TAX}" == "Y" ]; then
        sed -i 's/\s*XFORM_NAME\s*=.*/XFORM_NAME = TAX_XFORM/' ${config_file}
        sed -i 's/\s*SERVER_TYPE\s*=.*/SERVER_TYPE = TAX/' ${config_file}
      elif [ "${START_FD}" == "Y" ]; then
        sed -i 's/\s*SERVER_TYPE\s*=.*/SERVER_TYPE = FAREDISPLAY/' ${config_file}
      fi
    fi
  fi

  if [ -e ./tseserver.cfg.node ] ; then
    allargs="${allargs} -D TSE_SERVER.OVERRIDE_CFGS=tseserver.cfg.node"
    if [ -e ./tseserver.cfg.user ] ; then
      allargs="${allargs}|tseserver.cfg.user"
    fi
  elif [ -e ./tseserver.cfg.user ] ; then
    allargs="${allargs} -D TSE_SERVER.OVERRIDE_CFGS=tseserver.cfg.user"
  fi

#------------------------------------------------------------------------------
# start it up
#
  if [ "${DEBUG_TSESERVER_GDB}" = "Y" ] ; then
    CMD="gdb -d $(readlink -f $(pwd)/../..) --args ${EXE} ${allargs}"
  elif [ "${DEBUG_TSESERVER_CGDB}" = "Y" ] ; then
    CMD="cgdb -- -d $(readlink -f $(pwd)/../..) --args ${EXE} ${allargs}"
  elif [ "${DEBUG_TSESERVER}" = "Y" ] ; then
    CMD="totalview ${EXE} -search_path ${VOB_DIR} -a ${allargs}"
  else
    CMD="${EXE} ${allargs}"
  fi

  echo "server.sh: Creating dummy appmon runctl file..."
  echo "APPMON_COMMAND=\"${CMD}\"" > .app.SERVICE.1.runctl

  if [ -e ./appmon_hook.sh ] ; then
    echo "server.sh: Executing appmon_hook.sh prestart..."
    export APPMON_COMMAND="${CMD}"
    ./appmon_hook.sh "prestart"
  fi

  echo "server.sh: Launching: ${CMD} $*"
  ${CMD} $*


