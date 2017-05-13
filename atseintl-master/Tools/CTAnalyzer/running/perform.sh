#!/bin/bash

#SERVERDIR=/opt/atseintl/bin/debug
SERVERDIR=/vobs/atseintl/bin/debug
DIR=`pwd`
cd $SERVERDIR
./pserver.sh 1>$DIR/server.1.out 2>$DIR/server.2.out &
cd $DIR

echo "Starting Valgrind..."

while [ -z`cat server.1.out | grep "** TseServer is running **"` ]
do
  sleep 5
done

echo "Valgrind started"

#sleep 2000  # 70 minutes, set by experimenting 
             # actual value depends on the pre-caching

RELEASE=
if [ z"$BUILD" = z ]; then
RELEASE=debug
else
RELEASE=$BUILD
fi

IOR=${HOME}/tseserver.perf.ior
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/lib/$RELEASE
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/vobs/atseintl/Tools/Diagnostic/lib
LD_LIBRARY_PATH=$FFCDIR/bin:$LD_LIBRARY_PATH

CONTROL_PATH=/opt/calltree-0.9.7/bin

function transaction {
    $CONTROL_PATH/ct_control -z

    read -u3 CMDXML
    echo "Sending: $CMDXML"
    /vobs/atseintl/Tools/CorbaTestClient/Client -i ${IOR} -c "$CMDXML"
    $CONTROL_PATH/ct_control -d "$1"  #"Cold cache, simple"

    TODAY=`date +"%Y-%m-%d"`
    mkdir $TODAY
    mkdir $TODAY/archive
    cd $TODAY

    mv $SERVERDIR/cachegrind.out.* .

    for f in `ls cachegrind.out.*`
      do
      $CONTROL_PATH/ct_annotate --inclusive=yes --tree=calling $f > $f.$2.txt #sc
      rm $f
    done

    cd ..
    python analyze.py -d $TODAY cachegrind $TODAY/$3 #simple-cold.xml
    mv $TODAY/cachegrind*.$2.txt $TODAY/archive/
    rm -f $TODAY/cachegrind*
}

exec 3< requests.txt
#exec 3< pooling.txt
# cold cache - simple
transaction "Cold cache, simple" "sc" "simple-cold.xml"

# cold cache - complex
transaction "Cold cache, complex" "cc" "complex-cold.xml"

# warm cache - simple
exec 3< requests.txt
transaction "Warm cache, simple" "sw" "simple-warm.xml"

# Warm cache - complex
transaction "Warm cache, complex" "cw" "complex-warm.xml"

$CONTROL_PATH/ct_control -k
rm -f $TODAY/cachegrind*

cd /login/sg620831/performance
mkdir performance_data/$TODAY
cp $DIR/$TODAY/*.xml performance_data/$TODAY
/opt/java/bin/jar -uf performance.war performance_data/$TODAY/*.xml
cd $DIR

export CLASSPATH=$CLASSPATH:$DIR/catalina-ant.jar
ant undeploy
ant deploy
ant install