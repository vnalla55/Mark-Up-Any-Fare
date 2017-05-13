#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

LD_LIBRARY_PATH=$(make print-STD_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | sort | uniq | tr '\n' ':')

# All libs used by makefile:
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}$(make print-LIBS | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | sort | uniq | tr '\n' ':')

# Add unit tests specific path
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}$(make print-TEST_LIBS_DIR | sed 's/\s/\n/g' | grep ^/ | tr '\n' ':')

# Add jemalloc lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}$(make print-JEMALLOC_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | tr '\n' ':')

# Add cppunit lib for running unit tests:
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}$(make print-CU_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | tr '\n' ':')

# Add ASAP lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}$(make print-ASAP_LIB_DIR | sed 's/\s/\n/g' | grep ^-L | sed 's/-L//' | tr '\n' ':')

# Add libraries built by Scons
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${PWD}/../sc_debug/libshared"

# Add Google Profiler library path
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/opt/atseintl/adm/thirdparty/gperftools-2.1/lib:/opt/atseintl/adm/thirdparty/libunwind-1.1/lib"

#echo $LD_LIBRARY_PATH

cd - > /dev/null
