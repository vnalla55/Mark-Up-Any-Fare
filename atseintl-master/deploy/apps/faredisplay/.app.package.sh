#!/bin/sh
# sourced in from package.sh

function faredisplay_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  . deploy/apps/tseshared/package.config.sh faredisplay ${builddir} ${target}
}

faredisplay_components $*
