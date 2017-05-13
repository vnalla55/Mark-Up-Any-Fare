#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showDAOs.tcl
#   Use the app console interface to retrieve DAO coverage statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set defaultPort 5006
set printTotals 0

proc printUsage {} {
    global argv0 defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
   or: [file tail $argv0] \[OPTIONS...\] host:port \{host:port ... \} \[OPTIONS...\] 
Options:
    --hosts=host:port,host,port,...
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
    --match=regExp      Pattern to match dao names
    --nonzero=1         Only DAOs that are not empty
    --zero=1            Only DAOs that are empty
    --labels            Label each row
    --verbose           Print status messages
"
}

set allOptions "hosts host port match nonzero nz zero labels verbose"
proc setOptions {allOptions} {
    global options argv
    set args ""
    for {set i 0} {$i < [llength $argv]} {incr i} {
        set tag [string range [lindex $argv $i] 0 1]
        set opt [split [lindex $argv $i] "="]
        set len [llength $opt]
        if {($len == 2 || $len == 1) && [string compare $tag "--"] == 0} {
            set optName "[string range [lindex $opt 0] 2 end]"
            if {$len == 2} {set value [split [lindex $opt 1] ","]} else {set value 1}
            if {[info exists options($optName)]} {
                append options($optName) $value
            } else {
                set options($optName) $value
            }
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

set args [setOptions $allOptions]
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    } else {
        set $option 0
    }
}
if {[info exists options(nz)]} {set nonzero $options(nz)}

set hostList ""
foreach arg $args {
    if {[regexp "^\[A-Za-z0-9\]+:\[0-9\]+$" $arg]} {
        lappend hostList $arg
    } else {
        puts stderr "Invalid host:port '$arg'"
    }
}
if {[string compare $hosts "0"] != 0} {
    foreach arg $hosts {
        if {[regexp "^\[A-Za-z0-9\]+:\[0-9\]+$" $arg]} {
            lappend hostList $arg
        } else {
            puts stderr "Invalid host:port '$arg'"
        }
    }
}
if {[string compare $host "0"] != 0} {
    set idx 0
    set portCount [llength $port]
    foreach arg $host {
        if {[regexp "^\[A-Za-z0-9\]+:\[0-9\]+$" $arg]} {
            lappend hostList $arg
        } else {
            if {$idx <= $portCount} {
                lappend hostList "$arg:$defaultPort"
            } else {
                lappend hostList "$arg:[lindex port $idx]"
            }
        }
    incr idx
    }
}
if {[llength $hostList] < 1} {
    puts stderr "Missing host and port"
    printUsage
    exit 1
}

proc readAppConsole {fd cmd} {
    global verbose
    set payload ""
    set payloadSize 0
    set hdr $cmd
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]
    puts -nonewline $fd $cmd
    flush $fd
    if {$verbose} {puts "Waiting for a response"}
    binary scan [read $fd 4] I headerSize
    binary scan [read $fd 4] I payloadSize
    set header [read $fd [expr $headerSize - 4]]
    return [string trim [read $fd $payloadSize] "\000"]
}

proc readHost {hostPort cmd} {
    global verbose
    set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$"
    if {! [regexp $hostPortMatch $hostPort match host port]} {
        puts stderr "invalid host:port '$hostPort'"
        return -1
    }
    set host [string trim $host]
    set port [string trim $port]
    set fd [socket $host $port]
    if {! [info exists fd]} {
        puts stderr "Could not connect to $host:$port"
        return -2
    }
    if {$verbose} {puts "Connected to $host:$port"}
    set result [readAppConsole $fd $cmd]
    close $fd
    return $result
}

proc addCounts payload {
    global counts
    set lines [split $payload |]
    set daoCount [expr [llength $lines] / 5]
    for {set i 0} {$i < $daoCount} {incr i} {
        set idx [expr $i * 5]
        set name [lindex $lines $idx]
        incr idx
        set data [lrange $lines $idx [expr $idx + 3]]
        if {! [info exists counts($name)]} {
            set counts($name) "0 0 0 0"
        }
        set c $counts($name)
        set new ""
        for {set j 0} {$j < 4} {incr j} {
            set newCount [expr [lindex $c $j] + [lindex $data $j]]
            lappend new $newCount
            incr idx
        }
        set counts($name) $new
    }
}

set coverageCmd "DAOC00010000"
foreach hostPort $hostList {
    set payload [readHost $hostPort $coverageCmd]
    addCounts $payload
}

set daoNames [lsort [array names counts]]
set width 16
foreach name $daoNames {
    if {[string length $name] > $width} {set width [string length $name]}
}

puts [clock format [clock seconds] -format "%Y-%m-%d %H:%M:%S"]
set fieldNames "loadCall getCall getAllCall getFDCall"
foreach field $fieldNames {set total($field) 0}
if {! $labels} {
    puts -nonewline [format "%${width}s" "DAO name"]
    puts "      load       get    getAll     getFD"
}

foreach name $daoNames {
    if {[info exists options(match)]} {
        if {[regexp $options(match) $name] == 0} {continue}
    }
    set data $counts($name)
    set j 0
    foreach field $fieldNames {
        set $field [lindex $data $j]
        incr j
    }
    if {$nonzero && [expr $loadCall + $getCall + $getAllCall + $getFDCall] == 0} {continue}
    if {$zero && [expr $loadCall + $getCall + $getAllCall + $getFDCall] > 0} {continue}
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
    set j 0
    foreach field $fieldNames {
        set total($field) [expr $total($field) + [lindex $data $j]]
        incr j
    }
}

if {$total(getCall) > 0 && $printTotals} {
    puts " Total loads: [format {%8d} $total(loadCall)]"
    puts "  Total gets: [format {%8d} $total(getCall)]"
    puts "Total getAll: [format {%8d} $total(getAllCall)]"
    puts " Total getFD: [format {%8d} $total(getFDCall)]"
}
exit 0
