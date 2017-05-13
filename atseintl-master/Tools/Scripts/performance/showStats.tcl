#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showStats.tcl
#   Use the app console interface to retrieve server statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set defaultPort 5006
proc printUsage {} {
    global argv0 defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
   or: [file tail $argv0] host:port \[OPTIONS...\]
   or: [file tail $argv0] file.ext \[OPTIONS...\]
Options:
    --host=hostname     Host name e.g. atsela04
    --port=portNumber   TCP port for tseserver (default=$defaultPort)
    --print=1           Formatted print (default is one line)
    --file=filename     Analyze a file from recordStats
    --database          Show database statistics
    --atLine=lineNumber Print statistics for a specific line in a file
    --plot              Prepare a datafile for gnuplot
    --plotAverage=n     Average response,cpu, etc over n samples
    --start=n           Start at line n in file
    --errors            Report error codes and counts
    --table             Generate an html table from the last entry
    --file2             Second file for table
"
}

set allOptions "host port print file database atLine plot plotAverage start errors table file2"
proc setOptions {allOptions} {
    global options argv
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

proc printAll {payload} {
    set groups [split $payload "<"]
    set number 0
    set trx 0
    set time 1
    set cpu 0
    set query 0
    foreach group $groups {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        if {[string compare $id STATS] == 0} {
            foreach item $data {
                if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                    switch $name {
                        ST {puts "     Start Time= [clock format $value -format {%Y-%m-%d %H:%M:%S}]"}
                        CP {
                            puts "       CPU Time= $value"
                            set cpu $value
                        }
                        IT {puts "    Itineraries= $value"}
                        DB {puts "Database errors= $value"}
                        SC {puts "  Socket closes= $value"}
                        CT {puts " Concurrent Trx= $value"}
                        L1 {puts " Load 1 minute = $value"}
                        L2 {puts " Load 5 minutes= $value"}
                        L3 {puts " Load 15 minute= $value"}
                        FM {puts "    Free memory= $value"}
                        RS {puts "            RSS= $value"}
                        DQ {
                            puts "    Database OK= $value"
                            set query $value
                        }
                        DH {puts "     Current DB= $value"}
                        VM {puts " Virtual Memory= $value"}
                        TH {puts "      Throttled= $value"}
                        default {puts "$item"}
                    }
                }
            }
        } else {
            puts -nonewline "$id: "
            for {set i 1} {$i < $len} {incr i} {
                set item [lindex $data $i]
                if {[regexp "(\[A-Za-z\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                    switch $name {
                        OK {
                            puts -nonewline " OK=$value"
                            if {$trx} {set number [expr $number + $value]}
                            }
                        ER {
                            puts -nonewline " Errors=$value"
                            if {$trx} {set number [expr $number + $value]}
                            }
                        ET {
                            puts -nonewline " ElapsedTime=$value"
                            if {$trx} {set time $value}
                            }
                        RQ {puts -nonewline " RequestSize=$value"}
                        RS {puts -nonewline " ResponseSize=$value"}
                        NM {
                            puts -nonewline " $value"
                            set trx [expr [string compare $value TRX] == 0]
                            }
                        default {puts -nonewline "$item"}
                    }
                }
            }
            puts ""
        }
    }
    if {$number > 1} {
        puts "Average transaction time = [format "%8.3f" [expr ${time} / $number.0]]"
        puts "        Average CPU time = [format "%8.3f" [expr ${cpu} / $number.0]]"
        puts " Average Queries per Trx = [format "%8.1f" [expr ${query} / $number.0]]"
    }
    return $number
}

proc printElapsed {payload number} {
    set dbTime 0
    set groups [split $payload "<"]
    foreach group $groups {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        if {[string compare $id ELAPSED] == 0} {
            foreach item $data {
                if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                    switch $name {
                        FC {puts " Fares Collection Service: $value"}
                        FV {puts " Fares Validation Service: $value"}
                        PO {puts "          Pricing Service: $value"}
                        SS {puts "         Shopping Service: $value"}
                        IA {puts "    Itin Analyzer Service: $value"}
                        TX {puts "              Tax Service: $value"}
                        CA {puts "        Fare Calc Service: $value"}
                        CU {puts "         Currency Service: $value"}
                        MI {puts "          Mileage Service: $value"}
                        IN {puts "         Internal Service: $value"}
                        FD {puts "     Fare Display Service: $value"}
                        FS {puts "    Fare Selector Service: $value"}
                        RF {puts "Rex Fare Selector Service: $value"}
                        FB {puts "     Free Baggage Service: $value"}
                        SF {puts "     Service Fees Service: $value"}
                        TF {puts "   Ticketing Fees Service: $value"}
                        S8 {puts "         Branding Service: $value"}
                        DB {puts "                 Database: $value"}
                        DS {
                            puts "                 Database: $value"
                            set dbTime $value
                        }
                        default {puts "$item"}
                    }
                }
            }
            if {$number > 1} {
                puts "    Average database time: [format "%7.3f" [expr ${dbTime} / ${number}.0]]"
            }
            puts ""
        }
    }
}

proc printColumns {payload} {
    global database maxVM
    set groups [split $payload "<"]
    set fields {CP IT CT L1 L2 L3 FM VM DQ DS}
    foreach field $fields {set val($field) 0}
    if {[regexp "^20" [lindex $groups 0]]} {
        set timeStamp [lindex $groups 0]
    } else {
        set timeStamp "0000-00-00 00:00:00 "
    }
    foreach group $groups {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        if {[string compare $id STATS] != 0 && [string compare $id ELAPSED] != 0} {continue}
        foreach item $data {
            if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                set val($name) $value
            }
        }
    }
    puts -nonewline $timeStamp
    puts -nonewline [format "%10.1f %8d %8d " $val(CP) $val(IT) $val(CT)]
    puts -nonewline [format "%9.5f %9.5f %9.5f " $val(L1) $val(L2) $val(L3)]
    puts -nonewline [format "%13d " $val(FM)]
    puts -nonewline [format "%13s " $val(VM)]
    if {$val(VM) > $maxVM} {set maxVM $val(VM)}
    if {$database} {puts -nonewline [format "%13d %13s" $val(DQ) $val(DS)]}
    puts ""
}

proc printPlot {line1 line2} {
    global database
    set group1 [split $line1 "<"]
    set group2 [split $line2 "<"]
    set fields {CP IT FM VM DQ DS ET RSP}
    foreach field $fields {
        set val1($field) 0
        set val2($field) 0
    }
    if {[regexp "^20" [lindex $group1 0]]} {
        set time1 [lindex $group1 0]
    } else {
        set time1 "0000-00-00 00:00:00 "
    }
    if {[regexp "^20" [lindex $group2 0]]} {
        set time2 [lindex $group2 0]
    } else {
        set time2 "0000-00-00 00:00:00 "
    }
    foreach group $group1 {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        set val1(NM) ""
        foreach item $data {
            if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                set val1($name) $value
            }
        }
        if {[string compare $id SP] == 0 && [string compare $val1(NM) TRX] == 0} {
            set val1(RSP) $val1(ET)
        }
    }
    foreach group $group2 {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        set val2(NM) ""
        foreach item $data {
            if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                set val2($name) $value
            }
        }
        if {[string compare $id SP] == 0 && [string compare $val2(NM) TRX] == 0} {
            set val2(RSP) $val2(ET)
        }
    }
    if {$val2(IT) > $val1(IT)} {
        set it [expr $val2(IT) - $val1(IT)]
        if {[string first "." $it] < 0} {append it ".0"}
        puts -nonewline $time2
        set fields {CP IT FM VM DQ DS ET RSP}
        puts -nonewline [format "%8d " $val2(IT)]
        puts -nonewline [format "%10.3f " [expr ($val2(RSP)-$val1(RSP))/$it]]
        puts -nonewline [format "%10.3f " [expr ($val2(CP)-$val1(CP))/$it]]
        puts -nonewline [format "%10.3f " [expr ($val2(DS)-$val1(DS))/$it]]
        puts -nonewline [format "%10.0f " [expr ($val2(DQ)-$val1(DQ))/$it]]
        puts -nonewline [format "%13s " $val2(FM)]
        puts -nonewline [format "%13s " $val2(VM)]
        set timediff [expr [clock scan $time2] - [clock scan $time1]]
        if {$timediff == 0} {
            set tps 0
        } else {
            set tps [expr $it / $timediff]
        }
        puts -nonewline [format "%10.3f " $tps]
        puts ""
    }
}

proc tableStats {line val} {
    global $val
    set groups [split $line "<"]
    set fields {CP IT DQ VM OK ER ET DS}
    foreach field $fields {set ${val}($field) 0}
    if {[regexp "^20" [lindex $groups 0]]} {
        set timeStamp [lindex $groups 0]
    } else {
        set timeStamp "0000-00-00 00:00:00 "
    }
    foreach group $groups {
        set data [split $group " "]
        set len [llength $data]
        if {$len <= 1} {continue}
        set id [lindex $data 0]
        if {[string compare $id STATS] == 0 || [string compare $id ELAPSED] == 0}  {
            foreach item $data {
                if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                    set ${val}($name) $value
                }
            }
        }
        if {[string compare $id SP] == 0} {
            set NM ""
            foreach item $data {
                if {[regexp "(\[A-Za-z0-9\]+)=\"(\[^\"\]*)\"" $item match name value]} {
                    if {[string compare $name NM] == 0} {set NM $value}
                    if {[string compare $NM TRX] == 0} {set ${val}($name) $value}
                }
            }
        }
    }
}

proc printTable {val1 val2} {
    global $val1 $val2
    puts "<p><table border=\"1\" cellpadding=\"2\">"
    puts "<tr><td>Transaction Count</td>"
    set TR1 [expr $${val1}(OK) + $${val1}(ER)]
    set TR2 [expr $${val2}(OK) + $${val2}(ER)]
    puts "<td align=\"right\">$TR1</td>"
    puts "<td align=\"right\">$TR2</td>"
    puts "<tr><td>Itineraries</td>"
    puts "<td align=\"right\">[expr $${val1}(IT)]</td>"
    puts "<td align=\"right\">[expr $${val2}(IT)]</td>"
    puts "<tr><td>Errors</td>"
    puts "<td align=\"right\">[expr $${val1}(ER)]</td>"
    puts "<td align=\"right\">[expr $${val2}(ER)]</td>"
    puts "<tr><td>Virtual Memory (GB)</td>"
    puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(VM) / (1024*1024*1024)]]</td>"
    puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val2}(VM) / (1024*1024*1024)]]</td>"
    if {$TR1 > 0 && $TR2 > 0} {
        puts "<tr><td>Average Response Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(ET) / $TR1]]</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val2}(ET) / $TR2]]</td>"
        puts "<tr><td>Average CPU Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(CP) / $TR1]]</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val2}(CP) / $TR2]]</td>"
        puts "<tr><td>Queries per transaction</td>"
        puts "<td align=\"right\">[format "%6.1f" [expr 1.0 * $${val1}(DQ) / $TR1]]</td>"
        puts "<td align=\"right\">[format "%6.1f" [expr 1.0 * $${val2}(DQ) / $TR2]]</td>"
        puts "<tr><td>Average Database Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(DS) / $TR1]]</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val2}(DS) / $TR2]]</td>"
    }
    puts "</table>"
}

proc print1Table {val1} {
    global $val1
    puts "<p><table border=\"1\" cellpadding=\"2\">"
    puts "<tr><td>Transaction Count</td>"
    set TR1 [expr $${val1}(OK) + $${val1}(ER)]
    puts "<td align=\"right\">$TR1</td>"
    puts "<tr><td>Itineraries</td>"
    puts "<td align=\"right\">[expr $${val1}(IT)]</td>"
    puts "<tr><td>Errors</td>"
    puts "<td align=\"right\">[expr $${val1}(ER)]</td>"
    puts "<tr><td>Virtual Memory (GB)</td>"
    puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(VM) / (1024*1024*1024)]]</td>"
    if {$TR1 > 0} {
        puts "<tr><td>Average Response Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(ET) / $TR1]]</td>"
        puts "<tr><td>Average CPU Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(CP) / $TR1]]</td>"
        puts "<tr><td>Queries per transaction</td>"
        puts "<td align=\"right\">[format "%6.1f" [expr 1.0 * $${val1}(DQ) / $TR1]]</td>"
        puts "<tr><td>Average Database Time</td>"
        puts "<td align=\"right\">[format "%6.3f" [expr 1.0 * $${val1}(DS) / $TR1]]</td>"
    }
    puts "</table>"
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
    return [string trim [read $fd $payloadSize] "\000"]
}

set port $defaultPort
set args [setOptions $allOptions]
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    } elseif {! [info exists $option]} {
        set $option 0
    }
}
if {[llength $args] > 1} {
    printUsage
    exit 1
}
if {[llength $args] == 1} {
    set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$"
    set arg [lindex $args 0]
    if {! [regexp $hostPortMatch $arg match host port]} {
        if {[string first "." $arg] >= 0} {
            set fileName $arg
        } else {
            set host $arg
        }
    }
}

if {[info exists options(file)]} {set fileName $options(file)}
if {(! [info exists host]) && (! [info exists fileName])} {
    puts "Host or file name is required"
    printUsage
    exit 1
}

if {[info exists fileName] && (! $plot) && (! $table)} {
    set maxVM 0
    set fd [open $fileName r]
    if {! [info exists fd]} {
        puts stderr "File: $fileName not found"
        exit 1
    }
    if {[info exists options(atLine)]} {
        set at [expr $options(atLine) - 1]
        for {set i 0} {$i < $at} {incr i} {
            set line [gets $fd]
            if {[eof $fd]} {break}
        }
        set line [gets $fd]
        if {[eof $fd]} {
            puts stderr "Line $at not found"
        } else {
            set loc [string first "<ELAPSED" $line]
            if {$loc >= 0} {
                set trxCount [printAll [string range $line 0 [expr $loc - 1]]]
                printElapsed [string range $line $loc end] $trxCount
            } else {
                printAll $line
            }
        }
    } else {
        set title "        time          cpu time  Itineraries  Trx Load 1Min Load 5Min Load15Min   FreeMemory  VirtualMemory"
        if {$database} {append title " DatabaseQuery  DatabaseTime"}
        puts $title
        while {! [eof $fd]} {
            set line [gets $fd]
            if {[eof $fd]} {break}
            printColumns $line
        }
    }
    close $fd
    puts stderr "Maximum virtual memory = $maxVM"
    exit 0
}

if {[info exists fileName] && $plot} {
    set fd [open $fileName r]
    if {! [info exists fd]} {
        puts stderr "File: $fileName not found"
        exit 1
    }
    set title "        time         Itineraries  Response    CPU     DatabaseTime  Queries  FreeMemory VirtualMemory   Trx/sec"
    if {[info exists options(start)]} {
        set start $options(start)
        while {! [eof $fd] && $start > 1} {
            gets $fd
            if {[eof $fd]} {break}
            incr start -1
        }
        puts ""
    } else {
        puts $title
    }
    set count 0
    set idx 0
    set ave 10
    set oldIt 0
    set lastTime [clock seconds]
    if {[info exists options(plotAverage)]} {set ave $options(plotAverage)}
    while {! [eof $fd]} {
        set plotLine($idx) [gets $fd]
        if {[string compare [string index $plotLine($idx) 0] "2"] != 0} {continue}
        if {[eof $fd]} {break}
        set thisTime [clock scan [string range $plotLine($idx) 0 18]]
        if {[regexp "IT=\"(\[0-9\]+)\"" $plotLine($idx) match it]} {
            if {$thisTime - $lastTime > 300} {puts ""}
            set lastTime $thisTime
            if {$it == $oldIt} {continue}
            set oldIt $it
        }
        incr count
        set next [expr ($idx + 1) % $ave]
        if {$count >= $ave} {
            printPlot $plotLine($next) $plotLine($idx)
        }
        set idx $next
    }
    close $fd
    exit 0
}

if {[info exists fileName] && $table} {
    set fd [open $fileName r]
    if {! [info exists fd]} {
        puts stderr "File: $fileName not found"
        exit 1
    }
    while {! [eof $fd]} {
        set line [gets $fd]
        if {[eof $fd]} {break}
        set lastLine $line
    }
    close $fd
    tableStats $lastLine t1
    if {[info exists options(file2)]} {
        set fd [open $options(file2) r]
        if {! [info exists fd]} {
            puts stderr "File: $options(file2) not found"
            exit 1
        }
        while {! [eof $fd]} {
            set line [gets $fd]
            if {[eof $fd]} {break}
            set lastLine $line
        }
        close $fd
        tableStats $lastLine t2
        printTable t1 t2
    } else {
        print1Table t1
    }
    exit 0
}

set fd [socket $host $port]
if {! [info exists fd]} {
    puts "Could not connect to $host:$port"
    exit 1
}
puts "Connected to $host:$port"
set stats [readAppConsole $fd "RFSH00010000"]
set elapsed [readAppConsole $fd "LAPS00010000"]
if {$errors} {
    set errorList [readAppConsole $fd "ERCT00010000"]
}
close $fd
if {$print} {
    puts "At: [clock format [clock seconds] -format {%Y-%m-%d %H:%M:%S}]"
    set trxCount [printAll $stats]
    printElapsed $elapsed $trxCount
} else {
    puts -nonewline $stats
    puts -nonewline $elapsed
    if {$errors && [string length $errorList] > 0} {
        puts -nonewline "<ERRORCOUNTS "
        set pairs [split $errorList "|"]
        foreach {code count} $pairs {
            if {[string length $code] > 0} {puts -nonewline "$code=\"$count\" "}
        }
        puts -nonewline "/>"
    }
    puts ""
}
exit 0
