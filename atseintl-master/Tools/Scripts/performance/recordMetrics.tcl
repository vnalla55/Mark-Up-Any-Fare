#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# recordMetrics.tcl
#   Parse a tsemetrics.log file and extract transaction statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set defaultIn "/opt/atseintl/Historical/historical/tsemetrics.log"
proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --input=filename    Name of the log file (historical tsemetrics.log)
    --output=filename   Output file (default is stdout)
    --delay=seconds     Delay between measurements (60)
"
}

proc setOptions {} {
    global options argv
    set allOptions "input output delay"
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
            puts stderr "Invalid option: [lindex $argv $i]"
            printUsage
            exit 1
        }
    }
}

setOptions
if {[info exists options(input)]} {
    set inFileName $options(input)
} else {
    set inFileName $defaultIn
}
if {! [file isfile $inFileName]} {
    puts stderr "Input file: $inFileName not found"
    exit 1
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
set delay 60000
if {[info exist options(delay)]} {set delay [expr $options(delay) * 1000]}

proc checkInput {} {
    global line TrxCount
    set ch [read stdin 1]
    if {[string length $ch] == 0} {
        puts "Read 0 characters from stdin"
        fileevent stdin readable ""
        return
    }
    puts stderr $line
    if {[string match "\[qQ\]" $ch]} {exit 0}
    puts stderr "$TrxCount transactions, Type Q to quit"
}

fileevent stdin readable {checkInput}

set cursor 0
set cleanUp 0
set Trx 0
set TrxCount 0
set num "\[0-9\]+"
set alnum "\[A-Za-z0-9\]+"
set flt "$num\\.$num"
set sep "\[ \t|\]+"
set space "\[ \t\]+"
set datematch "20\[0-9\]\[0-9\]-\[0-9\]\[0-9\]-\[0-9\]\[0-9\]"
set timematch "\[0-9\]\[0-9\]:\[0-9\]\[0-9\]:\[0-9\]\[0-9\]"
set msmatch "\\.\[0-9\]+"
set timestamp "${datematch} ${timematch}\[,0-9\]*:\[ \t\]*"
set TrxMatch "\[ \t]+Trx${sep}($num)${sep}($num)${sep}($flt)${sep}($flt)"
while {1 == 1} {
    if {[file size $inFileName] < $cursor} {
        # log file resarted check for a rollover file and finish it
        set rollFile "${inFileName}.1"
        if {[file isfile $rollFile] && [file size $rollFile] > $cursor} {
            set cleanUp 1
            set inFileSave $inFileName
            set inFileName $rollFile
        } else {
            set cursor 0
        }
    }
    if {[info exists fd]} {unset fd}
    set fd [open $inFileName r]
    if {! [info exists fd]} {break}
    if {$cursor > 0} {seek $fd $cursor}
    while {! [eof $fd]} {
        set line [gets $fd]
        if {[eof $fd]} {break}
        if {[regexp "^${timestamp}($num)" $line match t]} {
            set Trx $t
        }
        if {[string first "TRAVEL SEGMENTS:" $line] >= 0} {
            regexp "TRAVEL SEGMENTS:${space}(${num})" $line match segs($Trx)
        }
        if {[string first "FARE MARKETS:" $line] >= 0} {
            regexp "FARE MARKETS:${space}(${num})" $line match markets($Trx)
        }
        if {[string first "PAX TYPES:" $line] >= 0} {
            regexp "PAX TYPES:${space}(${num})" $line match paxTypes($Trx)
        }
        if {[string first "PAXTYPEFARES:" $line] >= 0} {
            regexp "PAXTYPEFARES:${space}(${num})" $line match fares($Trx)
        }
        if {[string first "Start Time:" $line] >= 0} {
            regexp "^(${datematch} ${timematch})" $line match time($Trx)
            if {[regexp "Start Time:\[ ']*(${datematch}\[ T\]*${timematch})(${msmatch})" $line match dt startMs($Trx)]} {
                regsub "T" $dt " " $dt
                set start($Trx) [clock scan $dt]
            }
        }
        if {[string first "PNR:" $line] >= 0} {
            regexp "PNR:\[ \]*'(${alnum})'" $line match PNR($Trx)
        }
        if {[string first "LNIATA:" $line] >= 0} {
            regexp "LNIATA:\[ \]*'(${alnum})'" $line match LNIATA($Trx)
        }
#        if {[string first "End Time:" $line] >= 0} {
#            regexp "^(${datematch} ${timematch})" $line match time($Trx)
#            if {[regexp "End Time:\[ ']*(${datematch}\[ T\]*${timematch})(${msmatch})" $line match dt endMs($Trx)]} {
#                regsub "T" $dt " " $dt
#                set end($Trx) [clock scan $dt]
#            }
#        }
        if {[string first "    Trx" $line] >= 0} {
            if {! [info exists segs($Trx)]} {set segs($Trx) 0}
            if {! [info exists markets($Trx)]} {set markets($Trx) 0}
            if {! [info exists paxTypes($Trx)]} {set paxTypes($Trx) 0}
            if {! [info exists fares($Trx)]} {set fares($Trx) 0}
            if {! [info exists start($Trx)]} {set start($Trx) 0}
#            if {! [info exists end($Trx)]} {set end($Trx) 0}
            if {! [info exists time($Trx)]} {set time($Trx) "0000-00-00 0:00:00,000"}
            if {! [info exists PNR($Trx)]} {set PNR($Trx) xxxxxx}
            if {! [info exists LNIATA($Trx)]} {set LNIATA($Trx) xxxxxx}
            regexp $TrxMatch $line match ok error elapsed cpu
            puts -nonewline $outFile "$time($Trx) "
            puts -nonewline $outFile [format "%9.4f %9.4f " $elapsed $cpu]
#            if {$start($Trx) != 0 && $end($Trx) != 0} {
#                set diff [expr ($end($Trx) - $start($Trx)) + ($endMs($Trx) - $startMs($Trx))]
#            } else {
#                set diff $elapsed
#            }
#            puts -nonewline $outFile [format "%9.4f %1s %1s " $diff $ok $error]
            puts -nonewline $outFile [format "%1s %1s " $ok $error]
            puts -nonewline $outFile [format "%2d %4d " $segs($Trx) $markets($Trx)]
            puts -nonewline $outFile [format "%4s %6s " $paxTypes($Trx) $fares($Trx)]
            puts $outFile [format "%6s %6s " $PNR($Trx) $LNIATA($Trx)]
            flush $outFile
            incr TrxCount
            unset segs($Trx)
            unset markets($Trx)
            unset paxTypes($Trx)
            unset fares($Trx)
            unset start($Trx)
#            unset end($Trx)
            unset time($Trx)
            unset PNR($Trx)
            unset LNIATA($Trx)
            if {[info exists startMs($Trx]} {unset startMs($Trx)}
#            if {[info exists endMs($Trx)]} {unset endMs($Trx)}
        }
    }
    set cursor [tell $fd]
    close $fd
    if {$cleanUp} {
        set inFileName $inFileSave
        set cursor 0
        set cleanUp 0
    } else {
        after $delay
    }
    update 
}
