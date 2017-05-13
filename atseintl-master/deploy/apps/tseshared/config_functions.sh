#!/bin/sh

###
### config_functions.sh
###
### Intended to be sourced in by config.vars and other configuration scripts
### for tseserver-based apoplications.
###

if [ -z "${COMMON_NODE}" ] ; then
  . ./set.atse.common.sh
  . ${ATSE_COMMON}/config.vars
fi

function init_vars
{
  local appname=$(echo ${COMMON_APPNAME} | tr "[:lower:]" "[:upper:]")
  eval PRIMARY_CFG=\$ADMIN_${appname}_PRIMARY_CFG
  if [ -z "${PRIMARY_CFG}" ] ; then
    PRIMARY_CFG="tseserver.acms.cfg"
  fi
  PRIMARY_CFG="-c ./${PRIMARY_CFG}"
  SECONDARY_CFG=""
  NUMBER_OF_SECONDARIES=0
}

function add_secondary_config
{
  if [ -e "./${1}" ] ; then

    if [ -z "${SECONDARY_CFG}" ] ; then
      SECONDARY_CFG="-D TSE_SERVER.OVERRIDE_CFGS="
    fi

    if [ ${NUMBER_OF_SECONDARIES} -gt 0 ] ; then
      SECONDARY_CFG="${SECONDARY_CFG}|"
    fi

    SECONDARY_CFG="${SECONDARY_CFG}${1}"
    NUMBER_OF_SECONDARIES=$(expr ${NUMBER_OF_SECONDARIES} + 1)

  fi
}

function add_to_lib_var
{
  local redhat_release="${1}"
  local var_to_set="${2}"
  local -a libset

  #             ------------     ----------------------------------     ----------------------------------
  #             COMPONENT        REDHAT 4 LIBRARY                       REDHAT 5 LIBRARY
  #             ------------     ----------------------------------     ----------------------------------
  libset[0]="   mysql            mysql_shared-4.0.22                    mysql_shared-4.1.21               "
  libset[1]="   sfc              sfc-1.17/lib                           sfc-1.20/lib                      "
  libset[2]="   ACE_wrappers     ACE_wrappers_OCI_1.3a_p16/lib          ACE_wrappers_OCI_1.3a_p17/lib     "
  libset[3]="   boost            boost_1_33_1-RHEL4/lib                 boost_1_60_0/lib                  "
  libset[4]="   xercesc          xercesc-2.7-RHEL4/lib                  xercesc-2.7-RHEL5/lib             "
  libset[5]="   log4cxx          apache-log4cxx-0.10.0/lib              apache-log4cxx-0.10.0/lib         "
  libset[6]="   eo               eo.11282005/lib                        eo.11282005/lib                   "
  libset[7]="   log4cplus        log4cplus_0827                         log4cplus_0827                    "
  libset[8]="   asap             asap.2.0/lib                           asap.2.0/lib                      "
  libset[9]="  asap-wrapper     asapwrapper-1.02d/lib                  asapwrapper-1.02d/lib             "
  libset[10]="  tibco            Tibco_binLibs_061705/lib               Tibco_binLibs_061705/lib          "
  libset[11]="  apr              apr-1.2.12/lib                         apr-1.2.12/lib                    "
  libset[12]="  apr-util         apr-util-1.2.12/lib                    apr-util-1.2.12/lib               "
  libset[13]="  oracle           oracle_v10.1/instantclient/lib         oracle_v11.2/lib                  "
  libset[14]="  berkeleydb       bdb_v4.7/lib                           bdb_v5.0/lib                      "
  libset[15]="  libevent         libevent-1.4.10/lib                    libevent-1.4.10/lib               "
  libset[16]="  libmemcached     libmemcached-0.28/lib                  libmemcached-0.28/lib             "
  libset[17]="  tcmalloc         google-perftools-1.4-ATSEv2            gperftools-2.1/lib                "
  libset[18]="  libunwind        libunwind-0.99-beta                    libunwind-1.1/lib                 "
  libset[19]="  jemalloc         jemalloc-2.1.1/lib                     jemalloc-3.4.1/lib                "
  libset[20]="  snappy           snappy-1.0.5/lib                       snappy-1.1.1/lib                  "
  libset[21]="  lz4              lz4-r124/lib                           lz4-r124/lib                      "
  libset[22]="  xalanc           xalan/lib                              xalan/lib                         "
  libset[23]="  tbb              tbb40_20120408oss/lib                  tbb40_20120408oss/lib             "
  #             ------------     ----------------------------------     ----------------------------------

  eval "var_value=\"\$$var_to_set\""

  local lib_location="${COMMON_ATSEINTL_HOME}/adm/thirdparty"

  local idx=0
  local var_value=""
  local libver=""
  local linktarget=""
  local rh4=""
  local rh5=""
  local dev=""
  if [ -n "${var_value}" ] ; then
    var_value="${var_value}:"
  fi
  var_value="${var_value}./lib"

  while [ ${idx} -lt ${#libset[*]} ] ; do

    rh4=$(echo ${libset[${idx}]} | awk '{print $2}')
    rh5=$(echo ${libset[${idx}]} | awk '{print $3}')

    if [ "${redhat_release}" = "4" -a "${rh4}" != "-" ] ; then
      libver="${lib_location}/${rh4}"
    elif [ "${redhat_release}" = "5" -a "${rh5}" != "-" ] ; then
      libver="${lib_location}/${rh5}"
    else
      libver=""
    fi

    if [ -n "${libver}" ] ; then
      if [ -n "${var_value}" ] ; then
        var_value="${var_value}:"
      fi
      var_value="${var_value}${libver}"
    fi

    idx=$(expr ${idx} + 1)
  done

  # RedHat 6 support
  if [ -x ./ld_path.sh ]; then
    var_value="$(./ld_path.sh)"
  fi
  eval "${var_to_set}=${var_value}"
}

function final_config_setup
{
  local app="${1}"
  local lib_path_var="${2}"
  local addl_var="${3}"
  local bigip_pool="${4}"
  local bigip_pool_ckpt="${5}"

  local tempargs=""

  add_to_lib_var ${COMMON_REDHAT_RELEASE} ${lib_path_var}

  # ACMS SUPPORT START
  eval ACMS_SUPPORT=\$ADMIN_${ADMIN_SERVICES}_ACMS_QUERY
  eval ACMS_DAILY_FILE=\$ADMIN_${ADMIN_SERVICES}_ACMS_DAILY_FILE

  if [ -z "${ACMS_SUPPORT}" ] ; then
    ACMS_SUPPORT="N"
  else
    ACMS_SUPPORT=$(echo ${ACMS_SUPPORT} | tr "[:lower:]" "[:upper:]")
  fi

  if [ "${ACMS_SUPPORT}" == "Y" ]; then
    PRIMARY_CFG="-c ./tseserver.acms.cfg"
    SECONDARY_CFG=""
    NUMBER_OF_SECONDARIES=0
    add_secondary_config tseserver.cfg.node
  elif [ ! -z "${ACMS_DAILY_FILE}" ] ; then
    PRIMARY_CFG="-c ${ACMS_DAILY_FILE}"
    SECONDARY_CFG=""
    NUMBER_OF_SECONDARIES=0
    add_secondary_config tseserver.cfg.node
  fi
  # ACMS SUPPORT END

  tempargs="-D TO_MAN.IOR_FILE=tseserver.${app}.ior ${PRIMARY_CFG} ${SECONDARY_CFG}"

  eval ${addl_var}=\"${tempargs}\"

  return 0
}

function atsev2_set_file_config_value
{
# atsev2_set_file_config_value FILE SECTION VARIABLE VALUE
  local f=${1}
  local s=${2}
  local c=${3}
  local v=${4}
  local t=$( mktemp --tmpdir=. )
  cat ${f} \
  | awk -v s=${s} -v c=${c} -v v=${v} '
    BEGIN { s="["s"]"; done=0 }
    /^\[.*\]$/ { S=$0; print }
    /^#/ { print }
    /^[^#]*=/  { if ( !done && ( s == S ) && ( $1 == c ) ) { print c,"=",v; done=1 } else { print } }
    /^$/ { if ( !done && ( s == S ) )                { print c,"=",v; done=1 } else { print } }
    END  { if ( !done )                     { print s; print c,"=",v; print "" } }' \
  > ${t}
  mv ${t} ${f}
}

declare CFGVAL=""
function atsev2_get_config_value
{
  CFGVAL=$(cfgshell_lookup ${1} ${2} ${3})
  appmon_log "${1}:${2} = [${CFGVAL}]"
}

function atsev2_numa
{
  declare APPMON_SERVICE=${1}

  # Support dual instances for NUMA hardware
  eval NUMA=\$ADMIN_${APPMON_SERVICE}_NUMA
  if [ "0${NUMA}" -gt 0 ]; then
    NUMA0=$(( NUMA -1 ))
    eval TSESERVER=\$ADMIN_${APPMON_SERVICE}_ACMS_FILES
    TMPCFG=$( mktemp --tmpdir=. )
    cp "${TSESERVER}" "${TMPCFG}"
    cfgshell_flatten_from_file_list ${TMPCFG}

    # [APPLICATION_CONSOLE] PORT 50##
    atsev2_get_config_value APPLICATION_CONSOLE PORT 0
    if [ "${CFGVAL}" = "0" ] ; then
      APPLICATION_CONSOLE_PORT=0
    else
      APPLICATION_CONSOLE_PORT=$(( CFGVAL + NUMA0 ))
    fi
    atsev2_set_file_config_value ${TMPCFG} APPLICATION_CONSOLE PORT ${APPLICATION_CONSOLE_PORT}

    # [SERVER_SOCKET_ADP] PORT 53###
    atsev2_get_config_value SERVER_SOCKET_ADP PORT 0
    if [ "${CFGVAL}" = "0" ] ; then
      SERVER_SOCKET_ADP_PORT=0
    else
      SERVER_SOCKET_ADP_PORT=$(( CFGVAL + NUMA0 ))
    fi
    atsev2_set_file_config_value ${TMPCFG} SERVER_SOCKET_ADP PORT ${SERVER_SOCKET_ADP_PORT}

    # [CACHE_ADP] REDO_MISSING_ORDERNO_FILE cachenotification-unprocessed-<SVC>.log
    atsev2_set_file_config_value ${TMPCFG} CACHE_ADP REDO_MISSING_ORDERNO_FILE cachenotification-unprocessed-${APPMON_SERVICE}.log

    # [CACHE_ADP] RESYNC_FROM_LAST_ORDERNO_FILE cachenotification-resync-<SVC>.log
    atsev2_set_file_config_value ${TMPCFG} CACHE_ADP RESYNC_FROM_LAST_ORDERNO_FILE cachenotification-resync-${APPMON_SERVICE}.log

    # [CACHE_ADP] THREAD_ALIVE_FILE cache.thread.alive-<SVC>
    atsev2_set_file_config_value ${TMPCFG} CACHE_ADP THREAD_ALIVE_FILE cache.thread.alive-${APPMON_SERVICE}

    # [DISK_CACHE_OPTIONS] DIRECTORY ../ldc-<SVC>
    atsev2_get_config_value DISK_CACHE_OPTIONS DIRECTORY ../ldc
    atsev2_set_file_config_value ${TMPCFG} DISK_CACHE_OPTIONS DIRECTORY ${CFGVAL}-${APPMON_SERVICE}

    # [TO_MAN] IOR_FILE .../tseserver-<SVC>.ior
    atsev2_get_config_value TO_MAN IOR_FILE tseserver.ior
    if [[ "${CFGVAL}" =~ ".ior" ]]; then
      CFGVAL=${CFGVAL/.ior/-${APPMON_SERVICE}.ior}
    else
      CFGVAL=${CFGVAL}-${APPMON_SERVICE}
    fi
    atsev2_set_file_config_value ${TMPCFG} TO_MAN IOR_FILE ${CFGVAL}

    # [TSE_SERVER] LOG_FILE tseserver-<SVC>.log
    atsev2_get_config_value TSE_SERVER LOG_FILE tseserver.log
    if [[ "${CFGVAL}" =~ ".log" ]]; then
      CFGVAL=${CFGVAL/.log/-${APPMON_SERVICE}.log}
    else
      CFGVAL=${CFGVAL}-${APPMON_SERVICE}
    fi
    atsev2_set_file_config_value ${TMPCFG} TSE_SERVER LOG_FILE ${CFGVAL}

    # [TSE_SERVER] LOG_CFG log4cxx-<SVC>.xml
    atsev2_get_config_value TSE_SERVER LOG_CFG log4cxx.xml
    LOG4CXX_XML_OLD=${CFGVAL}
    if [[ "${CFGVAL}" =~ ".xml" ]]; then
      CFGVAL=${CFGVAL/.xml/-${APPMON_SERVICE}.xml}
    else
      CFGVAL=${CFGVAL}-${APPMON_SERVICE}
    fi
    LOG4CXX_XML_NEW=${CFGVAL}
    atsev2_set_file_config_value ${TMPCFG} TSE_SERVER LOG_CFG ${CFGVAL}

    # --- tesserver-<SVC>.cfg
    mv ${TMPCFG} tseserver-${APPMON_SERVICE}.cfg

    # --- log4cxx-<SVC>.xml
    cat ${LOG4CXX_XML_OLD} \
    | sed "s/.log\"/-${APPMON_SERVICE}.log\"/" \
    > ${LOG4CXX_XML_NEW}

    # -- tseserver-<SVC>.cfg.user
    if [ ! -f tseserver-${APPMON_SERVICE}.cfg.user ]; then
      ln -snf tseserver.cfg.user tseserver-${APPMON_SERVICE}.cfg.user
    fi
  fi
}

