#!/bin/sh
# sourced in from package.sh

function shopping_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  . deploy/apps/tseshared/package.config.sh shopping ${builddir} ${target}
}

shopping_components $*
