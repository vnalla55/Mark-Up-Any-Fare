#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# newWeek.tcl
#   Move the pricing soak test graphs to start a new week
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --baseDir=dirname     Name of the directory with current data
    --lastDir=dirname     Name of the last week sub-directory
    --older=dirname       Name of the directory for older data
    --baseRelease=id      Baseline release (e.g. 2008.06.06)
    --testRelease=id      Test version (e.g. 2008.07.al)
"
}

set allOptions "baseDir lastDir older baseRelease testRelease"
proc setOptions {allOptions} {
    global options argv
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
            puts "Invalid option: [lindex $argv $i]"
            printUsage
            exit 1
        }
    }
}

set baseDir "/opt/atseintl/doc/PRICING_PERFORMANCE/weekly"
set lastDir "lastWeek"
set older "older"
setOptions $allOptions
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    } elseif {! [info exists $option]} {
        set $option 0
    }
}
if {! [info exists options(baseRelease)]} {
    puts stderr "baseRelease = ?"
    printUsage
    exit 1
}
if {! [info exists options(testRelease)]} {
    puts stderr "testRelease = ?"
    printUsage
    exit 1
}

set now [clock seconds]
set dow [clock format $now -format %w]
set dayOffset [expr $dow - 1 + 7]
set lastMondayTime [expr $now - $dayOffset * 86400]
set lastMonday [clock scan [clock format $lastMondayTime -format "%Y-%m-%d"]]
set lastMondayText [clock format $lastMonday -format "%Y-%m-%d"]
set lastMondayPrint [clock format $lastMonday -format "%B %d, %Y"]
set prevMondayTime [expr $lastMonday - 7 * 86400]
set prevMonday [clock scan [clock format $prevMondayTime -format "%Y-%m-%d"]]
set prevMondayText [clock format $prevMonday -format "%Y-%m-%d"]
set prevMondayPrint [clock format $prevMonday -format "%B %d, %Y"]
set thisMonday [expr $lastMonday + 86400 * 7]
set thisMondayText [clock format $thisMonday -format "%Y-%m-%d"]
set thisMondayDate [clock format $thisMonday -format "%b%d"]
set lastMondayDate [clock format $lastMondayTime -format "%b%d"]
set home "/login/sg955813"

# move last week's graphs
cd $baseDir/$lastDir
set fileList [glob "*.png"]
foreach file $fileList {
    if {[regsub "\\.png" $file "${prevMondayText}.png" newName]} {
        file rename $file "../$older/$newName"
    } else {
        puts stderr "Name substitute error on $file"
        exit 1
    }
}

# html for older data
set fd [open lastWeek.html r]
if {! [info exists fd]} {
    puts stderr "Can't open lastWeek.html in [pwd]"
    exit 1
}
set out [open "../$older/$prevMondayText.html" w]
if {! [info exists out]} {
    puts stderr "Can't create ../$older/$prevMondayText.html"
    close $fd
    exit 1
}
while {! [eof $fd]} {
    set line [gets $fd]
    if {[eof $fd]} {break}
    regsub "\\.png" $line "${prevMondayText}.png" line
    puts $out $line
}
close $fd
close $out

# move this week's graphs
cd $baseDir
set fileList [glob "*.png"]
foreach file $fileList {
    file rename $file "$lastDir/$file"
}

# html for last week
set fd [open thisWeek.html r]
if {! [info exists fd]} {
    puts stderr "Can't open thisWeek.html in [pwd]"
    exit 1
}
set out [open "$lastDir/lastWeek.html" w]
if {! [info exists out]} {
    puts stderr "Can't create $lastWeek/lastWeek.html"
    close $fd
    exit 1
}
while {! [eof $fd]} {
    set line [gets $fd]
    if {[eof $fd]} {break}
    regsub "This Week" $line "Week of $lastMondayPrint" line
    regsub "thisWeekMore" $line "lastWeekMore" line
    regsub "lastWeek/lastWeek" $line "../thisWeek" line
    regsub "Last Week" $line "This Week" line
    regsub "older/older" $line "../older/older" line
    puts $out $line
}
close $fd
close $out

# older menu
cd $baseDir/$older
set fd [open older.html r]
if {! [info exists fd]} {
    puts stderr "Can't open older.html in [pwd]"
    exit 1
}
set out [open "newer.html" w]
if {! [info exists out]} {
    puts stderr "Can't create $older/newer.html"
    close $fd
    exit 1
}
while {! [eof $fd]} {
    set line [gets $fd]
    if {[eof $fd]} {break}
    if {[string first "</ul>" $line] >= 0} {
        puts $out "<li><a href=$prevMondayText.html>Week of $prevMondayPrint</a></li>"
    }
    puts $out $line
}
close $fd
close $out
file rename -force "newer.html" "older.html"

# graph and html updates
if {[info exists options(baseRelease)] && [info exists options(testRelease)]} {
    # update the html template
    cd $home/bench
    set fd [open weeklyBase.html r]
    if {! [info exists fd]} {
        puts stderr "Can't open $home/bench/weeklyBase.html"
        exit 1
    }   
    set out [open "newWeek.html" w]
    if {! [info exists out]} { 
        puts stderr "Can't create temp file: newWeek.html"
        close $fd
        exit 1
    }   
    while {! [eof $fd]} {
        set line [gets $fd]
        if {[eof $fd]} {break}
        regsub "%base%" $line $baseRelease line
        regsub "%test%" $line $testRelease line
        puts $out $line
    }
    close $fd
    close $out
    unset fd
    unset out
    file rename -force "newWeek.html" "thisWeek.html"

    # update the gnuplot control file
    set fd [open plotBase.cmd r]
    if {! [info exists fd]} {
        puts stderr "Can't open $home/bench/plotBase.cmd"
        exit 1
    }   
    set out [open "plotWeekly.cmd" w]
    if {! [info exists out]} { 
        puts stderr "Can't create gnuplot control file: $home/bench/plotWeekly.cmd"
        close $fd
        exit 1
    }   
    while {! [eof $fd]} {
        set line [gets $fd]
        if {[eof $fd]} {break}
        regsub "%base%" $line $baseRelease line
        regsub "%test%" $line $testRelease line
        regsub "%day%" $line $thisMondayDate line
        puts $out $line
    }
    close $fd
    close $out
    unset fd
    unset out

    # update the graph script
    cd $home/bin
    set fd [open graph.sh r]
    if {! [info exists fd]} {
        puts stderr "Can't open $home/bin/graph.sh"
        exit 1
    }   
    set out [open "newGraph.sh" w]
    if {! [info exists out]} { 
        puts stderr "Can't create temp file: newGraph.sh"
        close $fd
        exit 1
    }   
    while {! [eof $fd]} {
        set line [gets $fd]
        if {[eof $fd]} {break}
        regsub -all $lastMondayDate $line $thisMondayDate line
        puts $out $line
    }
    close $fd
    close $out
    file rename -force "newGraph.sh" "graph.sh"
    exec chmod a+x graph.sh
}

# update bin/sync.sh
cd $home/bin
set fd [open sync.sh r]
if {! [info exists fd]} {
    puts stderr "Can't open $home/bin/sync.sh"
    exit 1
}   
set out [open "newSync.sh" w]
if {! [info exists out]} { 
    puts stderr "Can't create temp file: newSync.sh"
    close $fd
    exit 1
}   
while {! [eof $fd]} {
    set line [gets $fd]
    if {[eof $fd]} {break}
    regsub -all $lastMondayDate $line $thisMondayDate line
    puts $out $line
}
close $fd
close $out
file rename -force "newSync.sh" "sync.sh"
exec chmod a+x sync.sh
    
    
puts "Done updating from $lastMondayDate to $thisMondayDate"
