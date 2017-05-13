#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# addDate.tcl
#   Scan a file and add or subtract a constant from the dates
# Author: Jim Sinnamon
# Copyright: Sabre 2007
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] +-days <logFile >result
"
}

set dateMatch "^(.*)(20\[0-9\]\[0-9\]-\[0-9\]\[0-9\]-\[0-9\]\[0-9\])(.*)$"
if {[llength $argv] < 1} {
    printUsage
    exit 1
}
set offset [expr [lindex $argv 0] * 86400]
while {! [eof stdin]} {
    gets stdin line
    if {[eof stdin]} {break}
    set s [split $line]
    set len [llength $s]
    set idx 0
    foreach txt $s {
        if {[regexp $dateMatch $txt match before date after]} {
            set time [expr [clock scan $date] + $offset]
            puts -nonewline "${before}[clock format $time -format "%Y-%m-%d"]${after}"
        } else {
           puts -nonewline "$txt"
        }
        incr idx
        if {$idx < $len} {puts -nonewline " "}
    }
    puts ""
}
