#!/bin/bash
# $@ is to pass host and port from caller

basedir=$(dirname $0)
request=$basedir/MipReq.xml

# Concatenate response file name with current pid,
# that will allow to manually find appropriate response by run log,
# in a case, if it will be useful to analyze it
response=$basedir/Response_$$.out

# Heat up LDC cache and log the response on the 4th run
(seq 4 | xargs -i -n1 hammer.pl $@ --print --output $response $request) || exit 255

if diff -q --report-identical-files $response $basedir/Response_IntB.out
then
  exit 0 #good commit
elif diff -q --report-identical-files $response $basedir/Response_IntA.out
then
  exit 1 #bad commit
else
  exit 125 #skip commit
fi
