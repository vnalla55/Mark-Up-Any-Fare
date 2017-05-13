#!/bin/sh

ACTION=${1}

. ./cfgshell_functions.sh
. ./config_functions.sh

# alias appmon_log=echo

function atsev2_ldc_prestart
{
  atsev2_get_config_value DISK_CACHE_OPTIONS ENABLE_PRESTART_HOOK Y
  if [ "${CFGVAL}" = "Y" ] ; then
    local -i ldc_begin=$(date +%s)
    local -i ldc_end=0
    local -i ldc_delta=0
    atsev2_get_config_value DISK_CACHE_OPTIONS STARTUP_ACTION disable
    if [ "${CFGVAL}" = "load" ] ; then
      atsev2_get_config_value DISK_CACHE_OPTIONS DIRECTORY ../ldc
      local ldc_dir=${CFGVAL}
      if [ -n "${ldc_dir}" ] ; then
        if [ -e ${ldc_dir} ] ; then
          local bdb_path=/opt/atseintl/adm/thirdparty/bdb_v5.0/bin
          if [ -e ${bdb_path} ] ; then
            local bdb_lib_path=/opt/atseintl/adm/thirdparty/bdb_v5.0/lib
            if [ -e ${bdb_lib_path} ] ; then

              local dbf=""
              local dbof=""
              local -i dbrc=0
              local -i delrc=0
              local -i time_begin=0
              local -i time_end=0
              local -i time_delta=0

              export PATH=${bdb_path}:$PATH
              export LD_LIBRARY_PATH=${bdb_lib_path}:$LD_LIBRARY_PATH

              atsev2_get_config_value DISK_CACHE_OPTIONS PRESTART_RECOVER Y
              if [ "${CFGVAL}" = "Y" ] ; then
                time_begin=$(date +%s)
                db_recover -h ${ldc_dir} > db_recover.out 2>&1
                time_end=$(date +%s)
                time_delta=$(( time_end - time_begin ))
                chmod 755 db_recover.out
                appmon_log "Berkeley DB recover completed in ${time_delta} seconds."
              else
                rm -f db_recover.out
              fi

              atsev2_get_config_value DISK_CACHE_OPTIONS USE_TXN_MODEL N
              if [ "${CFGVAL}" = "Y" ] ; then
                time_begin=$(date +%s)
                db_archive -dav -h ${ldc_dir} > db_archive.out 2>&1
                time_end=$(date +%s)
                time_delta=$(( time_end - time_begin ))
                chmod 755 db_archive.out
                appmon_log "Berkeley DB archive completed in ${time_delta} seconds."
              else
                rm -f db_archive.out
              fi

              atsev2_get_config_value DISK_CACHE_OPTIONS PRESTART_DB_VERIFY Y
              if [ "${CFGVAL}" = "Y" ] ; then
                atsev2_get_config_value DISK_CACHE_OPTIONS USE_TXN_MODEL N
                if [ "${CFGVAL}" = "Y" ] ; then
                  rm -f db_verify.out
                  time_begin=$(date +%s)
                   for dbf in $(find . -type f -name '*.db') ; do
                    dbf=$(basename ${dbf})
                    echo "***"                      >> db_verify.out
                    echo "*** DB_VERIFY for ${dbf}" >> db_verify.out
                    echo "***"                      >> db_verify.out
                    db_verify -h ${ldc_dir} ${dbf}  >> db_verify.out 2>&1
                    dbrc=$?
                    if [ ${dbrc} -ne 0 ] ; then
                      rm -f ${dbf}
                      delrc=$?
                      if [ ${delrc} -eq 0 ] ; then
                        appmon_log "Deleted file [${dbf}] due to verification failure!"
                      else
                        appmon_log "Verification failure on file [${dbf}] but it could not be deleted!!!"
                      fi
                    fi
                  done
                  time_end=$(date +%s)
                  time_delta=$(( time_end - time_begin ))
                  appmon_log "Berkeley DB file verification completed in ${time_delta} seconds."
                fi
              fi

              atsev2_get_config_value DISK_CACHE_OPTIONS OPEN_FILE_SENTINELS Y
              if [ "${CFGVAL}" = "Y" ] ; then
                time_begin=$(date +%s)
                for dbof in $(find ${ldc_dir} -type f -name '*.*.db.open') ; do
                  dbf=${dbof%*.open}
                  rm ${dbof}
                  rm ${dbf}
                  appmon_log "Found and removed unclosed file ${dbf}"
                done
                time_end=$(date +%s)
                time_delta=$(( time_end - time_begin ))
                appmon_log "Berkeley DB unclosed file cleanup completed in ${time_delta} seconds."
              fi

            else
              appmon_log "ERROR: BDB library path does not exist: ${bdb_lib_path}"
            fi
          else
            appmon_log "ERROR: BDB path does not exist: ${bdb_path}"
          fi
          
          # Precache the files:
          if [ -d ${ldc_dir} ]; then
            if [ -f ${ldc_dir}/.clear ]; then
              appmon_log "Skipping LDC refresh - clear detected"
            else
              appmon_log "Precaching LDC files - begin"
              cksum ${ldc_dir}/*.db > /dev/null
              appmon_log "Precaching LDC files - end"
            fi
          fi
                
        else
          appmon_log "LDC directory is not present"
        fi
      fi
    fi
    ldc_end=$(date +%s)
    ldc_delta=$(( ldc_end - ldc_begin ))
    appmon_log "LDC prestart activities completed in ${ldc_delta} seconds."
  fi
}

function atsev2_find_filesys
{
  # find a writeable filesystem with enough disk space:
  local fsys
  for fsys in `pwd`/savedlogs /var/tmp/atsev2/cores /tmp/atsev2/cores $COMMON_SNAPSHOT_DIRECTORIES
  do
    [ -d $fsys ] || mkdir -p $fsys >> /dev/null
    [[ -d $fsys && -w $fsys ]] || continue
    local diskSpace=`df -P $fsys | awk '/Available/ { next; } {print $4; }'`
    if [[ $diskSpace -ge 50000 ]]; then
      mkdir -p $fsys || continue
      target_filesystem=${fsys}
      return 0
    fi
  done
  return 1
}

function atsev2_snapshot
{
  # if disk space allows, try to capture the system snapshot:
  local target_filesystem
  atsev2_find_filesys
  if [[ $? -ne 0 ]]; then
    email_content="${email_content}\nNot enough disk space to save a snapshot - skipping."
    appmon_log "Failed to save a snapshot - disk full."
    return 1
  fi

  local snapshot_timestamp=`date '+%Y%m%d_%H%M%S'`
  local snapshot_dirname=${target_filesystem}/appmon_hook_tmp_$$
  local snapshot_filename=${target_filesystem}/${APPMON_APPNAME}_${COMMON_BASELINE#atsev2.}_${COMMON_NODE}_${snapshot_timestamp}_snapshot.zip
  local appdir=`pwd`
  mkdir -p ${snapshot_dirname}
  for file in *.log *stderr *.errors
  do
    tail -n 200 ${file} > ${snapshot_dirname}/${file}
  done

  cd ${snapshot_dirname}
  awk -v stamp=`date +%s` -v NUMREQS=10 -f ${appdir}/lastRequests.awk < ${appdir}/tserequest.log > /dev/null
  zip -mq ${snapshot_filename} *
  cd - > /dev/null
  rm -r ${snapshot_dirname}

  ahi_snapshot="${snapshot_filename}"
  appmon_log "Snapshot saved to file ${ahi_snapshot}"
  return 0
}

# failure action

if [ "${ACTION}" = "failure" ]; then
  # if disk space allows, try to capture the system snapshot:
  atsev2_snapshot
  # Change core permissions to permit truncation once analyzed
  [ -r core.* ] && chmod -f 666 core.*
fi

# failure_email action

if [ "${ACTION}" = "failure_email" ]; then

  # load tseserver.cfg parameters (needed for the remainder of this block)
  cfgshell_flatten_from_tseserver_args "${APPMON_COMMAND}"

  # -- Max transaction restart e-mail hook
  atsev2_get_config_value TSE_SERVER LOG_FILE tseserver.log
  TSESERVER_LOG=${CFGVAL}
  if [ -r ${TSESERVER_LOG} ] ; then
    if tail -200 ${TSESERVER_LOG} | grep -q "Max Transactions exceeded" ; then
      email_subject="Application restarting because of max transactions.  ${cmd_id_string}"
    elif tail -200 ${TSESERVER_LOG} | grep -q "Max Virtual Memory exceeded" ; then
      email_subject="Application restarting because of virtual memory size.  ${cmd_id_string}"
    elif tail -200 ${TSESERVER_LOG} | grep -q "Available memory below limit" ; then
      email_subject="Application restarting because of available memory limit.  ${cmd_id_string}"
    fi
  fi
  if [ "xxx${ahi_snapshot}" != "xxx" ]; then
    email_content="${email_content}\nSnapshot saved to file ${ahi_snapshot}."
    unset ahi_snapshot
  fi

  # clear the config parameters
  cfgshell_reset
fi

# prestart action

if [ "${ACTION}" = "prestart" ]; then
  # Error if filesystem is over COMMON_FILESYSTEM_THRESHOLD %
  pctThreshold=${COMMON_FILESYSTEM_THRESHOLD:-95}
  if [ -n "${pctThreshold}" ] ; then
    pctUsed=$( df -kP . | awk 'NR>1{print $5}' | sed 's/%//' )
    if [ 0$pctUsed -gt ${pctThreshold} ] ; then
      email_subject="Filesystem full! ${cmd_id_string}"
      email_content="Filesystem ${PWD} at ${pctUsed}% full (threshold ${pctThreshold}%). Manually restart application once rectified."
      send_email "${email_subject}" "${email_content}"
      exit 9
    fi
  fi

  # Support dual instances for NUMA hardware
  atsev2_numa ${APPMON_SERVICE}

  # load tseserver.cfg parameters (needed for the remainder of this block)
  cfgshell_flatten_from_tseserver_args "${APPMON_COMMAND}"

  # perform pre-start LDC processing(if enabled)
  atsev2_ldc_prestart

  # get the thread alive file name and remove it - the cache update thread
  # will re-create it when it starts running
  atsev2_get_config_value CACHE_ADP THREAD_ALIVE_FILE cache.thread.alive
  rm -f "${CFGVAL}"

  # clear the config parameters
  cfgshell_reset
fi

# eof
