#!/bin/bash

echo ""
echo "Entering LDCTOOLS shell."

if [ -z "${1}" ] ; then
  echo "SORRY!  You have not specified an app."
else
  export APP=${1}
  export LDCTOOLS_LOCATION=$( builtin cd -- "${0%/*}" ; builtin pwd -P )
  bash --init-file ${LDCTOOLS_LOCATION}/env.vars
fi

echo "Leaving LDCTOOLS shell."
echo ""
