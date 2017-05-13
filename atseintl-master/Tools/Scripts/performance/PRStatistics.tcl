#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# PRStatistics.tcl
#   Generate Pricing statistics.
#   Data is obtained either from the instrumentation database or a
#   tserequest.log file.   Output is to a set of text and html files
# Author: Jim Sinnamon
# Copyright: Sabre 2007
# ------------------------------------------------------------------

if {[info exists env(USER)]} {
    set User $env(USER)
} else {
    catch [set User [exec whoami]]
    if {! [info exists User]} { set User unknown }
}
if {[string compare $User maker] == 0} {
    set outputDirectory /opt/atseintl/doc/PRICING_PERFORMANCE/stats
#    set tempDirectory /atsei/tmp/fareStat
    set tempDirectory $outputDirectory/long
    set options(longXML) $tempDirectory/longPR%.xml
} else {
    set outputDirectory $env(HOME)/tmp/stats
    set tempDirectory $env(HOME)/tmp
}
set printToLog 1
set printToStdout 1
set printNodes 0
set printService 0

# load mysql library
set file libmysqltcl2.51
set libdir "/opt/atseintl/FareDisplay/mysqltcl-2.51"
set libdir2 "/login/sg955813/opt/mysqltcl-2.51"
# change to libmysqltcl3.03 when you upgrade to mysql 4.1 or higher
set libFound 0
if {[file exists $libdir/${file}[info sharedlibextension]]} {
    load $libdir/${file}[info sharedlibextension]
    set libFound 1
} elseif {[file exists $libdir2/${file}[info sharedlibextension]]} {
    load $libdir2/${file}[info sharedlibextension]
    set libFound 1
} else {
    set pathList [split $env(LD_LIBRARY_PATH) ":"]
    for {set i 0} {i < [llength pathList]} {incr i} {
        set libdir [lindex $pathList i]
        if {[file exists $libdir/${file}[info sharedlibextension]]} {
            load $libdir/${file}[info sharedlibextension]
            set libFound 1
            break
        }
    }
}         
if {! $libFound} {
    puts stderr "can't find mysql library: $file[info sharedlibextension]"
    exit 1
}

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --file=requestfile  Use a tserequest.log file
    --host=hostname     Use a FSRESPONSE database
    --node=nodeid       Node name (application server)
    --port=portNumber   TCP port for the database (default=3306)
    --table=tablename   Name of the table in the database
    --day=dayOfMonth    Table suffix (--day=02 uses FSRESPONSE02)
    --limit=maxRows     Limit the number of transactions
    --service=service   Service id (e.g. INTL% or FAREDSP1)
    --printXML=1        Print the decoded xml to stdout
    --printTags=1       Count the XML tags and print
    --longXML=filename  Store xml for long (130sec) transactions
    --altCpu=1          AltPricingRequest CPU statistics
    --pcc=pccCode       One or more pseudo city codes
Special options:
    --node=Pricing      All pricing servers
"
}

set optionNames "file host node port table day limit service printXML printTags"
append optionNames " longXML altCpu pcc"
proc setOptions {} {
    global options argv optionNames
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
            if {[lsearch -exact $optionNames $optName] < 0} {
                puts "Unrecognized option: [lindex $argv $i]"
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

proc hasKeyValue {field value} {
    global xmlField
    if {! [info exists xmlField($field)]} {return 0}
    return [expr [string first $value $xmlField($field)] >= 0]
}

proc hasKey {field} {
    global xmlField
    return [info exists xmlField($field)]
}

proc hasMultipleKey {field} {
    global xmlField
    if {! [info exists xmlField($field)]} {return 0}
    return [expr [llength $xmlField($field)] > 1]
}

set PricingServers "('piclp004', 'piclp005', 'piclp009', 'piclp010', 'piclp011', 'piclp012', 'piclp013', 'piclp145', 'piclp152', 'piclp161', 'piclp162', 'piclp163', 'piclp164', 'piclp165', 'piclp166', 'piclp167', 'piclp215', 'piclp216', 'piclp217', 'piclp254', 'piclp255', 'piclp256', 'piclp257', 'piclp258', 'piclp259', 'pimlp001', 'pimlp002', 'pimlp003', 'pimlp004', 'pimlp005', 'pimlp006', 'pimlp007', 'pimlp043', 'pimlp044', 'pimlp045', 'pimlp046', 'pimlp047', 'pimlp048', 'pimlp049')"
set reqCounters "PricingRequest CurrencyConversionRequest MileageRequest AltPricingRequest SelectionRequest Long"
append reqCounters " INTLWPI1 INTLDC01"

set PRServices "INTLWPI1 INTLDC01"

set timers "Timed"
foreach {name} $timers {set rspTime($name) 0.0}
foreach {name} $timers {set cpuTime($name) 0.0}
foreach {name} $reqCounters {
    set cpuTime($name) 0.0
    set rspTime($name) 0.0
    set count($name) 0
}
set countTotal 0
set countTimed 0
set maxClockTime 0
set maxCpuTime 0
set count(overTime) 0
set maxAltCpu 0
set minAltCpu 100
set altCpuCount 0
set altCpuFlag 0
for {set i 0} {$i <= 200} {incr i} {set altCpu($i) 0}
for {set i 0} {$i <= 100} {incr i} {set altTenths($i) 0}
set xmlCount 0
set noXmlCount 0
set totalPAX 0

setOptions
set theTime [expr [clock seconds] - 86400]
set targetDay [clock format $theTime -format %d]
set day $targetDay
set printDate [clock format $theTime -format "%Y-%m-%d"]
if {! [info exists options(printXML)]} {set options(printXML) 0}
set printXML [expr ! [regexp {^[nN0]} $options(printXML)]]
if {! [info exists options(printTags)]} {set options(printTags) 0}
set printTags [expr ! [regexp {^[nN0]} $options(printTags)]]
if {[info exists options(altCpu)]} {set altCpuFlag $options(altCpu)}

# read the list of pricing tags
if {$printTags || $printXML} {
    set pricingTagsFile "/login/sg955813/etc/script/PricingTags.csv"
    set fd [open $pricingTagsFile r]
    if {! [info exists fd]} {
        puts("Can't open pricing tags: $pricingTagsFile");
        exit 1
    }
    while {! [eof $fd]} {
        gets $fd line
        if {[eof $fd]} {break}
        set tagDesc [split $line ","]
        set tagid [lindex $tagDesc 0]
        if {[info exists tags($tagid)]} {
            lappend tags($tagid) [lindex $tagDesc 1]
        } else {
            set tags($tagid) [list [lindex $tagDesc 1]]
        }
        if {[llength $tagDesc] >= 5} {
            set tagGroup [lindex $tagDesc 4]
            if {[string length $tagGroup] > 0} {
                set tags(${tagGroup}:${tagid}) [list [lindex $tagDesc 1]]
            }
        }
    }
    close $fd
}

if {[info exists options(host)]} {
    set useDatabase 1
    set host [lindex $options(host) 0]
    if {[info exists options(port)]} {
        set port $options(port)
    } else {
        set port 3306
    }
    if {[info exists options(table)]} {
        set table $options(table)
        set day [string range $table [expr [string length $table] - 2] end]
        if {[regexp {^[0-9][0-9]$} $targetDay]} {set targetDay $day}
    } else {
        if {[info exists options(day)]} {
            set table "FSRINSTR$options(day)"
            set targetDay $options(day)
        } else {
            set table "FSRINSTR$targetDay"
        }
    }
} else {
    if {[info exists options(file)]} {
        set useDatabase 0
        set logFileName [lindex $options(file) 0]
    } else {
        puts stderr "Either --file or --host is required" 
        puts stderr [array get options]
        printUsage
        exit 1
    }
}
if {$targetDay != [string trimleft $day "0"]} {
    set td [string trimleft $day "0"]
    set theDay [string trimleft $day "0"]
    set theTime [expr $theTime + 86400 * ($td - $theDay)]
    set printDate [clock format $theTime -format "%Y-%m-%d"]
}
if {[info exists options(longXML)]} {
    set fn $options(longXML)
    if {[set idx [string first "%" $fn]] >= 0} {
        set prefix [string range $fn 0 [expr $idx - 1]]
        set suffix [string range $fn [expr $idx + 1] end]
        set fn ${prefix}${printDate}${suffix}
    }
    set xmlFd [open $fn "a"]
    if {! [info exists xmlFd]} {
        puts stderr "Unable to open long xml file: fn"
    }
}
 
set DBindex 0

if {$useDatabase} {
    set db [mysqlconnect -host $host -port $port -user hybfunc -db ATSEHYB2]
    if { ! [info exists db] } {
        puts stderr "can't connect to mysql database"
        exit 2
    }
    puts stderr "connected to ATSEHYB2 on $host"
    set fields "fs.nodeid,fs.servicename,trim(xml.inputcmd),fs.cpuused,fs.elapsedtime"
    append fields ",fs.transactionid,fs.transactionstart,fs.homeagency,fs.agentcity"
    append fields ",fs.paxtype1,fs.paxtype2,fs.paxtype3,fs.paxtype4"
    set where ""
    if {[info exists options(node)]} {
        set node $options(node)
        if {[string first "%" $node] >=0} {
            append where " and fs.nodeid like '$node'"
        } else {
            if {[string compare [string tolower $node] "farequote"] == 0} {
                append where " and fs.nodeid in $FareQuoteServers"
            } elseif {[string compare [string tolower $node] "pricing"] == 0} {
                append where " and fs.nodeid in $PricingServers"
            } else {
                append where " and fs.nodeid='$node'"
            }
        }
    }
    if {[info exists options(service)]} {
        if {[string first "%" $options(service)] >=0} {
            append where " and fs.servicename like '$options(service)'"
        } else {
            append where " and fs.servicename='$options(service)'"
        }
    } else {
        set PRServiceFilter "("
        foreach service $PRServices {
            append PRServiceFilter "'$service', "
        }
        set PRServiceFilter [string range $PRServiceFilter 0 [expr [string length $PRServiceFilter] - 3]]
        append PRServiceFilter ")"
        append where " and fs.servicename in $PRServiceFilter"
    }
    if {[info exists options(pcc)]} {
        puts stderr "options(pcc) = '$options(pcc)'"
        if {[llength $options(pcc)] > 1} {
            set len [llength $options(pcc)]
            append where " and homeagency in ("
            for {set pccNum 0} {$pccNum < $len} {incr pccNum} {
                if {$pccNum != 0} {append where ","}
                append where " \'[lindex $options(pcc) $pccNum]\'"
            }
            append where ")"
        } else {
            if {[string first "%" $options(pcc)] >=0} {
                append where " and homeagency like '$options(pcc)'"
            } else {
                append where " and homeagency='$options(pcc)'"
            }
        }
    }
    if {[info exists options(limit)]} {
        set maxRows $options(limit)
    } else {
        set maxRows 4000000
    }
    puts stderr "using table $table"
    set where "where [string range $where 4 end]"
    set limit "limit $maxRows"
    set using "using (servicename,nodeid,transactionid,clienttransactionid)"
    set join "left outer join XMLREQUEST$targetDay as xml"
#    puts stderr "select $fields from $table as fs $join $using $where $limit"
    set rows [mysqlsel $db "select $fields from $table as fs $join $using $where $limit"]
    puts stderr "$rows rows selected"
    set rowNumber 0
    set endData [expr $rowNumber >= $rows]
} else {
    set file [lindex $options(file) 0]
    set infl [open $file r]
    if { ! [info exists infl] } {
        puts stderr "Can't open file: $file"
        exit 1
    }
    set endData [eof $infl] 
}

while {$endData == 0} {
    if {$useDatabase} {
        set row [mysqlnext $db]
        set nodeid [lindex $row 0]
        set servicename [lindex $row 1]
        set inputcmd [string trim [lindex $row 2]]
        set cpuused [lindex $row 3]
        set timeUsed [lindex $row 4]
        set homeAgency [lindex $row 7]
        set agentCity [lindex $row 8]
        set hasXML [expr [string length $inputcmd] > 0]
        if {$hasXML} {incr xmlCount} else {incr noXmlCount}

        if {[string length [string trim $cpuused]] == 0} {set cpuused 0}
        if {[string length [string trim $timeUsed]] == 0} {set timeUsed 0}
        set cpu [expr ${cpuused}.0 / 1000000.0]
        set clockTime [expr ${timeUsed}.0 / 1000000.0]
        if {$cpu > 0.0 || $clockTime > 0.0} {
            set cpuTime(Timed) [expr $cpuTime(Timed) + $cpu]
            set rspTime(Timed) [expr $rspTime(Timed) + $clockTime]
            incr countTimed
            if {$clockTime > 30.0} {
                incr count(Long)
                set cpuTime(Long) [expr $cpuTime(Long) + $cpu]
                set rspTime(Long) [expr $rspTime(Long) + $clockTime]
            }
            if {$clockTime > 130.0} {
                incr count(overTime)
                if {[info exists xmlFd] && $clockTime > 160.0} {
                    set transactionId [lindex $row 5]
                    set transactionTime [lindex $row 6]
                    puts -nonewline $xmlFd "$transactionTime: "
                    puts -nonewline $xmlFd "$transactionId "
                    puts -nonewline $xmlFd "Time: $clockTime"
                    puts -nonewline $xmlFd " cpuTime: $cpu "
                    puts -nonewline $xmlFd "$servicename on $nodeid "
                    puts $xmlFd $inputcmd
                }
            }
            if {$clockTime > $maxClockTime} {set maxClockTime $clockTime}
            if {$cpu > $maxCpuTime} {set maxCpuTime $cpu}
        }

        incr rowNumber
        if {$rowNumber >= $rows} {
            set rows 0
            incr DBindex
            if {[llength $options(host)] > $DBindex} {
                mysqlclose $db
                unset db
                set host [lindex $options(host) $DBindex]
                if {[info exists options(limit)]} {
                    set maxRows [expr $options(limit) - $countTotal - 1]
                    if {$maxRows <= 0} {set maxRows 1}
                } else {
                    set maxRows 4000000
                }
                set db [mysqlconnect -host $host -port $port -user hybfunc -db ATSEHYB2]
                if { ! [info exists db] } {
                    puts stderr "can't connect to mysql database on $host"
                    set rows 0
                } else {
                    puts stderr "connected to ATSEHYB2 on $host"
                    puts stderr "using table $table"
                    set limit "limit $maxRows"
                    set rows [mysqlsel $db "select $fields from $table as fs $join $using $where $limit"]
                    puts stderr "$rows rows selected"
                    if {$rows < 0} {
                        puts stderr "select $fields from $table as fs $join $using $where $limit"
                        puts stderr "mysql status: [mysqlinfo $db info]"
                        puts stderr "mysql state: [mysqlstate $db]"
                        puts stderr "mysqlstatus(code): $mysqlstatus(code)"
                        puts stderr "mysqlstatus(command): $mysqlstatus(command)"
                        puts stderr "mysqlstatus(message): $mysqlstatus(message)"
                    }
                }
            }
            set rowNumber 0
            set endData [expr $rowNumber >= $rows]
        }
    } else {
        set inputcmd [string trim [gets $infl]]
        set nodeid "xxxxxxxx"
        set servicename "FARE"
        set cpu 0
        set clockTime 0
        set endData [eof $infl]
        set hasXML 1
        if {$endData} {
            incr DBindex
            if {[llength $options(file)] > $DBindex} {
                close $infl
                unset infl
                set file [lindex $options(file) 0]
                set infl [open $file r]
                if { ! [info exists infl] } {
                    puts stderr "Can't open file: $file"
                    break;
                }
                set inputcmd [string trim [gets $infl]]
                set endData [eof $infl]
            }
        }
        if {$endData} { break }
        if {[info exists options(limit)]} {
            if {$countTotal >=  $options(limit)} {break}
        }
    }
    incr countTotal
    if {$hasXML} {
        set start [string first "<" $inputcmd]
        set line [string range $inputcmd $start end]
        set xml [split $inputcmd "<"]
        set nGroups [llength $xml]
        set context {}
        set thisGroup {}
        set contextLast -1
        set indent ""
        if {[array exists xmlField]} {unset xmlField}
        if {[array exists groupCount]} {unset groupCount}
        if {$printXML} {puts [format "cpu=%6.3f rsp=%6.3f" $cpu $clockTime]}
        for {set j 1} {$j < $nGroups} {incr j} {
            set group [string trim [lindex $xml $j]]
            set items [split $group " />"]
            if {[string compare [string index $group 0] "/"] == 0} {
                set id [lindex [split $items " />"] 1]
                set idx [lsearch $context $id]
                if {$idx >= 0} {
                    set context [lreplace $context $idx $idx]
                    incr contextLast -1
                    set thisGroup [lindex $context $contextLast]
                    if {$printXML} {
                        set indent [string range $indent 2 end]
                        puts "${indent}</$id>"
                    }
                } else {
                    puts stderr "end tag \"$group\" not found"
                    puts stderr "in \"$context\""
                }
                continue
            }
    
            set nTags [llength $items]
            if {$nTags < 1} {continue}
            set id [lindex $items 0]
            lappend context $id
            incr contextLast
            set thisGroup $id
            if {! [info exists groupCount($id)]} {set groupCount($id) 0}
            incr groupCount($id)
            if {! [info exists count($id)]} {set count($id) 0}
            incr count($id)
            if {$contextLast == 0} {
                if {! [info exists cpuTime($id)]} {set cpuTime($id) 0.0}
                set cpuTime($id) [expr $cpuTime($id) + $cpu]
                if {! [info exists rspTime($id)]} {set rspTime($id) 0.0}
                set rspTime($id) [expr $rspTime($id) + $clockTime]
                if {! [info exists reqCount($id)]} {set reqCount($id) 0}
                incr reqCount($id)
            }
            if {$printXML} {
                set gname "${thisGroup}:${id}"
                if {[info exists tags($gname)]} {
                    set tagTxt [lindex $tags($gname) 0]
                    puts [format "%-30s %s" "$indent<$id" "$tagTxt"]
                } elseif {[info exists tags($id)]} {
                    set tagTxt [lindex $tags($id) 0]
                    puts [format "%-30s %s" "$indent<$id" "$tagTxt"]
                } else {
                    puts "$indent<$id"
                }
                set indent "  $indent"
            }
            # for DTS, discard encoded data
            if {[string compare $thisGroup "DTS"] == 0} {
                set i [string first ">" $group]
                if {$i > 0} {
                    set group [string trim [string range $group 0 $i]]
                    set items [split $group " />"]
                }
            }
    
            for {set idx 1} {$idx < $nTags} {incr idx} {
                set item [split [lindex $items $idx] "="]
                if {[llength $item] == 2} {
                    set name [lindex $item 0]
                    set value [string trim [lindex $item 1] " \"/>"]
                    set gname "${thisGroup}:${name}"
                    if {[info exists xmlField($gname)]} {
                        lappend xmlField($gname) $value
                    } else {
                        set xmlField($gname) $value
                    }
                    if {$printTags} {
                        if {[info exists tags($gname)]} {
                            if {! [info exists count($gname)]} {set count($gname) 0}
                            incr count($gname)
                        } else {
                            if {! [info exists count($name)]} {set count($name) 0}
                            incr count($name)
                        }
                    }
                    if {$printXML} {
                        if {[info exists tags($gname)]} {
                            set tagTxt [lindex $tags($gname) 0]
                            puts [format "%-30s %s" "$indent$name=$value" "$tagTxt"]
                        } elseif {[info exists tags($name)]} {
                            set tagTxt [lindex $tags($name) 0]
                            puts [format "%-30s %s" "$indent$name=$value" "$tagTxt"]
                        } else {
                            puts "$indent$name=$value"
                        }
                    }
                }
            }
    
            if {[regexp "/>$" $group]} {
                set idx [lsearch $context $id]
                if {$idx >= 0} {
                    set context [lreplace $context $idx $idx]
                    set indent [string range $indent 2 end]
                    incr contextLast -1
                    set thisGroup [lindex $context $contextLast]
                    if {$printXML} {puts "$indent/> $id"}
                }
            }
        }
    } else {
        # no xml
        set id $servicename
        if {! [info exists cpuTime($id)]} {set cpuTime($id) 0.0}
        set cpuTime($id) [expr $cpuTime($id) + $cpu]
        if {! [info exists rspTime($id)]} {set rspTime($id) 0.0}
        set rspTime($id) [expr $rspTime($id) + $clockTime]
        if {! [info exists reqCount($id)]} {set reqCount($id) 0}
        incr reqCount($id)
        if {! [info exists count($id)]} {set count($id) 0}
        incr count($id)
        for {set pt 1} {$pt <= 4} {incr pt} {
            set pType [lindex $row [expr $pt + 8]]
            if {[string length $pType] > 0} {
                if {! [info exists PAX($pType)]} {set PAX($pType) 0}
                incr PAX($pType)
                incr totalPAX
            }
        }
    }

    if {[info exists Nodes($nodeid)]} {
        incr Nodes($nodeid)
    } else {
        set Nodes($nodeid) 1
    }
    if {[info exists ServiceName($servicename)]} {
        incr ServiceName($servicename)
    } else {
        set ServiceName($servicename) 1
    }
    # count segments & average time
    if {[info exists groupCount(SGI)]} {
        set nSGI $groupCount(SGI)
        if {! [info exists SGIcount($nSGI)]} {set SGIcount($nSGI) 0}
        incr SGIcount($nSGI)
        if {! [info exists SGIcpu($nSGI)]} {set SGIcpu($nSGI) 0.0}
        set SGIcpu($nSGI) [expr $SGIcpu($nSGI) + $cpu]
        if {! [info exists SGIrsp($nSGI)]} {set SGIrsp($nSGI) 0.0}
        set SGIrsp($nSGI) [expr $SGIrsp($nSGI) + $clockTime]
    }
    # passenger types
    if {[info exists xmlField(PXI:B70)]} {
        set pType $xmlField(PXI:B70)
        if {! [info exists PAX($pType)]} {set PAX($pType) 0}
        incr totalPAX
        incr PAX($pType)
    }
    # RexPricing
    if {[info exists xmlField(RexPricingRequest:S96)]} {
        set type $xmlField(RexPricingRequest:S96)
        if {! [info exists REX($type)]} {set REX($type) 0}
        incr REX($type)
    }
    # alt pricing cpu histogram
    if {$altCpuFlag} {
        if {[string compare $id "AltPricingRequest"] == 0} {
            incr altCpuCount
            if {$cpu > $maxAltCpu} {set maxAltCpu $cpu}
            if {$cpu < $minAltCpu} {set minAltCpu $cpu}
            set bucket [expr int($cpu)]
            if {$bucket > 100} {set bucket 100}
            incr altCpu($bucket)
            if {$cpu < 10.0} {
                set bucket [expr int($cpu * 10.0)]
                if {$bucket > 100} {set bucket 100}
                incr altTenths($bucket)
            }
        }
    }
}

#
# done reading
#
if {$useDatabase} {
    if {[info exists db]} {mysqlclose $db}
} else {
    if {[info exists infl]} {close $infl}
}
if {[info exists xmlFd]} {close $xmlFd}

#
#  -- print results --
#     procedures
#
proc top10 {arrayName} {
    upvar $arrayName arrayVal
    set tenth 0
    set top {}
    for {set i 0} {$i < 10} {incr i} {lappend top {z 0}}
    foreach {key value} [array get arrayVal] {
        if {$value > $tenth} {
            set top [lreplace $top 9 9 [list "$key" $value]]
            set top [lsort -integer -index 1 -decreasing $top]
            set tenth [lindex [lindex $top 9] 1]
        }
    }
    return $top
}

# --

proc putTop10 {arrayName fileName heading total title} {
    upvar $arrayName data
    global printToLog printToStdout outputDirectory count printDate
    set Top10 [top10 data]
    set separator "#****************************************************"
    if {$printToLog} {
        set filename "$outputDirectory/${fileName}.tmp"
        set fd [open $filename "w+"]
        if {! [info exists fd]} {
            puts stderr "Can't create statistics file: $filename"
        } else {
            puts $fd "$separator"
            puts $fd "# $printDate                $title $total"
            puts $fd "$separator"
            puts $fd "$heading"
            if {$total == 0} {set total 1.0} else {set total ${total}.0}
            set index 0
            foreach {pair} $Top10 {
                incr index
                set value [lindex $pair 1]
                set line [format "%-5d  %14s %7d   %5.2f" $index "[lindex $pair 0]" $value "[expr 100 * $value / $total]"]
                if {$value != 0} {puts $fd $line}
            }
        }
        copyTail $fileName $fd 0
    }
    if {$printToStdout} {
        puts ""
        puts "Top 10 $arrayName"
        puts "$heading"
        set index 0
        foreach {pair} $Top10  {
            incr index
            set value [lindex $pair 1]
            set line [format "%-5d  %14s %7d   %5.2f" $index "[lindex $pair 0]" $value "[expr 100 * $value / $total]"]
            if {$value != 0} {puts $line}
        }
    }
}

# --

proc copyTail {fileName fd skip} {
#       copy the data from the previous day's file and rename
    global outputDirectory
    set oldfile "$outputDirectory/${fileName}.txt"
    set newfile "$outputDirectory/${fileName}.tmp"
    if {[file exists $oldfile]} {
        set f2 [open $oldfile "r"]
        if {[info exists f2]} {
#           skip heading
            for {set i 0} {$i < $skip} {incr i} {
                set line ""
                gets $f2 line
            }
            while {! [eof $f2]} {
                gets $f2 line
                if {! [eof $f2]} {puts $fd $line}
            }
        close $f2
        }
        if {[file exists "$outputDirectory/${fileName}.bak"]} {
            file delete "$outputDirectory/${fileName}.bak"
        }
        file rename $oldfile "$outputDirectory/${fileName}.bak"
    }
    close $fd
    file rename $newfile $oldfile
    catch {file attributes $oldfile -permissions 00664}
}

proc putSGIcount {fd} {
    global SGIcount SGIcpu SGIrsp
    set counters {}
    set sum 0
    foreach {key value} [array get SGIcount] {
        if {$value > 0} {
            lappend counters $key
        }
        set sum [expr $sum + $value]
    }
    set t [lsort -integer $counters]
    puts $fd "# segments  count  percent  Ave rsp   Ave CPU"
    foreach {key} $t {
        set val $SGIcount($key)
        set ave [expr $SGIcpu($key) / $val]
        set rsp [expr $SGIrsp($key) / $val]
        set pct [expr 100.0 * $val / $sum]
        puts $fd [format "      %2d %8d    %5.2f   %6.3f    %6.3f" $key $val $pct $rsp $ave]
    }
}

proc putREXcount {fd} {
    global REX
    set keys [lsort [array names REX]]
    if {[llength $keys] == 0} {
        puts $fd "No S96 tags found"
        return
    }
    puts $fd "type   count"
    foreach key $keys {
        puts $fd [format "%4s %7d" $key $REX($key)]
    }
}

#
#  -- print results --
#

if {$printToStdout} {
    if {$printNodes} {
        foreach {key value} [array get Nodes] {
            puts "Nodes($key) = $value"
        }
    puts ""
    }
    if {$printService} {
        foreach {key value} [array get ServiceName] {
            puts "ServiceName($key) = $value"
        }
    }
}

if {$printTags} {
    puts ""
    set counters {}
    foreach {key value} [array get count] {
        if {$value > 0} {
            lappend counters $key
        }
    }
    set t [lsort $counters]
    foreach {key} $t {
        set value $count($key)
        set line [format "%-8s = %7d" $key $value]
        if {[info exists tags($key)]} {
            puts [format "%-30s %s" $line [lindex $tags($key) 0]]
        } else {
            puts $line
        }
    }
}

set total $countTotal
set title "Total requests ="

if {[array exists SGIcount]} {
    if {$printToLog} {
        set separator "#****************************************************"
        set filename "$outputDirectory/segments.tmp"
        set fd [open $filename "w+"]
        if {! [info exists fd]} {
            puts stderr "Can't create statistics file: $filename"
        } else {
            puts $fd "$separator"
            puts $fd "# $printDate                $title $total"
            puts $fd "$separator"
            putSGIcount $fd
            copyTail segments $fd 0
        }
    }
    if {$printToStdout} {
        puts "Segments"
        putSGIcount stdout 
    }
}

if {[array exists PAX]} {
    set heading "Rank  Passenger type    count  percent"
    putTop10 PAX PAXType $heading $totalPAX $title
}

if {[array exists REX]} {
    puts stdout ""
    puts stdout "Exchange Types"
    putREXcount stdout
} else {
    puts stdout "No S96 transactions found"
}

#
# print results
#   statistics
#
if {$printToLog} {
    set filename "$outputDirectory/PRStatistics.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts $fd "#Date               Total    %Pricing  %Currency   %Mileage  %AltPrice  %Selection >30seconds %INTLWPI1  %INTLDC01 AveTime  AveCPU"
        puts -nonewline $fd "$printDate"
        puts -nonewline $fd "       "
        set total $countTotal.0
        if {$countTotal == 0} {set total 1.0}
        puts -nonewline $fd [format "%8d " $countTotal]
        foreach {name} $reqCounters {
            set value [expr 100 * $count($name) / $total]
            puts -nonewline $fd "     [format "%6.2f" $value]"
         } 
        set TotalSeconds $rspTime(Timed)
        set TotalCPU $cpuTime(Timed)
        if {$countTimed == 0} {set Timed 1.0} else {set Timed $countTimed.0}
        puts -nonewline $fd " [format "%7.3f" [expr $TotalSeconds / $Timed]]"
        puts -nonewline $fd " [format "%7.3f" [expr $TotalCPU / $Timed]]"
        puts $fd ""
    }
    copyTail "PRStatistics" $fd 1
#   raw counts
    set filename "$outputDirectory/PRCounts.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts $fd "Date          Total  Pricing  Mileage AltPrice Currency Selection >30Secs INTLWPI1 INTLDC01      Time         CPU"
        puts -nonewline $fd $printDate
        puts -nonewline $fd " [format "%8d" $countTotal]"
        foreach {name} $reqCounters {
            puts -nonewline $fd " [format "%8d" $count($name)]"
         } 
        foreach {name} $timers {
            puts -nonewline $fd " [format "%12.3f" $rspTime($name)]"
            puts -nonewline $fd " [format "%12.3f" $cpuTime($name)]"
        }
        puts $fd ""
    }
    copyTail "PRCounts" $fd 1
}

if {$printToStdout} {
    puts ""
    puts -nonewline "   Request type             count percent"
    if {$cpuTime(Timed) > 0.0 || $rspTime(Timed) > 0.0} {puts "  response   cpuTime"} else {puts ""}
    set c $countTotal
    if {$c == 0} {set c 1}
    foreach {key} $reqCounters {
        set pct [expr 100.0 * $count($key) / $c]
        puts -nonewline [format "%25s %7d  %6.2f" $key $count($key) $pct]
        if {$cpuTime($key) > 0.0 || $rspTime($key) > 0.0} {
            set ave [expr $cpuTime($key) / $count($key)]
            set rsp [expr $rspTime($key) / $count($key)]
            puts [format "   %7.3f   %7.3f" $rsp $ave]
        } else {
            puts ""
        }
    }
    puts -nonewline [format "%25s %7d" "Total" $countTotal]
    if {$cpuTime(Timed) > 0.0} {
        set ave [expr $cpuTime(Timed) / $countTimed]
        set rsp [expr $rspTime(Timed) / $countTimed]
        puts [format "           %7.3f   %7.3f" $rsp $ave]
    } else {
        puts ""
    }
    puts ""
    puts "# with response time > 130 sec = $count(overTime)"
    puts "Longest transaction = $maxClockTime seconds"
    puts "Longest CPU time = $maxCpuTime seconds"
    puts "$xmlCount with XML, $noXmlCount without XML"
}

proc printTimers {filename} {
    global printToStdout printToLog outputDirectory
    global reqCounters count rspTime cpuTime printDate
    global countTimed
    if {$printToLog} {
        set filenm "$outputDirectory/$filename.tmp"
        set fd [open $filenm "w+"]
        if {! [info exists fd]} {
            puts stderr "Can't create statistics file: $filename"
        } else {
            puts $fd "  Date         Total        Pricing       Currency       Mileage     AltPricing    Selection    >30 seconds    INTLWPI1    INTLDC01"
            puts $fd "             rsp   cpu     rsp    cpu    rsp    cpu    rsp    cpu    rsp    cpu    rsp    cpu   rsp    cpu     rsp    cpu   rsp    cpu"
            puts $fd "           ------------- ------------- ------------- ------------- ------------- ------------- ------------- ------------- -------------"
#           puts $fd "xxxx-xx-xx 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000"
            puts -nonewline $fd "$printDate"
            if {$countTimed == 0} {set c 1.0} else {set c $countTimed}
            set rTime [expr $rspTime(Timed) / $c]
            set cTime [expr $cpuTime(Timed) / $c]
            puts -nonewline $fd " [format "%6.3f %6.3f" $rTime $cTime]"
            foreach {name} $reqCounters {
                if {$count($name) == 0} {set c 1.0} else {set c $count($name)}
                set rTime [expr $rspTime($name) / $c]
                set cTime [expr $cpuTime($name) / $c]
                puts -nonewline $fd " [format "%6.3f %6.3f" $rTime $cTime]"
            }
            puts $fd ""
        }
    copyTail "Performance" $fd 3
    }
}

proc printTimersHtml {filename} {
    global outputDirectory
    set baseName "$outputDirectory/${filename}" 
    set fd [open "${baseName}.tmp" "w"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
        return 0
    } 
    set f2 [open "$outputDirectory/${filename}.txt" "r"]
    if {! [info exists f2]} {
        puts stderr "Can't open data file: $filename"
        close $fd
        return 0
    } 
    puts $fd "<html>"
    puts $fd "<head>"
    puts $fd "  <title>Fare Display Performance Report</title>"
    puts $fd "</head>"
    puts $fd "<body>"
    puts $fd ""
    puts $fd "<h2>ATSEv2 Fare Display Servers</h2>"
    puts $fd "<p><table border=\"1\" cellpadding=\"2\">"
    puts $fd "<tr>"
    puts $fd ""
    puts $fd "<td>Date</td>"
    puts $fd "<td>Average Response Time</td>"
    puts $fd "<td>Average CPU Time</td>"
    for {set i 0} {$i < 3} {incr i} {
        # skip header
        gets $f2 line
    }
    while {! [eof $f2]} {
        gets $f2 line
        if {! [eof $f2]} {
            puts $fd "<tr>"
            puts $fd "<td>[lindex $line 0]</td>"
            puts $fd "<td align=\"right\">[lindex $line 1]</td>"
            puts $fd "<td align=\"right\">[lindex $line 2]</td>"
        }
    }
    close $f2
    puts $fd "</table>"
    puts $fd "<p>"
    puts $fd "<a href=\"${filename}.txt\">Details</a>"
    puts $fd "</body>"
    puts $fd "</html>"
    close $fd
    # delete old backup, rename old file, make this the new file
    if {[file exists "${baseName}.bak"]} {
        file delete "${baseName}.bak"
    }
    file rename ${baseName}.html "${baseName}.bak"
    file rename ${baseName}.tmp ${baseName}.html
}

if {$printToStdout && $altCpuFlag && $altCpuCount > 0} {
    puts "Cpu time for AltPricingRequests"
    if {$minAltCpu > $maxAltCpu} {set minAltCpu $maxAltCpu}
    puts "Minimum AltPricingRequest = [format {%7.3f} $minAltCpu] seconds"
    puts "Maximum AltPricingRequest = [format {%7.3f} $maxAltCpu] seconds"
    for {set i 0} {$i < 100} {incr i} {
        puts "[format "%3d-%-3d seconds %6d" $i [expr $i+1] $altCpu($i)]"
    }
    puts "[format "  >100  seconds %6d" $altCpu(100)]"
    puts "Details:"
    for {set i 0} {$i < 100} {incr i} {
        set v1 [expr $i / 10.0]
        set v2 [expr ($i + 1) / 10.0]
        puts "[format "%3.1f-%-3.1f seconds %6d" $v1 $v2 $altTenths($i)]"
    }
}
    
if {$printToLog && $altCpuFlag} {
    set filename "$outputDirectory/altCpu.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts -nonewline $fd "   Date       min     max   count  "
        for {set i 0} {$i < 100} {incr i} {
            puts -nonewline $fd "[format "%2d-%-3d" $i [expr $i+1]]"
        }
        puts $fd "    >100"
        puts -nonewline $fd "$printDate"
        puts -nonewline $fd "[format "%7.3f %7.3f %7d " $minAltCpu $maxAltCpu $altCpuCount]"
        for {set i 0} {$i < 100} {incr i} {
            puts -nonewline $fd "[format "%6d" $altCpu($i)]"
        }
        puts $fd "[format "%8d" $altCpu(100)]"
        copyTail "altCpu" $fd 1
    }
    set filename "$outputDirectory/altCpuDetails.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts -nonewline $fd "   Date       min     max  "
        for {set i 0} {$i < 100} {incr i} {
            set v1 [expr $i / 10.0]
            set v2 [expr ($i + 1) / 10.0]
            puts -nonewline $fd "[format "%3.1f-%-3.1f " $v1 $v2]"
        }
        puts $fd ""
        puts -nonewline $fd "$printDate"
        puts -nonewline $fd "[format "%7.3f %7.3f " $minAltCpu $maxAltCpu]"
        for {set i 0} {$i < 100} {incr i} {
            set v1 [expr $i / 10.0]
            set v2 [expr ($i + 1) / 10.0]
            puts -nonewline $fd "[format "%8d" $altTenths($i)]"
        }
        puts $fd ""
        copyTail "altCpuDetails" $fd 1
    }
}

printTimers "Performance"
exit 0


