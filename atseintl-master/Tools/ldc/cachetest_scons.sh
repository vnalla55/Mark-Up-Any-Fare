
ATSE_PATH=`pwd`/../..

echo $ATSE_PATH

THIRD_PARTY_LIBDIR=/opt/atseintl/adm/thirdparty


export LD_LIBRARY_PATH=$THIRD_PARTY_LIBDIR/jemalloc-3.4.1/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$THIRD_PARTY_LIBDIR/tbb40_20120408oss/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$THIRD_PARTY_LIBDIR/apache-log4cxx-0.10.0/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$THIRD_PARTY_LIBDIR/xercesc-2.8.0/lib/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ATSE_PATH/sc_debug/Allocator/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ATSE_PATH/sc_debug/idop_libcpp/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ATSE_PATH/sc_debug/idop_libxml/:$LD_LIBRARY_PATH

echo $LD_LIBRARY_PATH

$ATSE_PATH/sc_debug/Tools/ldc/cachetest

