#!/bin/sh
# called by <app>/.app.package.sh

function package_config
{
  local app=${1}
  local builddir=${2}
  local target=${3}
  local vobappname=${4}

  if [ -z "${vobappname}" ] ; then
    vobappname=${app}
  fi

  local system=""
  local f=""
  local search_path=""
  local tag=""
  local sys=""
  local mask=""
  local cfg_target=""
  local mask_idx=0
  local sysfile=""
  local bname=""
  local origin=$(pwd)

  if [ ! -e ${target}/prestart.sh ] ; then
    cd ${target}
    ln -sf ../tseshared.${BASELINE}/prestart.sh prestart.sh
    cd ${origin}
  fi

  if [ ! -e ${target}/checkhung.sh ] ; then
    cd ${target}
    ln -sf ../tseshared.${BASELINE}/checkhung.sh checkhung.sh
    cd ${origin}
  fi

  if [ ! -e ${target}/appmon_hook.sh ] ; then
    cd ${target}
    ln -sf ../tseshared.${BASELINE}/appmon_hook.sh appmon_hook.sh
    cd ${origin}
  fi

  if [ ! -e ${target}/cfgshell_functions.sh ] ; then
    cd ${target}
    ln -sf ../tseshared.${BASELINE}/cfgshell_functions.sh cfgshell_functions.sh
    cd ${origin}
  fi

  local -a masks=( 'tseserver*cfg' '*.properties' 'dbaccess*ini' 'dbconn*ini' )

  local -a SYSs=(DevHybrid TestHybrid CertHybrid PlabHybrid ProdHybrid)
  local -a TAGs=(dev       integ      cert       plab       prod      )

  local i
  for (( i = 0 ; i < ${#SYSs[@]} ; i++ )) ; do
    sys=${SYSs[$i]}
    tag=${TAGs[$i]}

    cfg_target="${target}/cfg.${sys}"

    if [ -e ${cfg_target} ] ; then
      rm -rf ${cfg_target}
    fi
    mkdir -p ${cfg_target}

    if [ -e deploy/${tag}/config/${vobappname} ] ; then
      search_path="deploy/${tag}/config/${vobappname}"
    else
      search_path="Server"
    fi

    set -o noglob
    mask_idx=0
    while [ ${mask_idx} -lt ${#masks[*]} ] ; do
      mask=${masks[${mask_idx}]}
      for f in $(find ${search_path} -name ${mask} 2> /dev/null) ; do
        if [ "${f}" != "Server/dbaccess.ini" ] ; then
          cp_func ${f} ${cfg_target} L

          # Generate dbaccess-exp.ini from dbaccess-ora.ini
          if [ "$(basename ${f})" = "dbaccess-ora.ini" ] ; then
            cat ${f} \
            | sed 's/useExplorer=./useExplorer=Y/' \
            | awk '{print} /^useExplorer/{print "explorerServiceSuffix=_EXP"}' \
            | awk '{print} /^idletimeout=/{print "refreshtimeout=60";print "refreshtimeoutpercentvary=0"}' \
            > ${cfg_target}/dbaccess-exp.ini
          fi

        else
          cat ${f} | sed 's/\/vobs\/atseintl\/bin\/debug\///g' > ${cfg_target}/dbaccess.ini
        fi
      done
      mask_idx=$(expr ${mask_idx} + 1)
    done
    set +o noglob

    # log4crc
    f=${search_path}/log4crc-prime/log4crc
    if [ -e ${f} ] ; then
      cp_func ${f} ${cfg_target} L
    fi
    f=${search_path}/log4crc
    if [ -e ${f} ] ; then
      cp_func ${f} ${cfg_target} L
    fi

    # log4cxx.xml
    f=${search_path}/log4cxx.xml
    if [ -e ${f} ] ; then
      cp_func ${f} ${cfg_target} L
    fi

  done
}

package_config $*
