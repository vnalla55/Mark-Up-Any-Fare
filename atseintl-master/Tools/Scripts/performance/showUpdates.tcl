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
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
    --match=regExp      Pattern to match cache names
    --nonzero=1         Only caches that are not empty
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
if {[info exists options(labels)]} {set labels $options(labels)} else {set labels 0}
puts "Connected to $host:$port"
set payload ""
set payloadSize 0
set hdr "UPDT00010000"
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
set cacheCount [expr [llength $lines] / 8]
set width 16
for {set i 0} {$i < $cacheCount} {incr i} {
    set idx [expr $i * 8]
    set data [lrange $lines $idx [expr $idx + 7]]
    set name [lindex $data 0]
    if {[string length $name] > $width} {set width [string length $name]}
}
set fieldNames "updates flushes deletes noneDeleted cpuTime insert remove"
foreach field $fieldNames {set total($field) 0}
if {! $labels} {
    puts -nonewline [format "%-${width}s" "Cache name"]
    puts "    update  delete   flush    cpuTime NoneDeleted insert  remove"
}

for {set i 0} {$i < $cacheCount} {incr i} {
    set idx [expr $i * 8]
    set data [lrange $lines $idx [expr $idx + 7]]
    set name [lindex $data 0]
    if {[info exists options(match)]} {
        if {[regexp $options(match) $name] == 0} {continue}
    }
    set j 1
    foreach field $fieldNames {
        set $field [lindex $data $j]
        incr j
    }
    if {$nonZero && $updates == 0 && $flushes == 0} {continue}
    if {$labels} {
        puts -nonewline "[format "%${width}s updates=%7d" $name $updates]"
        puts -nonewline "[format { deletes=%7d} $deletes]"
        puts -nonewline "[format { flushes=%7d} $flushes]"
        puts -nonewline "[format { CpuTime=%12.03f} $cpuTime]"
        puts -nonewline "[format { none=%7d} $noneDeleted]"
        if {[expr ($insert + $remove) > 0]} {
            puts -nonewline "[format { rowsInserted=%7d} $insert]"
            puts -nonewline "[format { rowsRemoved=%7d} $remove]"
        }
    } else {
        puts -nonewline "[format "%${width}s %7d" $name $updates]"
        puts -nonewline "[format { %7d} $deletes]"
        puts -nonewline "[format { %7d} $flushes]"
        puts -nonewline "[format { %12.03f} $cpuTime]"
        puts -nonewline "[format { %7d} $noneDeleted]"
        if {[expr ($insert + $remove) > 0]} {
            puts -nonewline "[format { %7d} $insert]"
            puts -nonewline "[format { %7d} $remove]"
        }
    }
    puts ""
    set j 1
    foreach field $fieldNames {
        set total($field) [expr $total($field) + [lindex $data $j]]
        incr j
    }
}

if {$total(updates) > 0} {
    puts " Total updates: [format {%8d} $total(updates)]"
    puts " Total deletes: [format {%8d} $total(deletes)]"
    puts " Total flushes: [format {%8d} $total(flushes)]"
    puts "Total CPU time: [format {%8.3f} $total(cpuTime)]"
    puts "Total updates with no data removed: [format {%8d} $total(noneDeleted)]"
}
close $fd
exit 0
