#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SC_BUILD="${BUILD}"
if [ "x${SC_BUILD}" = "x" ]; then SC_BUILD=debug; fi

SC_DIR="${DIR}/../sc_${SC_BUILD}"

LD_LIBRARY_PATH="$(make -C "$DIR" print-LIBS | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | grep -v lib/${SC_BUILD} | sort | uniq | tr '\n' ':'):${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="$(make -C "$DIR" print-JEMALLOC_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | tr '\n' ':'):${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="$(make -C "$DIR" print-CU_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | tr '\n' ':'):${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="/opt/atseintl/adm/thirdparty/gperftools-2.1/lib:/opt/atseintl/adm/thirdparty/libunwind-1.1/lib:${LD_LIBRARY_PATH}"

LD_LIBRARY_PATH="${SC_DIR}/libshared:${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${SC_DIR}/test:${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${SC_DIR}/test/DBAccessMock:${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${SC_DIR}/test/Runner:${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${SC_DIR}/test/testdata:${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${SC_DIR}/test/testdata/tinyxml:${LD_LIBRARY_PATH}"

export LD_LIBRARY_PATH
