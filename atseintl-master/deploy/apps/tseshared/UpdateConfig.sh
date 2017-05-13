#!/bin/sh
#-----------------------------------------------------------------------------
#
#         Update Dynamic Configuration
#         Date: January 2011
#         Coleman Ammerman
#
#-----------------------------------------------------------------------------
# set -x

alias appmon_log=echo

function main
{
  echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"
  echo "* Command: ${0} ${*}"
  echo "* Invoked by $(whoami) at $(date)"
  echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

  . ./cfgshell_functions.sh
  . ./config_functions.sh
  . ${ATSE_COMMON}/admin.sh

  # start up application services
  admin_app_services acmspull ${*}

  local app_acport
  local service
  local count
  local pfx
  local appmonParams

  for SVC in ${ADMIN_SERVICES} ; do
    say "Updating Configuration for service ${SVC}"
    atsev2_numa ${SVC}

    # determine appconsole port
    service="${SVC}"
    count=1
    pfx="${service}.${count}"
    appmonParams="./.${COMMON_APPNAME}.${COMMON_BASELINE}.${pfx}.runctl"
    if [ -s "${appmonParams}" ]; then
      . ./${appmonParams}

      # load tseserver.cfg parameters (needed for the remainder of this block)
      cfgshell_flatten_from_tseserver_args "${APPMON_COMMAND}"

      # [APPLICATION_CONSOLE] PORT 50##
      app_acport=$(cfgshell_lookup APPLICATION_CONSOLE PORT 0)

      # reload configuration
      if [ "${app_acport}" != "0" ] ; then
        ./dcfg.pl localhost ${app_acport}
      fi
    fi
  done
}

# ensure we are running within the app directory
cd $(dirname $(which ${0}))

# do the work
main ${*}
exit 0
