#!/bin/sh
# sourced in from package.sh

function tseshared_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  cp_func "Xform/currencyRequest.cfg"                      ${target}
  cp_func "Xform/detailXmlConfig.cfg"                      ${target}
  cp_func "Xform/fareDisplayRequest.cfg"                   ${target}
  cp_func "Xform/mileageRequest.cfg"                       ${target}
  cp_func "Xform/pricingDetailRequest.cfg"                 ${target}
  cp_func "Xform/pricingDisplayRequest.cfg"                ${target}
  cp_func "Xform/pricingRequest.cfg"                       ${target}
  cp_func "Xform/taxDisplayRequest.cfg"                    ${target}
  cp_func "Xform/taxOTARequest.cfg"                        ${target}
  cp_func "Xform/taxRequest.cfg"                           ${target}
  cp_func "Xform/xmlConfig.cfg"                            ${target}
  cp_func "Xform/pfcDisplayRequest.cfg"                    ${target}
  cp_func "Xform/taxInfoRequest.cfg"                       ${target}

  cp_func "Server/asap.ini"                                ${target}
  cp_func "Server/cacheNotify.xml"                         ${target}
  cp_func "Server/tsi.ini"                                 ${target}

  cp_func "bin/${builddir}/tseserver"                      ${target}
  cp_func "bin/${builddir}/tseserverje"                    ${target}

  cp_func "deploy/integ/config/bin/log4crc"                ${target}
  cp_func "deploy/integ/config/bin/log4cxx.xml"            ${target}

  cp_func "Tools/Scripts/core_analysis/analyseCore"        ${target}
  cp_func "Tools/Scripts/core_analysis/processCore"        ${target}
  cp_func "Tools/Scripts/core_analysis/lastRequests"       ${target}
  cp_func "Tools/Scripts/core_analysis/lastRequests.awk"   ${target}

  mkdir -p                                                 ${target}/lib
  cp_func "lib/${builddir}/*"                              ${target}/lib

  # only DevHybrid gets these next ones

  mkdir -p                                                 ${target}/tabledefs.DevHybrid
  cp_func "bin/debug/mysql.*.ini"                          ${target}/tabledefs.DevHybrid
}

tseshared_components $*



