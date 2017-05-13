#!/bin/bash
. ./set.atse.common.sh
. ${ATSE_COMMON}/config.vars

#This is unfortunate but we had to make hardcode because current scripts does not allow to change default 3rdParty location
COMMON_THIRDPARTY=${COMMON_ATSEINTL_HOME}/adm/thirdparty

declare LD=
LD=./lib
LD=${LD}:${COMMON_THIRDPARTY}/oracle_v11.2/lib
LD=${LD}:${COMMON_THIRDPARTY}/bdb_v5.0/lib
LD=${LD}:${COMMON_THIRDPARTY}/libmemcached-0.28/lib
LD=${LD}:${COMMON_THIRDPARTY}/jemalloc-3.4.1/lib
LD=${LD}:${COMMON_THIRDPARTY}/tbb40_20120408oss/lib
LD=${LD}:${COMMON_THIRDPARTY}/snappy-1.1.1_debug/lib
LD=${LD}:${COMMON_THIRDPARTY}/lz4-r124_debug/lib
LD=${LD}:${COMMON_THIRDPARTY}/boost_1_60_0_debug/lib
LD=${LD}:${COMMON_THIRDPARTY}/apache-log4cxx-0.10.0_debug/lib
LD=${LD}:${COMMON_THIRDPARTY}/libunwind-1.1/lib
LD=${LD}:${COMMON_THIRDPARTY}/gperftools-2.1/lib
LD=${LD}:${COMMON_THIRDPARTY}/apr-1.5.0_debug/lib
LD=${LD}:${COMMON_THIRDPARTY}/apr-util-1.5.3_debug/lib


case ${COMMON_REDHAT_RELEASE} in
    6)
        LD=${LD}:${COMMON_THIRDPARTY}/xercesc-2.8.0/lib
        LD=${LD}:${COMMON_THIRDPARTY}/xalan/lib
        ;;
    *)
        LD=${LD}:${COMMON_THIRDPARTY}/xercesc-2.7-RHEL5/lib
        LD=${LD}:${COMMON_THIRDPARTY}/xalan/Xalan-C_1_10_0_linux_64/lib
        ;;
esac
echo ${LD}
