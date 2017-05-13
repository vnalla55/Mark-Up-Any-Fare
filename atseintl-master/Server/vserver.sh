#!/bin/bash

RELEASE=

if [ z"$BUILD" = z ]; then
   RELEASE=debug
else
   RELEASE=$BUILD
fi

IOR=${HOME}/tseserver.ior

while getopts "c:dD:hi:" flag
do
  if [ "$flag" == "i" ]; then
        LINE=`echo $OPTARG | tr [:lower:] [:upper:]`
	IOR=${HOME}/tseserver$LINE.ior
  fi
done

echo 'Setting release to '$RELEASE
echo 'Setting iorfile to '$IOR

export VALGHOME=/opt/valgrind-2.0.0
export VALGPROF=/opt/calltree-0.9.7
export VALGHOMEBIN=$VALGHOME/bin
export VALGHOMELIB=$VALGHOME/lib/valgrind
export VALGPROFBIN=$VALGPROF/bin
export VALGPROFLIB=$VALGPROF/lib/valgrind

export PATH=$VALGPROF:$VALGPROFBIN:$VALGPROFLIB:$VALGHOME:$VALGHOMEBIN:$VALGHOMELIB:$PATH
export LD_LIBRARY_PATH=$VALGPROF:$VALGPROFLIB:$VALGHOME:$VALGHOMELIB:$LD_LIBRARY_PATH


export FFCDIR=/vobs/atseintl/Tools/CTAnalyzer/running/FFC
export LD_LIBRARY_PATH=$FFCDIR:$LD_LIBRARY_PATH

echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo

${VALGHOME}/bin/valgrind -v --skin=calltree --simulate-cache=no --dump-threads=yes /vobs/atseintl/bin/debug/tseserver -D TO_MAN.IOR_FILE=$IOR $*

