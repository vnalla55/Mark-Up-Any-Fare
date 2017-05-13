#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# curDate.tcl
#   Scan a file and set the D07 tags to the current date
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] <xmlFile >result
"
}

set datematch "\"?20\[0-9\]\[0-9\]-\[0-9\]\[0-9\]-\[0-9\]\[0-9\]\"?"
set tags [list D07]
if {[llength $argv] > 1} {
    printUsage
    exit 1
}
set today [clock format [clock seconds] -format "%Y-%m-%d"]
while {! [eof stdin]} {
    gets stdin line
    if {[eof stdin]} {break}
    set s [split $line]
    set len [llength $s]
    set idx 0
    foreach txt $s {
        foreach tag $tags {
            if {[regsub "${tag}=${datematch}" $txt "${tag}=\"${today}\"" new]} {
                set txt $new
            }
        }
        puts -nonewline "$txt"
        incr idx
        if {$idx < $len} {puts -nonewline " "}
    }
    puts ""
}
