#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# reloadDB.tcl
#   Send a reload database configuration signal to the app console interface
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\] \[host:port\] \[OPTIONS...\]
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port help"
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
    set payload 1
    set payloadSize [string length $payload]
    set hdr $cmd
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa${hsize}a${payloadSize} $headerSize $payloadSize $hdr $payload]
    
    puts -nonewline $fd $cmd
    flush $fd
    puts "Waiting for a response"
    
    binary scan [read $fd 4] I headerSize
    binary scan [read $fd 4] I payloadSize
    set header [read $fd [expr $headerSize - 4]]
    return [read $fd $payloadSize]
}

set args [setOptions]
if {[info exists options(help)]} {
    printUsage
    exit 0
}
if {[llength $args] == 1} {
    set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$"
    regexp $hostPortMatch [lindex $args 0] match host port
}

if {[info exists options(host)]} {
    set host $options(host)
}
if {[info exists options(port)]} {
    set port $options(port)
}
if {(! [info exists host]) || (! [info exists port])} {
    puts stderr "Host and port are required"
    printUsage
    exit 1
}

set fd [socket $host $port]
if {! [info exists fd]} {
    puts stderr "Could not connect to $host:$port"
    exit 1
}
puts "Connected to $host:$port"
puts [readAppConsole $fd "RLDB00010000"]
close $fd
exit 0
