#!/bin/bash

export ACMS_APP_NAME=shoppinghist
export ACMS_DMN_ENV=dev

$(dirname $0)/acms_server.sh "$@"

