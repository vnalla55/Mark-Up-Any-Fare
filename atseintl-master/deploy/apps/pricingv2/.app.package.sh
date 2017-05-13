#!/bin/sh
# sourced in from package.sh

function pricingv2_components
{
  local osdir=${1}
  local builddir=${2}
  local target=${3}

  . deploy/apps/tseshared/package.config.sh pricingv2 ${builddir} ${target} pricing
}

pricingv2_components $*
