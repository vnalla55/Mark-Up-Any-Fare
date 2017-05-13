#!/bin/bash

RELEASE=
if [ z"$BUILD" = z ]; then
RELEASE=debug
else
RELEASE=$BUILD
fi

IOR=${HOME}/tseserver.ior
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/lib/$RELEASE
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/Tools/Diagnostic/lib
LD_LIBRARY_PATH=$FFCDIR/bin:$LD_LIBRARY_PATH

exec 3< $1
read -u3 CMDXML
echo "Sending $CMDXML"
/vobs/atseintl/Tools/CorbaTestClient/Client -i ${IOR} -c "$CMDXML"



