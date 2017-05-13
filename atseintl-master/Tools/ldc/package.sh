#!/bin/sh

MY_LOCATION=$(which ${0})
MY_DIR=$(dirname ${MY_LOCATION})
MY_PKG=ldctools.tar.gz
MY_RC=0

cd ${MY_DIR}
cd ..
rm -f ldctools
ln -sf ldc ldctools
rm -f ${MY_PKG} 
tar -h -czf ${MY_PKG} ldctools
MY_RC=$?
rm -f ldctools
if [ -e ${MY_PKG} ] ; then
  echo "Created $(pwd)/${MY_PKG}"
else
  echo "ERROR: Unable to create $(pwd)/${MY_PKG}!"
fi

exit ${MY_RC}
