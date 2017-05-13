#!/bin/bash
#
# description: ATSE respawn starter
#

HOSTNAME=`uname -n`;export HOSTNAME

# Check that networking is up.
[ "${NETWORKING}" = "no" ] && exit 0

RETVAL=0

BASEDIR="/login/maker/bin"
CONFIGFILE="apps.${HOSTNAME}.conf"
prog="respawn.pl"

case "$1" in
   start)
      echo -n $"Starting $prog: "

      cd $BASEDIR
      su -c "nohup ./${prog} respawn.conf.d/${CONFIGFILE} > /dev/null &" maker
      ;;
   stop)
      ;;
   *)
      echo $"Usage: $0 {start|stop}"
      exit 1
esac

exit $RETVAL

