#!/bin/sh

tran="$(cat ${1})"

# ensure we are running within the same directory
# where we are located 
cd $(dirname $(which ${0}))
. ./env.vars

limit=1

rm  -f "${APPDIR}/hammer.txn"
rm  -f "${APPDIR}/hammer.log"

echo "${tran}" | hammer_tseserver.sh -app ${APP} -host ${HOST} -port ${REQPORT} -concurrent 1 -retry -limit ${limit}

cat "${APPDIR}/hammer.log"
