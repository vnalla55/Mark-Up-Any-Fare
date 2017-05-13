#!/bin/bash

function findBaseDir() {
   local dir=$PWD
   local dirname=`basename $dir`
   while [[ "$dir" != '/' && "$dirname" != 'Taxes' ]]
   do
      dir=`dirname $dir`
      dirname=`basename $dir`
   done
   if [[ "$dir" == '/' ]]
   then
      echo Not in any directory named Taxes, aborting.
      exit 1
   fi
   BASE_DIR=$dir
}

findBaseDir

cd "$BASE_DIR/TestServer"

if [ x"$1" == "x--build" ]; then
  shift
  echo Rebuilding the server...
  scons atpco_test_server -j120 DIST=1
  if [ $? -gt 0 ]; then
      echo "Build failed!"; 
    exit 1
  fi
elif [ x"$1" == "x--clean" ]; then
  shift
  echo Cleaning the server...
  scons -c atpco_test_server
  if [ $? -gt 0 ]; then
    echo "Cleaning failed!"; 
    exit 1
  fi
fi

BUILD=debug
BINARY="$BASE_DIR/TestServer/Server/$BUILD/xtaxserver"
if [ ! -x "$BINARY" ]; then
  echo The server "$BINARY" has not been built yet, please use the \"--build\" option.
  exit 1
fi

echo -n Setting LD_LIBRARY_PATH=
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/atseintl/adm/thirdparty/boost_1_60_0/lib
echo $LD_LIBRARY_PATH

echo Starting $BINARY
        $BINARY $1 $2 $3 $4 $5 $6 $7 $8 $9

