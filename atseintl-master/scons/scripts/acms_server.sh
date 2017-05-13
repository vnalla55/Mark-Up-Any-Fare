#!/bin/bash

export ACMS_SUPPORT=Y

NB_OF_SHIFTS=0
while getopts ":b:u:a:e:c:dnhPTF" opt; do
  case $opt in
    b)
      export ACMS_BSL=$OPTARG
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 2)
      ;;
    u)
      export ACMS_USER=$OPTARG
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 2)
      ;;
    a)
      export ACMS_ACT_LIST=$OPTARG
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 2)
      ;;
    e)
      export ACMS_DMN_ENV=$OPTARG
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 2)
      ;;
    d)
      export DEBUG_TSESERVER=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 1)
      ;;
    n)
      export NO_ACMS_DOWNLOAD=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 1)
      ;;
    c)
      export CONFIG_FILE=$OPTARG
      export NO_ACMS_DOWNLOAD=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 2)
      ;;
    P)
      export START_NON_HISTORICAL=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 1)
      echo "Starting pricing server"
      ;;
    T)
      export START_TAX=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 1)
      echo "Starting tax server"
      ;;
    F)
      export START_FD=Y
      NB_OF_SHIFTS=$(expr ${NB_OF_SHIFTS} + 1)
      echo "Starting faredisplay server"
      ;;
    h)
      echo
      echo " This script starts ${ACMS_APP_NAME} application with support for ACMS"
      echo " Available options (all optional):"
      echo " -u param : user name different than your login"
      echo " -b param : baseline different than the latest baseline"
      echo " -a param : activity list; separator: |"
      echo " -e param : for which environment to get config; dev is default"
      echo "            (dev, daily, integ, cert)"
      echo " -d : debug; starts application with totalview"
      echo " -n : do not download tseserver.acms.cfg, just use the existing one"
      echo " -c : do not query ACMS for config, use provided one instead"
      echo " -h : help"
      echo
      exit 1
      ;;
  esac
done

for (( i=0; i<${NB_OF_SHIFTS}; i++ ))
do
  shift
done

$(dirname $0)/server.sh $*
