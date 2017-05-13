#!/bin/sh
#
# common prestart sequence for tseserver-based applications
#

. ./set.atse.common.sh
. ${ATSE_COMMON}/admin.sh
load_app_config

# export custom config.vars variables
export TRX_ALLOCATOR

# set environment-specific variables
export ATSE_NO_GETOPTS=1
export GLIBCPP_FORCE_NEW=1
export COMMON_DB_INI=${COMMON_DB_INI:-/opt/db/ini}

# oracle-specific
export TNS_ADMIN=${TNS_ADMIN:-${COMMON_DB_INI}}
export NLS_LANG=AMERICAN_AMERICA.AL32UTF8

# create links to tseshared components
for x in $(find ../tseshared.${COMMON_BASELINE}/* -maxdepth 0) ; do
  BASE_FILENAME=$(basename ${x})
  if [ ! -e ./${BASE_FILENAME} ] ; then
    ln -sf ${x} ./${BASE_FILENAME}
  fi
done

# create links to tseshared debug symbols
if [ "${ADMIN_ULIMIT_CORES}" = "unlimited" ]; then
  if [ -d ../tseshared.${COMMON_BASELINE}.debug ]; then
    ln -sf ../tseshared.${COMMON_BASELINE}.debug .debug
  fi
fi

# set up links to config files
for x in $(find . -type l -name "tseserver*cfg") ; do
  if [ -n "$(echo \"$(dirname $(readlink ${x}))\" | grep -E 'cfg\..*')" ] ; then
    rm -f ${x}
  fi
done
if [ "${COMMON_SYSTYPE}" = "stage" ] ; then
  if [ ! -e cfg.${COMMON_SYSTYPE} ] ; then
    ln -snf cfg.ProdHybrid cfg.${COMMON_SYSTYPE}
  fi
fi
for x in $(find -L cfg.${COMMON_SYSTYPE} -maxdepth 1 -type f) ; do
  BASE_FILENAME=$(basename ${x})
  rm -f ./${BASE_FILENAME}
  ln -sf ${x} ./${BASE_FILENAME}
done

# DB access
if [ "${COMMON_SYSTYPE}" = "DevHybrid" ] ; then

  if [ -f dbaccess.ini ] ; then
    for x in $(find tabledefs.${COMMON_SYSTYPE}/*) ; do
      BASE_FILENAME=$(basename ${x})
      rm -f ${BASE_FILENAME}
      ln -sf ${x} ${BASE_FILENAME}
    done
  fi

else

  . /etc/profile

  if [ "${COMMON_SYSTYPE}" = "TestHybrid" ] ; then
    AppSpecificDBConn=${COMMON_DB_INI}/dbconn.${COMMON_APPNAME}.ini
    if [[ -s ${AppSpecificDBConn} ]] ; then
      ln -snf ${AppSpecificDBConn} dbconn.ini
    else
      TestHybridNodeName=$(echo ${COMMON_NODE} | tr "[:upper:]" "[:lower:]")
      NodeSpecificDBConn=dbconn.${TestHybridNodeName}.ini
      if [[ -s $NodeSpecificDBConn ]] ; then
        mv $NodeSpecificDBConn dbconn.ini
      fi
    fi
  fi

  if [ ! -e dbconn.ini ] ; then
    if [ -e ${COMMON_DB_INI}/dbconn.ini ] ; then
      ln -sf ${COMMON_DB_INI}/dbconn.ini dbconn.ini
    fi
  fi

fi

# set umask and ulimits
umask 002
ulimit -Ss ${ADMIN_ULIMIT_STACK}
ulimit -c ${ADMIN_ULIMIT_CORES}
if [ "${ADMIN_ULIMIT_CORES}" = "0" ] ; then
  say "Cores OFF."
else
  say "Cores ON."
fi

# Error if filesystem is over COMMON_FILESYSTEM_THRESHOLD %
pctThreshold=${COMMON_FILESYSTEM_THRESHOLD:-95}
if [ -n "${pctThreshold}" ] ; then
  pctUsed=$( df -kP . | awk 'NR>1{print $5}' | sed 's/%//' )
  if [ 0$pctUsed -gt ${pctThreshold} ] ; then
    say         "Filesystem at ${pctThreshold}% full (threshold ${pctUsed}%)."
    exitError 9 "Filesystem at ${pctThreshold}% full (threshold ${pctUsed}%)."
    exit 9
  fi
fi

# Spit out an error if it was set by an earlier script:
if [ -n "${COMMON_ERROR_MESSAGE}" ] ; then
  say "${COMMON_ERROR_MESSAGE}"
fi

# -- v2 monitor script --
if [ -x v2mon ]; then
  for SVC in ${ADMIN_SERVICES} ; do
    if [ ! -e v2mon.${SVC} ]; then
      ln -snf v2mon v2mon.${SVC}
    fi
    say "Starting v2mon for ${SVC}"
    ./v2mon.${SVC} start ${SVC}
  done
fi

. ${ATSE_COMMON}/admin.sh
