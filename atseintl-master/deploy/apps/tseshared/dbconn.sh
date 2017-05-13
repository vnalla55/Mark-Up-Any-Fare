#!/bin/ksh

#===========================================================================
# MANUAL CONFIGURATION

FUNCxx=1

FUNCCx=1
FUNCC9=0

FUNCPA=0
FUNCPB=0
FUNCPx=1

#===========================================================================
# SETUP

TEST=0
if [ "${USER}" == "sg206456" ]; then
  TEST=1
  set -x
  TEST_SVC=${1:-MIPD}
  TEST_NODE=${2:-pimlp999}
  TEST_ENV=${3:-prod}
fi

ATSE_COMMON=/opt/atse/common
if (( TEST )); then
  ATSE_COMMON=${PWD}/${TEST_ENV}
fi

ORACLE_SVC_SUFFIX=$1

#===========================================================================
# CSF COMMON VARS

SHOWVARS_TMP=/tmp/.dbconn.showvars.$$
${ATSE_COMMON}/showvars.sh > ${SHOWVARS_TMP}
. ${SHOWVARS_TMP}

if (( TEST )); then
  COMMON_NODE=${TEST_NODE}
  COMMON_DB_INI=${PWD}/${TEST_ENV}/opt.db.ini
  COMMON_ADMIN_USER=${COMMON_STAGING_NODE_USER}
fi

#===========================================================================
# ORACLE CONNECTIONS

if (( TEST )); then
  cp "${COMMON_DB_INI}/bak/dbconn.ini" "${COMMON_DB_INI}"
fi

WALLET_SCRIPT=$( ls -1 ${COMMON_DB_INI}/oracle-wallet/${COMMON_ADMIN_USER}/sqlplus.*.${COMMON_ADMIN_USER}.sh | head -1 )
if (( TEST )); then
  WALLET_SCRIPT=$( ls -1 ${PWD}/${TEST_ENV}/wallet.sh | head -1 )
fi
if [ ! -x "${WALLET_SCRIPT}" ]; then
  echo "ERROR: Unable to locate wallet validation script (${COMMON_DB_INI}/oracle-wallet/${COMMON_ADMIN_USER}/sqlplus.*.${COMMON_ADMIN_USER}.sh)."
  exit
fi

WALLET_OUT=/tmp/.dbconn.wallet.$$

${WALLET_SCRIPT} \
| grep okay \
| tr '[:lower:]' '[:upper:]' \
| awk '{ print $4 }' \
| awk -F@ '{ print $2 }' \
> ${WALLET_OUT}

cat ${WALLET_OUT}

#===========================================================================
# DETERMINE BEST SERVICE

# DBConnect\ORACLE_EXPLORER=:@FUNCP_MIPD
# DBConnect\ORACLE=func_au_shp:tmp987@FUNCPB_MIPD,func_au_shp:tmp987@FUNCP5_MIPD,func_au_shp:tmp987@FUNCP6_MIPD

for DBCONNECT in ORACLE_EXPLORER ORACLE_HISTORICAL_EXPLORER ORACLE ORACLE_HISTORICAL ; do
  ORACLE=$( grep -i "^DBConnect.${DBCONNECT}=" ${COMMON_DB_INI}/dbconn.ini | cut -f2 -d= | cut -f1 -d, )
  if [ -z "${ORACLE_USR}" ]; then
    ORACLE_USR=$( echo "${ORACLE}" | cut -f1 -d@ | cut -f1 -d: )
  fi
  if [ -z "${ORACLE_PWD}" ]; then
    ORACLE_PWD=$( echo "${ORACLE}" | cut -f1 -d@ | cut -f2 -d: )
  fi
  if [ -z "${ORACLE_SVC}" ]; then
    ORACLE_SVC=$( echo "${ORACLE}" | cut -f2 -d@ )
  fi
  if [ -z "${ORACLE_SVC_PREFIX}" ]; then
    ORACLE_SVC_PREFIX=$( echo "${ORACLE_SVC}" | cut -f1 -d_ | cut -c-5 )
  fi
  if [ -z "${ORACLE_SVC_SUFFIX}" ]; then
    ORACLE_SVC_SUFFIX=$( echo "${ORACLE_SVC}" | cut -f2 -d_ )
  fi
done

if (( TEST )); then
  ORACLE_SVC_SUFFIX=${TEST_SVC}
fi

if [ -z "${ORACLE_SVC_SUFFIX}" ]; then
  echo "ERROR: Unable to determine oracle service"
else

#===========================================================================
# GENERATE CLUSTER AVAIL AND WEIGHTS

typeset -A FUNC

CLUSTERS=$( grep ${ORACLE_SVC_SUFFIX} ${WALLET_OUT} | cut -f1 -d_ )
for _cluster in ${CLUSTERS} ; do
  eval _weight=\$${_cluster}
  if [ -z "${_weight}" ]; then
    ICP=$( echo ${COMMON_SYSTYPE_ALIAS} | cut -c1 | tr '[:lower:]' '[:upper:]' )
    _weight=\${FUNC${ICP}x}
    if [ -z "${_weight}" ]; then
      _weight=${FUNCxx}
      if [ -z "${_weight}" ]; then
        _weight=1
      fi
    fi
  fi
  if (( TEST )); then
    echo "${_cluster}=${_weight}"
    echo "FUNC[\"${_cluster}\"]=${_weight}"
  fi
  eval FUNC["${_cluster}"]=${_weight}
done

if (( TEST )); then
  echo "!FUNC[*]=${!FUNC[*]}"
  echo "FUNC[*]=${FUNC[*]}"
fi

#===========================================================================
# LOAD BALANCE

dbs=""
#hash=$( echo ${COMMON_NODE}x | cksum | awk '{ print $1 }' )
hash=$( echo ${COMMON_NODE} | sed 's/[^1-9]*//' )

# for iter in pri sec ter ; do
for iter in pri sec ter qua qui ses sept oct nan ; do
  typeset -i sum=0
  for i in ${FUNC[*]} ; do
    (( sum += i ))
  done

  if (( sum > 0 )); then
    val=$(( hash % sum ))

    _db=""
    for _cluster in ${!FUNC[*]} ; do
      if [ -z "${_db}" ]; then
        if (( val < FUNC[${_cluster}] )); then
          _db=${_cluster}
        else
          (( val -= FUNC[${_cluster}] ))
        fi
      fi
    done

    if [ -n "${_db}" ]; then
      dbs="${dbs} ${_db}"
      FUNC[${_db}]=0
    fi
  fi
done

#===========================================================================
# WALLET TEST

if grep -q "TNS_ADMIN.*oracle-wallet" ${COMMON_ATSEINTL_HOME}/*/config.vars* ; then
  ORACLE_USR=""
  ORACLE_PWD=""
fi

#===========================================================================
# CONNECTION STRING

# DBConnect\ORACLE=func_au_shp:tmp654@FUNCC2_MIPA,func_au_shp:tmp654@FUNCC1_MIPA

DBCONNECT_ORACLE=""
for _cluster in ${dbs} ; do
  if [ -n "${DBCONNECT_ORACLE}" ]; then
    DBCONNECT_ORACLE="${DBCONNECT_ORACLE},"
  fi
  DBCONNECT_ORACLE="${DBCONNECT_ORACLE}${ORACLE_USR}:${ORACLE_PWD}@${_cluster}_${ORACLE_SVC_SUFFIX}"
done

if (( TEST )); then
  echo "DBConnect\\ORACLE=${DBCONNECT_ORACLE}"
  echo "DBConnect\\ORACLE_HISTORICAL=${DBCONNECT_ORACLE}"
fi

#===========================================================================
# RECREATE DBCONN FILE

DBCONN_INI=${COMMON_DB_INI}/dbconn.ini
DBCONN_BAK=${COMMON_DB_INI}/dbconn.ini.bak
DBCONN_TMP=/tmp/.dbconn.dbconn.$$

cp "${DBCONN_INI}" "${DBCONN_BAK}"

# DBConnect\ORACLE=func_au_shp:tmp654@FUNCC2_MIPA,func_au_shp:tmp654@FUNCC1_MIPA
# DBConnect\ORACLE_HISTORICAL=func_au_shp:tmp654@FUNCC2_MIPA,func_au_shp:tmp654@FUNCC1_MIPA

cat >${DBCONN_TMP} <<-EOF
	DBConnect\ORACLE=${DBCONNECT_ORACLE}
	DBConnect\ORACLE_HISTORICAL=${DBCONNECT_ORACLE}
EOF
cat ${DBCONN_INI} \
| grep -vi "^DBConnect.ORACLE=" \
| grep -vi "^DBConnect.ORACLE_HISTORICAL=" \
>> ${DBCONN_TMP}

mv "${DBCONN_TMP}" "${DBCONN_INI}"

#===========================================================================
# CLEANUP

fi
rm -f /tmp/.dbconn.*.$$

#===========================================================================
# EXIT

exit
