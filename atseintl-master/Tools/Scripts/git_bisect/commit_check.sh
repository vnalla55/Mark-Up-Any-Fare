#!/bin/bash
#
# Please follow up README for run instructions
#

#
# This script does
#
# 1. make
#
# If hammer_test.sh scipt is present than
#
# 2. clean LDC cache (depends on config)
# 3. start tseserver on local or remote box
# 4. execute hammer_test.sh to make decision if commit is good or bad
# 5. stop tseserver or checks if the server crashed
#
# You are welcome to extend this functionality by editing this script
# or via adding custom steps under commit_checks/hooks/
#

. $(dirname $0)/commit_check.cfg || exit 255 # abort git bisect run

START_TSESERVER_SCRIPT=$(dirname $0)/start_tseserver.pl
HAMMER_TEST_SCRIPT=$(dirname $0)/$HAMMER_TEST_SCRIPT
export VOB_DIR=`pwd`
HOOK_DIR=$(dirname $0)/commit_check.hooks

if [ ! -x $HAMMER_TEST_SCRIPT ]
then
  echo "Please follow up README for run instructions."
  echo ""
  echo "$HAMMER_TEST_SCRIPT was not found or cannot execute it."
  echo "Will make decision only on build result."
fi

# tseserver start log is also redirected to tseserver.log file by $START_TSESERVER_SCRIPT,
# tseserver_crashed check depends expecially on this
start_tseserver() {

  if [ "$SERVER_HOST" != "$(hostname)" ]
  then
    $START_TSESERVER_SCRIPT \
      ssh $SERVER_HOST \
        "'cd $VOB_DIR/bin/debug && VOB_DIR=$VOB_DIR ./$SERVER_RUN_AS -p $SERVER_PORT'"
  else
    $START_TSESERVER_SCRIPT \
      bash -c \
        "'cd $VOB_DIR/bin/debug && VOB_DIR=$VOB_DIR ./$SERVER_RUN_AS -p $SERVER_PORT'"
  fi
  return $?
}

stop_tseserver() {

  local kill_sig=TERM

  if [ "$SERVER_HOST" != "$(hostname)" ]
  then
    ssh $SERVER_HOST \
      killall -$kill_sig --wait --verbose tseserver 2>&1 | grep -v 'Operation not permitted'
  else
    killall -$kill_sig --wait --verbose tseserver 2>&1 | grep -v 'Operation not permitted'
  fi
}

has_tseserver_crashed() {
  grep 'A SEGV signal was caught' tseserver.log
}

report_the_next_step() {

  echo =================================================================
  echo \* $(basename $0): "$@"
  echo =================================================================
}

run_hooks() {

  for hook in $HOOK_DIR/$1/*
  do
    if [ -f $hook -a -x $hook ] ; then
      report_the_next_step "$1-hook $(basename $hook)"

      $hook || return 1
    fi
  done
  return 0
}

run_pre_hooks() {
  run_hooks pre
}

run_post_hooks() {
  run_hooks post
}

#
# --- main flow:
#

exit_code=0

# abort bisect if hook fails
run_pre_hooks || exit_code=255

if [ $exit_code = 0 ]
then
  report_the_next_step "$RUN_GMAKE"
  bash -c "$RUN_GMAKE"
  exit_code=$?
  if [ $exit_code = 0 ]
  then
    exit_code=0 # gmake OK
  elif [ $exit_code -gt 125 ] # command not found or killed by signal
  then
    exit_code=255
  else
    exit_code=$EXITCODE_ON_MAKE_ERROR
  fi
fi

if [ $exit_code = 0 -a -x $HAMMER_TEST_SCRIPT ]
then
  if [ "$CLEAN_LDC_CACHE" = yes ] ; then
    report_the_next_step "clean up ldc cache..."
    rm -rfv $VOB_DIR/bin/ldc
  fi

  report_the_next_step "starting tseserver..."
  start_tseserver
  if [ $? != 0 ]
  then
    exit_code=$EXITCODE_ON_SERVER_CANNOT_START
  else
    report_the_next_step "run hammer test"
    $HAMMER_TEST_SCRIPT --host $SERVER_HOST --port $SERVER_PORT
    exit_code=$?

    if has_tseserver_crashed
    then
      exit_code=$EXITCODE_ON_SERVER_CRASH
    else
      report_the_next_step "stopping tseserver (wat a few sec)..."
      stop_tseserver
    fi
  fi
fi

# abort bisect if hook fails
run_post_hooks || exit_code=255

if [ $exit_code = 0 ]
then
  report_the_next_step 'reporting the commit as "good"'
elif [ $exit_code = 125 ]
then
  report_the_next_step 'reporting the commit to be skipped'
elif [ $exit_code -gt 125 ]
then
  report_the_next_step 'aborting bisect process'
else
  report_the_next_step 'reporting the commit as "bad"'
fi

exit $exit_code
