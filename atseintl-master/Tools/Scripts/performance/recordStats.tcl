#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# recordStats.tcl
#   Use the app console interface to retrieve server statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set defaultPort 5002
set defaultDelay 60
proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
    --output=file       Output file (default is stdout)
    --delay=seconds     Delay between measurements (default=$defaultDelay)
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port output delay"
    for {set i 0} {$i < [llength $argv]} {incr i} {
        set tag [string range [lindex $argv $i] 0 1]
        set opt [split [lindex $argv $i] "="]
        if {[llength $opt] == 2 && [string compare $tag "--"] == 0} {
            set optName "[string range [lindex $opt 0] 2 end]"
            set options($optName) [lindex $opt 1]
            if {[string first $optName $allOptions] < 0} {
                puts stderr "Unknown option: $optName"
                printUsage
                exit 1
            }
        } else {
            puts "Invalid option: [lindex $argv $i]"
            printUsage
            exit 1
        }
    }
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
    binary scan [read $fd 4] I headerSize
    binary scan [read $fd 4] I payloadSize
    set header [read $fd [expr $headerSize - 4]]
    return [read $fd $payloadSize]
}

setOptions
if {! [info exists options(host)]} {
    puts "Missing host name"
    printUsage
    exit 1
}
set host $options(host)
if {[info exists options(port)]} {
    set port $options(port)
} else {
    set port $defaultPort
}
if {[info exists options(output)]} {
    set outFileName $options(output)
    set outFile [open $outFileName a]
    if {! [info exists outFile]} {
        puts stderr "Can't open output file: $outFileName"
        exit 1
    }
} else {
    set outFile stdout
}
set delay [expr $defaultDelay * 1000]
if {[info exist options(delay)]} {set delay [expr $options(delay) * 1000]}

proc checkInput {} {
    global line exitFlag
    puts $line
    set ch [read stdin 1]
    if {[string match "\[qQ\]" $ch]} {set exitFlag 1}
    if {[string length $ch] == 0} {
        puts stderr "Zero length string on stdin"
        fileevent stdin readable ""
    } else {
        puts stderr "Type Q to quit"
    }
}

set exitFlag 0
fileevent stdin readable {checkInput}

while {1 == 1} {
    set fd [socket $host $port]
    if {! [info exists fd]} {
        puts stderr "Could not connect to $host:$port"
        exit 1
    }
    set stats [string trim [readAppConsole $fd "RFSH00010000"] "\000"]
    set elapsed [string trim [readAppConsole $fd "LAPS00010000"] "\000"]
    close $fd
    set line "[clock format [clock seconds] -f {%Y-%m-%d %H:%M:%S}] ${stats}${elapsed}"
    puts $outFile $line
    flush $outFile
    if {$exitFlag} {break}
    update
    after $delay
}
