#!/bin/sh

RELEASE=

if [ z"$BUILD" = z ]; then
   RELEASE=debug
else
   RELEASE=$BUILD
fi

echo 'Setting release to '$RELEASE

. ../../Server/set_ld_path.sh
