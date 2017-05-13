#!/bin/sh
# set -x

# { Changes to support graceful shutdown of the v2 application
load_app_config

# -- v2 monitor script --
for SVC in ${ADMIN_SERVICES} ; do
  if [ -x v2mon.${SVC} ]; then
    say "Stopping v2mon for ${SVC}"
    ./v2mon.${SVC} stop ${SVC}
  fi
done

# } END
