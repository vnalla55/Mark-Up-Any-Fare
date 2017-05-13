# CompressedCacheTest Makefile
#
#  Copyright Sabre 2016
#
#          The copyright to the computer program(s) herein
#          is the property of Sabre.
#          The program(s) may be used and/or copied only with
#          the written permission of Sabre or in accordance
#          with the terms and conditions stipulated in the
#          agreement/contract under which the program(s)
#          have been supplied.
#
#-------------------------------------------------------------------

BINUTILS_DIR_ := /opt/binutils-2.23.2-gcc-4.8.1/bin
CPPSTD_ := c++11
#ASAN := -fsanitize=address
#SANLINK := -static-libasan
OPTIMIZATIONS := -O2 -finline-functions -funroll-loops -minline-all-stringops
CXXFLAGS := -g ${OPTIMIZATIONS} -std=${CPPSTD_} -Wno-error=maybe-uninitialized ${ASAN}
LINKFLAGS := -O2 -B${BINUTILS_DIR_} ${GOLD_LINKFLAGS_}
GOLD_LINKFLAGS_ := -fuse-ld=gold
LINKFLAGS := -B${BINUTILS_DIR_} ${GOLD_LINKFLAGS_} -static-libstdc++
THIRD_PARTY_LIBDIR_ := /opt/atseintl/adm/thirdparty
CXX := ${THIRD_PARTY_LIBDIR_}/gcc-4.8.1/bin/g++

BOOST_DIR := /opt/atseintl/3rdParty/boost_1_58_0
BOOST_INCLUDE := -I$(BOOST_DIR)/include
BOOST_LIB_DIR := $(BOOST_DIR)/lib
LBOOSTLIBS := ${BOOST_LIB_DIR}/libboost_date_time.a \
              ${BOOST_LIB_DIR}/libboost_system.a \
              ${BOOST_LIB_DIR}/libboost_thread.a \
              ${BOOST_LIB_DIR}/libboost_prg_exec_monitor.a \
              ${BOOST_LIB_DIR}/libboost_unit_test_framework.a

SNAPPY_DIR := /opt/atseintl/3rdParty/snappy-1.1.1
SNAPPY_INCLUDE := -I$(SNAPPY_DIR)/include
SNAPPY_LIB_DIR := $(SNAPPY_DIR)/lib
SNAPPY_LIB := ${SNAPPY_LIB_DIR}/libsnappy.a

JEMALLOC_LIB_DIR := /opt/atseintl/3rdParty/jemalloc-3.4.1/lib
JEMALLOC := ${JEMALLOC_LIB_DIR}/libjemalloc.a

INCLUDES := $(BOOST_INCLUDE) $(SNAPPY_INCLUDE)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@

# set the list of object files from the list of source files
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)

compressedcache: $(OBJS)
	$(CXX) $(CXXFLAGS) ${LINKFLAGS} -o compressedcache $(OBJS) $(LBOOSTLIBS) $(SNAPPY_LIB) $(JEMALLOC) -lrt -lpthread ${SANLINK}

clean:
	@echo Deleting objects
	rm -f $(OBJS) compressedcache
