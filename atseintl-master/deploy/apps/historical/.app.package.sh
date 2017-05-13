#!/bin/sh
# sourced in from package.sh

function historical_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  . deploy/apps/tseshared/package.config.sh historical ${builddir} ${target}
}

historical_components $*
