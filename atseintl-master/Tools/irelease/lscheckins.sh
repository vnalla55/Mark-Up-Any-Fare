#!/bin/bash

CLEARTOOL="/usr/atria/bin/cleartool"
VOB="/vobs/atse-intl-proj"

if [ "$#" -lt "1" ]
then
  echo "Usage: $0 [{old-baseline | -pre}] new-baseline"
  exit 2
fi

# Default old baseline
BL_OLD="-pre"

if [ "$#" -ge "2" ]
then
  BL_OLD="$1"
  shift
fi

# Allow user to enter old baseline as "-pre" explicitly
if [ "$BL_OLD" != "-pre" ]
then
  BL_OLD="${BL_OLD}@${VOB}"
fi

BL_NEW_SUB="${1}"
BL_NEW="${BL_NEW_SUB}@${VOB}"

echo "<html><style>table.lsci { background-color: #d1e4ca; border-collapse: collapse; } table.lsci td { border: 1px solid black; font-size: 8.5pt }</style><body>"
echo "<h3>Files modified in baseline ${BL_NEW_SUB}</h3>"
echo "<table class=\"lsci\">"
$CLEARTOOL diffbl -ver $BL_OLD $BL_NEW | grep '/vobs/atseintl' | grep -v 'Tools/irelease' | awk '/@@/{print $2}' | \
    xargs -i{} $CLEARTOOL desc -fmt "<tr><td>%Fu</td><td>%Nd</td><td>%En</td><td>%Nc</td></tr>\n" {} | \
    sed 's/[^>]*\/vobs\/atseintl\///'
echo "</table></body></html>"
