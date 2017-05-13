#!/bin/bash
CUR_DIR="$( cd "$( dirname "$0" )" && pwd )"
NSIQCPPSTYLE_DIR=$CUR_DIR/nsiqcppstyle

NSIQCPPSTYLE=$NSIQCPPSTYLE_DIR/nsiqcppstyle
RULES_FILE=$NSIQCPPSTYLE_DIR/source.txt

if [ $# -eq 0 ]
then
  echo "Usage:   $0 [FILE1] [FILE2] [DIR1] [DIR2] ..."
  echo "Example: $0 ."
fi

for ARG in "$@"
do
  $NSIQCPPSTYLE --output=gcc -f $RULES_FILE $ARG | awk '/^\//'
done
