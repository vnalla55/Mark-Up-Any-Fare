#!/bin/bash
PYTH=/atse_git/fbldfunc/python/bin
export PATH="${PYTH}:$PATH"
SCONS=/atse_git/fbldfunc/scons/lib/scons/scons
CXX_DIR=`dirname \`$SCONS -u --show |grep -w CXX | grep -oe "\S*$"\``
export PATH="${CXX_DIR}:$PATH"
ROOT_DIR=`$SCONS -u --show | grep -w ROOT_DIR | grep -oe"\S*$"`
GCOV=${CXX_DIR}/gcov
GCOV_REPORTER=/atse_git/atsev2ci/gcovr_3_2.py
RATS=/opt/rats-2.4/bin/rats
LIZARD="/opt/Python-2.7.9/bin/python2.7 /opt/lizard/lizard.py"

XMLS=${ROOT_DIR}/xmls
rm -rf $XMLS
mkdir -p $XMLS

JOBS_LIMIT=264
BUILDTYPE=debug
TARGET=all

BUILD_PARAMS="TYPE=$BUILDTYPE -j $JOBS_LIMIT COVERAGE=yes TESTS_REPORTS_DESTINATION=$XMLS DIST=yes LINKTYPE=shared --debug=explain --debug=time --no-cache"

RATS_PARAMS="--xml --resultsonly --columns --context ${ROOT_DIR}"
CMD_BUILD="$SCONS $BUILD_PARAMS $TARGET"
CMD_GCOV_REPORT="$GCOV_REPORTER --gcov-executable=$GCOV -r . -e .*/test/.* -e .*Test.* --xml-pretty -o cov.xml ${ROOT_DIR}/sc_$BUILDTYPE"

CMD_RATS_REPORT="$RATS $RATS_PARAMS"

/usr/bin/time $CMD_BUILD || exit 1

/usr/bin/time $CMD_GCOV_REPORT

/usr/bin/time $LIZARD -C 1000000 -x "${ROOT_DIR}/test/**" -x "${ROOT_DIR}/*/test/*" -x "${ROOT_DIR}/*/componentTest/*" -t 8 -V --xml -b FOREACH -b REVERSE_FOREACH -b BOOST_FOREACH -b BOOST_REVERSE_FOREACH $ROOT_DIR > $ROOT_DIR/lizard.xml

/usr/bin/time $CMD_RATS_REPORT > ${ROOT_DIR}/rats.xml

// Allow Sonar analysis
for file in `grep -l -e"<testsuites></testsuites>" $XMLS/*.xml`; do rm $file; done

