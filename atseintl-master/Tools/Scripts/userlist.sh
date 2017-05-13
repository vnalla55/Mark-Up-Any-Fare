#!/bin/ksh

# get the list of users for the current system.
/usr/bin/w -h | /usr/bin/awk '{print $1;}' | sort -u
