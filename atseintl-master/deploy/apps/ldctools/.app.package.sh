#!/bin/sh
# sourced in from package.sh

function ldctools_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  cp_func "Tools/ldc/${builddir}/cachetest"       ${target}
  cp_func "Tools/ldc/${builddir}/listBDB"         ${target}
  cp_func "Tools/ldc/${builddir}/cacheCompare"    ${target}
  cp_func "Tools/ldc/${builddir}/listCacheEvents" ${target}
  cp_func "Tools/ldc/${builddir}/bdbCompare"      ${target}
  cp_func "Tools/ldc/${builddir}/readTest"        ${target}
  cp_func "Tools/ldc/${builddir}/threadTest"      ${target}

  cp_func "Tools/ldc/cachetest.sh"            ${target}
  cp_func "Tools/ldc/dumpkeys.sh"             ${target}
  cp_func "Tools/ldc/package.sh"              ${target}
  cp_func "Tools/ldc/testnotify.sh"           ${target}

  cp_func "Tools/ldc/cacheNotify.xml"         ${target} L
  cp_func "Tools/ldc/config_functions.sh"     ${target} L
}

ldctools_components $*
