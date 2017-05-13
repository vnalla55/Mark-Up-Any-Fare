#!/bin/bash

export SHOWMSG="$(scons -u --show)"

ROOT=`echo "${SHOWMSG%x}" | grep ROOT_DIR | sed "s/ROOT_DIR.*-.*\ //"`

# Determine build dir name

if [ -z "${SERVERNAME}" ] ; then
  SERVERNAME=tseserver
fi

if [ -e tseserver ]; then
  BUILD_DIR=$(dirname `dirname \`readlink -f tseserver\``)
  export SHOWMSG="$(scons -u --show BUILDDIR=$BUILD_DIR)"
  ABS_BUILDDIR=1
else
  BUILD_DIR=${ROOT}/sc_debug
  ABS_BUILDDIR=0
fi

# Determine LD_LIBRARY_PATH using determined build dir name

LD_LIBRARY_PATH=`echo "${SHOWMSG%x}" | grep STD_LIBPATH_ | sed "s/STD_LIBPATH_.*-.*\ //" | sed "s/\/$//g"`
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:`echo "${SHOWMSG%x}" | grep EXTERNAL_TEST_LIBPATHS | sed "s/EXTERNAL_TEST_LIBPATHS.*-.*\['//" | sed "s/', '/:/g" | sed "s/']//g"`
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:`echo "${SHOWMSG%x}" | grep EXTERNAL_LIBPATHS | sed "s/EXTERNAL_LIBPATHS_.*-.*\['//" | sed "s/', '/:/g" | sed "s/']//g"`
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"/opt/atseintl/adm/thirdparty/gperftools-2.1/lib:/opt/atseintl/adm/thirdparty/libunwind-1.1/lib"
if [ $ABS_BUILDDIR = 1 ]; then
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"`echo "${SHOWMSG%x}" | grep ^LIB_INSTALL_DIR_ | sed "s/LIB_INSTALL_DIR_.*-.*\#\///"`"
else
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:"`echo "${SHOWMSG%x}" | grep ^LIB_INSTALL_DIR_ | sed "s/LIB_INSTALL_DIR_\s*-\s*//" | sed "s@\#@${ROOT}@g"`"
fi

LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/test"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/test/DBAccessMock"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/test/Runner"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/test/testdata"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/test/testdata/tinyxml"
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${BUILD_DIR}/testroot/bld/Pricing"

export LD_LIBRARY_PATH
export ROOT
unset ABS_BUILDDIR

