#!/bin/bash
. ./set.atse.common.sh                                              
. ${ATSE_COMMON}/config.vars

COMMON_THIRDPARTY=${COMMON_ATSEINTL_HOME}/adm/thirdparty

declare LD=
LD=${LD}:./lib
LD=${LD}:${COMMON_THIRDPARTY}/oracle_v11.2/lib
LD=${LD}:${COMMON_THIRDPARTY}/boost_1_60_0/lib
LD=${LD}:${COMMON_THIRDPARTY}/bdb_v5.0/lib
LD=${LD}:${COMMON_THIRDPARTY}/libmemcached-0.28/lib
LD=${LD}:${COMMON_THIRDPARTY}/apache-log4cxx-0.10.0/lib
LD=${LD}:${COMMON_THIRDPARTY}/jemalloc-3.4.1/lib
LD=${LD}:${COMMON_THIRDPARTY}/tbb40_20120408oss/lib
LD=${LD}:${COMMON_THIRDPARTY}/snappy-1.1.1/lib
LD=${LD}:${COMMON_THIRDPARTY}/lz4-r124/lib

case ${COMMON_REDHAT_RELEASE} in
    6)
       LD=${LD}:${COMMON_THIRDPARTY}/apr-1.4.6/lib
       LD=${LD}:${COMMON_THIRDPARTY}/apr-util-1.4.1/lib
       LD=${LD}:${COMMON_THIRDPARTY}/xercesc-2.8.0/lib
       LD=${LD}:${COMMON_THIRDPARTY}/asap.3.0/lib
       LD=${LD}:${COMMON_THIRDPARTY}/asapw-2016.08.00_s/lib
       LD=${LD}:${COMMON_THIRDPARTY}/xalan/lib
       ;;
    *)
       LD=${LD}:${COMMON_THIRDPARTY}/apr-1.2.12/lib
       LD=${LD}:${COMMON_THIRDPARTY}/apr-util-1.2.12/lib
       LD=${LD}:${COMMON_THIRDPARTY}/xercesc-2.7-RHEL5/lib
       LD=${LD}:${COMMON_THIRDPARTY}/asap.2.0/lib
       LD=${LD}:${COMMON_THIRDPARTY}/asapwrapper-1.02d/lib
       LD=${LD}:${COMMON_THIRDPARTY}/xalan/Xalan-C_1_10_0_linux_64/lib
       ;;
esac
echo ${LD}
