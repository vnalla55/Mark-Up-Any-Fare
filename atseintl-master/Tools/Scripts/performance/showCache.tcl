#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showCache.tcl
#   Use the app console interface to retrieve cache statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2007
# ------------------------------------------------------------------

set defaultPort 5006
proc printUsage {} {
    global argv0 defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
   or: [file tail $argv0] host:port \[OPTIONS...\]
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
    --match=regExp      Pattern to match cache names
    --nonzero=1         Only caches that are not empty
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port match nonzero nz"
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
            lappend args $opt
        }
    }
    return $args
}

set args [setOptions]
if {[llength $args] > 1} {
    printUsage
    exit 1
}
if {[llength $args] == 1} {
    set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$"
    if {! [regexp $hostPortMatch [lindex $args 0] match host port]} {
        set host [lindex $args 0]
    }
}

if {[info exists options(host)]} {set host $options(host)}
if {[info exists options(port)]} {set port $options(port)}
if {! [info exists host]} {
    puts "Missing host name"
    printUsage
    exit 1
}
if {! [info exists port]} {set port $defaultPort}

set fd [socket $host $port]
if {! [info exists fd]} {
    puts "Could not connect to $host:$port"
    exit 1
}
set nonZero 0
if {[info exists options(nonzero)]} {
    set nonZero $options(nonzero)
}
if {[info exists options(nz)]} {
    set nonZero $options(nz)
}
puts "Connected to $host:$port"
set payload ""
set payloadSize 0
set hdr "TATS00010000"
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
set payload [read $fd $payloadSize]
set lines [split $payload |]
set cacheCount [expr [llength $lines] / 5]
set width 16
for {set i 0} {$i < $cacheCount} {incr i} {
    set idx [expr $i * 5]
    set data [lrange $lines $idx [expr $idx + 4]]
    set name [lindex $data 0]
    if {[string length $name] > $width} {set width [string length $name]}
}
for {set i 0} {$i < $cacheCount} {incr i} {
    set idx [expr $i * 5]
    set data [lrange $lines $idx [expr $idx + 4]]
    set name [lindex $data 0]
    if {[info exists options(match)]} {
        if {[regexp $options(match) $name] == 0} {continue}
    }
    if {$nonZero && [lindex $data 2] == 0} {continue}
    puts -nonewline "[format "%${width}s max=%7d" $name [lindex $data 1]]"
    puts -nonewline "[format { size=%7d} [lindex $data 2]]"
    puts -nonewline "[format { access=%7d} [lindex $data 3]]"
    puts -nonewline "[format { read=%9s} [lindex $data 4]]"
    set access [lindex $data 3]
    set reads [lindex $data 4]
    if {$reads > 0} {
        if {[string length $reads] <= 9} {
            puts -nonewline "[format { %6.2f%%} [expr (100.0 * ($reads- $access)) / $reads]]"
        }
    }
    puts ""
    
}

close $fd
exit 0
