#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showRelease.tcl
#   Use the app console interface to get details
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0 defaultHost defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --host=hostname     Host name e.g. atsela04 (default=$defaultHost)
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port"
    set args ""
    for {set i 0} {$i < [llength $argv]} {incr i} {
        set tag [string range [lindex $argv $i] 0 1]
        set opt [split [lindex $argv $i] "="]
        set len [llength $opt]
        if {($len == 2 || $len == 1) && [string compare $tag "--"] == 0} {
            set optName "[string range [lindex $opt 0] 2 end]"
            if {$len == 2} {set value [lindex $opt 1]} else {set value 1}
            set options($optName) $value
            if {[string first $optName $allOptions] < 0} {
                puts stderr "Unknown option: $optName"
                printUsage
                exit 1
            }
        } else {
            lappend args [lindex $argv $i]
        }
    }
    return $args
}

proc readAppConsole {fd cmd} {
    set payload ""
    set payloadSize 0
    set hdr $cmd
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]

    puts -nonewline $fd $cmd
    flush $fd
    puts "Waiting for a response"

    binary scan [read $fd 4] I headerSize
    binary scan [read $fd 4] I payloadSize
    set header [read $fd [expr $headerSize - 4]]
    return [read $fd $payloadSize]
}

set defaultHost picli405
set defaultPort 5006
set args [setOptions]
if {[info exists options(host)]} {
    set host $options(host)
} else {
    set host $defaultHost
}
if {[info exists options(port)]} {
    set port $options(port)
} else {
    set port $defaultPort
}

set fd [socket $host $port]
if {! [info exists fd]} {
    puts stderr "Could not connect to $host:$port"
    exit 1
}
puts "Connected to $host:$port"
set details [split [readAppConsole $fd "ETAI00010000"] "|"]
if {[llength $details] >= 1} {
    puts "Process: [lindex $details 0]"
}
if {[llength $details] >= 2} {
    puts "  Owner: [lindex $details 1]"
}
if {[llength $details] >= 3} {
    puts "Release: [lindex $details 2]"
}
if {[llength $details] >= 4} {
    puts "Port: [lindex $details 3]"
}
close $fd
exit 0
