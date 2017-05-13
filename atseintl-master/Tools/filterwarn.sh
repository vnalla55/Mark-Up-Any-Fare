#!/bin/sh

set -e
set -o pipefail

# This magic swaps stdout and stderr, then pipes stdout to seds and than swaps back
# Probably it can be optimized to run sed only once with a single script
# But, to be honest it much more "readable" in this way.

# perl: also filter out 'instantiated from here' at the end of a line
#       if the previous line was with 'boost... archive|serializ'
"$@" 3>&1 1>&2 2>&3 | perl -ne '($x,$y)=($y,/boost.+(archive|serializ[ea])/); print if !$y && !(/instantiated from here$/ && $x)' 4>&1 1>&2 2>&4
