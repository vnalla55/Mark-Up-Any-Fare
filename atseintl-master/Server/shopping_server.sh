#!/bin/bash

export ACMS_APP_NAME=shopping
export ACMS_DMN_ENV=dev
# if you need to get daily use value "daily"
# for integration use "integ"
# for certification use "cert"

$(dirname $0)/acms_server.sh "$@"

