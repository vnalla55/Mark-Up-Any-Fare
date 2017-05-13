#!/bin/bash

function dump_keys
{
  local c=""
  for c in $(cachetest.sh $* -cmd type | grep '[0-9*]\. ' | awk '{print $2}') ; do
    cachetest.sh $* -cache ${c} -cmd memkeys | grep -v '^INFO:' 
  done
}

dump_keys $*
