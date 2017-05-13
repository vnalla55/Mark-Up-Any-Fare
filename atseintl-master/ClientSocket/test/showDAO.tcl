#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showDAO.tcl
#   Use the app console interface to retrieve DAO coverage statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
   or: [file tail $argv0] \[OPTIONS...\] host:port \[OPTIONS...\] 
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=5002)
    --match=regExp      Pattern to match dao names
    --nonzero=1         Only daos that are not empty
    --labels            Label each row
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port match nonzero nz labels"
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
        set fd [open [lindex $args 0]]
        if {! [info exists fd]} {
            puts stderr "host:port or ior file not found: [lindex $args 0]"
            printUsage
            exit 1
        }
        set line ""
        set line [gets $fd]
        close $fd
        if { ! [regexp $hostPortMatch $line match host port]} {
            puts stderr "Invalid ior file: [lindex $args 0]"
            printUsage 
            exit 1
        }
    }
} else {
    if {! [info exists options(host)]} {
        puts "Missing host name"
        printUsage
        exit 1
    }
    set host $options(host)
    if {[info exists options(port)]} {
        set port $options(port)
    } else {
        set port 5002
    }
}
    
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
if {[info exists options(labels)]} {set labels $options(labels)} else {set labels 0}
puts "Connected to $host:$port"
set payload ""
set payloadSize 0
set hdr "DAOC00010000"
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

# puts $payload
set lines [split $payload |]
set daoCount [expr [llength $lines] / 5]
set width 16
for {set i 0} {$i < $daoCount} {incr i} {
    set idx [expr $i * 5]
    set data [lrange $lines $idx [expr $idx + 4]]
    set name [lindex $data 0]
    if {[string length $name] > $width} {set width [string length $name]}
}
set fieldNames "loadCall getCall getAllCall getFDCall"
foreach field $fieldNames {set total($field) 0}
if {! $labels} {
    puts -nonewline [format "%${width}s" "DAO name"]
    puts "      load       get    getAll     getFD"
}

for {set i 0} {$i < $daoCount} {incr i} {
    set idx [expr $i * 5]
    set data [lrange $lines $idx [expr $idx + 4]]
    set name [lindex $data 0]
    if {[info exists options(match)]} {
        if {[regexp $options(match) $name] == 0} {continue}
    }
    set j 1
    foreach field $fieldNames {
        set $field [lindex $data $j]
        incr j
    }
    if {$nonZero && [expr $loadCall + $getCall + $getAllCall + $getFDCall] == 0} {continue}
    if {$labels} {
        puts -nonewline "[format "%${width}s loadCall=%9d" $name $loadCall]"
        puts -nonewline "[format { gets=%9d} $getCall]"
        puts -nonewline "[format { getAlls=%9d} $getAllCall]"
        puts -nonewline "[format { getFD=%9d} $getFDCall]"
    } else {
        puts -nonewline "[format "%${width}s %9d" $name $loadCall]"
        puts -nonewline "[format { %9d} $getCall]"
        puts -nonewline "[format { %9d} $getAllCall]"
        puts -nonewline "[format { %9d} $getFDCall]"
    }
    puts ""
    set j 1
    foreach field $fieldNames {
        set total($field) [expr $total($field) + [lindex $data $j]]
        incr j
    }
}

if {$total(getCall) > 0} {
    puts " Total loads: [format {%8d} $total(loadCall)]"
    puts "  Total gets: [format {%8d} $total(getCall)]"
    puts "Total getAll: [format {%8d} $total(getAllCall)]"
    puts " Total getFD: [format {%8d} $total(getFDCall)]"
}
close $fd
exit 0

