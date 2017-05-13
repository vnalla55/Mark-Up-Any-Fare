#!/bin/sh

# ensure we are running within the same directory
# from which we are running
cd $(dirname $(which ${0}))
. ./env.vars

DBCONN=/opt/atseintl/${APP}/dbconn.ini
if [ -e ${DBCONN} ] ; then
  DBHOST=$(grep HYBRID ${DBCONN} | grep -v '#' | awk -F'[@:]' '{print $2}')
  DBPORT=$(grep HYBRID ${DBCONN} | grep -v '#' | awk -F'[:/]' '{print $2}')
else
  DBHOST=atseidb1.dev.sabre.com
  DBPORT=3310
fi

THE_QUERY="$*"

if [ -z "${THE_QUERY}" ] ; then
  echo "SORRY!  You must supply a SQL query."
  exit 1
fi

echo ${THE_QUERY}

MYSQL_CMD="/usr/local/mysql/bin/mysql -A"
MYSQL_CMD="${MYSQL_CMD} --host=${DBHOST}"
MYSQL_CMD="${MYSQL_CMD} --port=${DBPORT}"
MYSQL_CMD="${MYSQL_CMD} --database=ATSEHYB1"
MYSQL_CMD="${MYSQL_CMD} --user=hybfunc"
#MYSQL_CMD="${MYSQL_CMD} --batch"
MYSQL_CMD="${MYSQL_CMD} --password="
#MYSQL_CMD="${MYSQL_CMD} --disable-auto-rehash"
#MYSQL_CMD="${MYSQL_CMD} --skip-column-names"

$MYSQL_CMD

#echo "${THE_QUERY}" | ${MYSQL_CMD}
