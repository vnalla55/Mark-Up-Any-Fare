#!/bin/bash

TOTAL_RESPONSE_TIME=0
COUNT=0
AVG=0
N=0

for N in $(cat ${1}) ; do
  if [ -n "${N}" ] ; then 
    TOTAL_RESPONSE_TIME=$(echo "${TOTAL_RESPONSE_TIME} + ${N}" | bc)
    COUNT=$(echo "${COUNT} + 1" | bc)
  fi
done

AVG=$(echo "scale=6; ${TOTAL_RESPONSE_TIME} / ${COUNT}" | bc)

printf "Average response time for %d transactions: %0f" ${COUNT} ${AVG}

