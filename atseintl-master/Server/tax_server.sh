#!/bin/bash

export ACMS_APP_NAME=tax
export ACMS_DMN_ENV=daily

$(dirname $0)/acms_server.sh "$@"

