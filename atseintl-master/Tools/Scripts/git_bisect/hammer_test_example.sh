#!/bin/bash
# $@ is to pass host and port from caller
hammer.pl $@ --print is_req.xml --diag 922 | grep "EXCLUDE_CODE_SHARE_SOP"
result=$?

# Optional:
# consider "EXCLUDE_CODE_SHARE_SOP" in output is present for "bad" commit
if [ $result -le 1 ]
then
  exit $(expr 1 - $result) # invert exit code
else
  exit $result # something wrong with hammer or tseserver
fi
