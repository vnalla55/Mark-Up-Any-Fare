#!/bin/sh
#-----------------------------------------------------------------------------
#
#         Common Application Deploy
#         Date: July 2007
#         John Watilo
#
#-----------------------------------------------------------------------------
# set -x

function ideploy_main
{
  echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
  echo "* Command: ${0} ${*}"
  echo "* Invoked by $(whoami) at $(date)"
  echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

  # ensure we are running within the directory we are located in
  cd $(dirname $(which ${0}))

  # execute the common deploy script
  local baseline=$(cat .baseline)
  local app=$(cat .app)
  local src=$(pwd -P)
  local target=""

  # set up location of target directory
  if [ -z "${TARGET_SUB_DIR}" ] ; then
    target="${SCM_NODE_ROOT}${SCM_APPLICATION_ROOT}.${SCM_BASELINE}"
  else
    target=${TARGET_SUB_DIR}
  fi

  echo  ""
  echo  "#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
  echo  "# Deploying Application..."
  echo  "#     FROM: ${src}"
  echo  "#       TO: ${target}"
  echo  "#      APP: ${app}"
  echo  "#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
  echo  ""

  # remove the target directory if it already exists
  if [ -e ${target} ] ; then
    rm -rf ${target}
  fi

  # copy the contents of this directory to the target
  local cpCommand="cp -rp ${src} $(dirname ${target})"
  echo ${cpCommand}
  eval ${cpCommand}

  # perform additional first-time setup if defined for the app
  if [ -e ${target}/isetup.sh ] ; then
    cd ${target}
    ./isetup.sh
  fi
}

# do the work
ideploy_main ${*}

