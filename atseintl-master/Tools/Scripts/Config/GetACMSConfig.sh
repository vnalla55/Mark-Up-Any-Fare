#!/bin/ksh


if [[ $# -ne 3 ]]; then
   echo -e >&2 "\n** Error: Missing/Incorrect parameters **\n"
   echo >&2 "Usage: $0 release_label application environment"
   echo -e >&2 "\nApplication must be one of these:"
   echo >&2 "shopping|shoppingis|shoppinghist|shoppingesv|pricingv2|faredisplay|tax|historical (also shopping_d and shoppingis_d in prod)"
   echo -e >&2 "\nEnvironment should be one of these:"
   echo -e >&2 "prod|cert|integ|dev|daily|plab"
   echo -e >&2 "\nExample:"
   echo -e >&2 "$0 atsev2.2010.07.00 historical prod"
   echo -e >&2 "$0 atsev2.2010.06.01 shoppingis plab\n"

   exit 1
fi

APP=$2
if [ "$APP" = "pricing" ] ; then
  APP="pricingv2"
fi

/opt/atse/common/acmsquery/acmsquery.sh -svchost hybd05 -svcport 47030 -baseline $1 -user snpfunc -FAM atsev2 -APP $APP -ENV $3
