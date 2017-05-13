#!/bin/sh
# sourced in from package.sh

function shoppingesv_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  . deploy/apps/tseshared/package.config.sh shoppingesv ${builddir} ${target}

  cp_func "Server/VIS_Beta.txt" ${target}
}

shoppingesv_components $*
