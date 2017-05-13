#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# FDStatistics.tcl
#   Generate FareDisplay statistics.
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
    set outputDirectory /opt/atseintl/doc/FAREDISPLAY_PERFORMANCE/stats
    set tempDirectory $outputDirectory/long
    set options(longXML) $tempDirectory/longFD%.xml
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
    --table=tablename   Name of the table in the database
    --node=nodeid       Node name (application server)
    --service=service   Service id (e.g. INTL% or FAREDSP1)
    --day=dayOfMonth    Table suffix (--day=02 uses FSRESPONSE02)
    --port=portNumber   TCP port for the database (default=3306)
    --limit=maxRows     Limit the number of transactions
    --longXML=filename  Store xml for long (130sec) transactions
    --byHour=0 or 1     Timings by hour of the day
Special options:
    --node=FareDisplay  Select all FareDisplay servers
    --node=Pricing      All Pricing servers
"
}

proc setOptions {} {
    global options argv
    for {set i 0} {$i < [llength $argv]} {incr i} {
        set tag [string range [lindex $argv $i] 0 1]
        set opt [split [lindex $argv $i] "="]
        set len [llength $opt] 
        if {($len == 2 || $len == 1) && [string compare $tag "--"] == 0} {
            set optName "[string range [lindex $opt 0] 2 end]"
            if {$len == 2} {set value [split [lindex $opt 1] ","]} else {set value 1}
            if {[info exists options($optName)]} {
                set options($optName) "$options($optName) $value"
            } else {
                set options($optName) $value
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

set FareDisplayServers "('piclp006', 'piclp007', 'piclp008', 'piclp143', 'piclp144', 'piclp168', 'piclp219', 'piclp220', 'piclp221', 'piclp222', 'piclp223', 'piclp224', 'piclp225', 'piclp226', 'piclp260', 'pimlp038', 'pimlp039', 'pimlp040', 'pimlp041', 'pimlp042')"

set PricingServers "('piclp004', 'piclp005', 'piclp009', 'piclp010', 'piclp011', 'piclp012', 'piclp013', 'piclp145', 'piclp152', 'piclp161', 'piclp162', 'piclp163', 'piclp164', 'piclp165', 'piclp166', 'piclp167', 'piclp215', 'piclp216', 'piclp217', 'piclp254', 'piclp255', 'piclp256', 'piclp257', 'piclp258', 'piclp259', 'pimlp001', 'pimlp002', 'pimlp003', 'pimlp004', 'pimlp005', 'pimlp006', 'pimlp007', 'pimlp043', 'pimlp044', 'pimlp045', 'pimlp046', 'pimlp047', 'pimlp048', 'pimlp049')"

set FDServices "BKGCDSP1 FAREDSP1 MILGDSP1 RULEDSP1 TAXBDSP1"
set FDServicesId "RB FQ MP RD FT"

set ALLcounters "Total FQ RD LRD RB FT MP SDSQ SDST ABCS"
set FQcounters "Shop PT CI ALL AC SA SD PV PL NoShop"
set counterNames "$ALLcounters $FQcounters"
set counterStrings {
{Total}
{Fare Quote}
{all Rule Display}
{Long Rule Display only}
{BookingCode}
{Fare Tax}
{Mileage}
{SDS FQ reque}
{SDS FT} 
{Abacus} 
{Shopper (all carrier and multicarrier)}
{Passenger Type (Any Pax Type requests)}
{Child Infant (includes CI qualifiers and Pax types, CNN, INS and INF)}
{ALL inclusion code} 
{Account Code} 
{Sort Ascending} 
{Sort Descending} 
{Private Fare} 
{Published Fare} 
}
set timerStrings {
{Total}
{Fare Quote}
{All Rule Display}
{Long Rule Display only}
{BookingCode}
{Fare Tax}
{Mileage}
{Shopper}
{NonShopper}
}
foreach {name} $counterNames {set count($name) 0}
set timers "Total FQ RD LRD RB FT MP Shop NoShop"
foreach {name} $timers {set rspTime($name) 0.0}
foreach {name} $timers {set cpuTime($name) 0.0}
set cpuTimeLong 0.0
set rspTimeLong 0.0
set countLong 0
set maxClockTime 0
set count(overTime) 0
set xmlCount 0
set noXmlCount 0

setOptions
set theTime [expr [clock seconds] - 86400]
set targetDay [clock format $theTime -format %d]
set day $targetDay
set printDate [clock format $theTime -format "%Y-%m-%d"]
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
set theDay [string trimleft $day "0"]
if {$targetDay != $theDay} {
    set td [string trimleft $targetDay "0"]
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
        puts stderr "Unable to open long xml file: $options(longXML)"
    }
}
set byHour 0
if {[info exists options(byHour)] && $options(byHour) > 0 && $useDatabase} {
    set byHour 1
    foreach {timer} $timers {
        for {set ix 0} {$ix < 24} {incr ix} {
            if {$ix < 10} {set idx "0$ix"} else {set idx $ix}
            set HRsp(${timer}${idx}) 0.0
            set HCpu(${timer}${idx}) 0.0
            set HNum(${timer}${idx}) 0
        }
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
    set fields "$fields,fs.transactionid,fs.transactionstart,fs.homeagency,fs.agentcity"
    set where ""
    if {[info exists options(node)]} {
        set node $options(node)
        if {[string first "%" $node] >=0} {
            append where " and fs.nodeid like '$node'"
        } else {
            if {[string compare [string tolower $node] "farequote"] == 0} {
                append where " and fs.nodeid in $FareDisplayServers"
            } elseif {[string compare [string tolower $node] "faredisplay"] == 0} {
                append where " and fs.nodeid in $FareDisplayServers"
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
        set FDServiceFilter "("
        foreach service $FDServices {
            append FDServiceFilter "'$service', "
        }
        set FDServiceFilter [string range $FDServiceFilter 0 [expr [string length $FDServiceFilter] - 3]]
        append FDServiceFilter ")"
        append where " and fs.servicename in $FDServiceFilter"
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
    set i 0
    set endData [expr $i >= $rows]
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

        set cpu [expr ${cpuused}.0 / 1000000.0]
        set cpuTime(Total) [expr $cpuTime(Total) + $cpu]
        set clockTime [expr ${timeUsed}.0 / 1000000.0]
        set rspTime(Total) [expr $rspTime(Total) + $clockTime]
        if {$cpu > 30.0 || $clockTime > 30.0} {
            # long transaction
            incr countLong
            set cpuTimeLong [expr $cpuTimeLong + $cpu]
            set rspTimeLong [expr $rspTimeLong + $clockTime]
        }
        if {$clockTime > 130.0 && $hasXML} {
            incr count(overTime)
            if {[info exists xmlFd]} {
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

        incr i
        if {$i >= $rows} {
            set rows 0
            incr DBindex
            if {[llength $options(host)] > $DBindex} {
                mysqlclose $db
                unset db
                set host [lindex $options(host) $DBindex]
                if {[info exists options(limit)]} {
                    set maxRows [expr $options(limit) - $count(Total) - 1]
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
                    if {$rows > 0} {puts stderr "$rows rows selected"}
                }
            }
            set i 0
            set endData [expr $i >= $rows]
        }
    } else {
        set inputcmd [string trim [gets $infl]]
        set hasXML 1
        set nodeid "xxxxxxxx"
        set servicename "FARE"
        set cpu 0
        set clockTime 0
        set endData [eof $infl]
        if {$endData} {
            incr DBindex
            if {[llength $options(file)] > $DBindex} {
                close $db
                unset db
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
            if {$count(Total) >=  $options(limit)} {break}
        }
    }
    incr count(Total)
    if {$hasXML} {
        set xml [split $inputcmd]
        set len [llength $xml]
        if {[array exists xmlField]} {unset xmlField}
        for {set j 0} {$j < $len} {incr j} {
            set item [split [lindex $xml $j] "="]
            if {[llength $item] == 2} {
                set name [lindex $item 0]
                set value [string trim [lindex $item 1] " \""]
                set quote [string first "\"" $value]
                if {$quote >= 0} {
                    set value [string range $value 0 [expr $quote - 1]]
                }
                if {[info exists xmlField($name)]} {
                    lappend xmlField($name) $value
                } else {
                    set xmlField($name) $value
                }
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

    if {$hasXML} {
        set reqType 00
        set isLRD 0
        if {[hasKey S58]} {
            set reqType $xmlField(S58)
            if {[string first $reqType "FQ RD FT RB MP"] >= 0} {
                incr count($reqType)
                if {$reqType == "FQ" && [hasKeyValue S82 SDS]} {incr count(SDSQ)}
                if {$reqType == "RD" && ! [hasKey Q46]} {
                    incr count(LRD)
                    set isLRD 1
                }
                if {$reqType == "FT" && [hasKeyValue S82 SDS]} {incr count(SDST)}
            }
        }
        if {[hasKeyValue B00 1B]} {incr count(ABCS)}
        if {[hasKey A20]} {
            set pcc [lindex $xmlField(A20) 0]
            if {[info exists agentPCC($pcc)]} {
                incr agentPCC($pcc)
            } else {
                set agentPCC($pcc) 1
            }
        }
        # city pair
        if {[hasKey A01] && [hasKey A02]} {
            set cityPair [lsort "[lindex $xmlField(A01) 0]
                 [lindex $xmlField(A02) 0]"]
            if {[info exists cityPairs($cityPair)]} {
                incr cityPairs($cityPair)
            } else {
                set cityPairs($cityPair) 1
            }
        }
        
        # calculate response & cpu time by request type
        if {[string first $reqType $timers] >= 0} {
            set cpuTime($reqType) [expr $cpuTime($reqType) + $cpu]
            set rspTime($reqType) [expr $rspTime($reqType) + $clockTime]
            if {$isLRD} {
                set cpuTime(LRD) [expr $cpuTime(LRD) + $cpu]
                set rspTime(LRD) [expr $rspTime(LRD) + $clockTime]
            }
            if {$byHour} {
                set req $reqType
                set transactionTime [lindex $row 6]
                set hIdx [string range [lindex $transactionTime 1] 0  1]
                set HRsp(${req}${hIdx}) [expr $HRsp(${req}${hIdx}) + $clockTime]
                set HCpu(${req}${hIdx}) [expr $HCpu(${req}${hIdx}) + $cpu]
                incr HNum(${req}${hIdx}) 
                if {$isLRD} {
                    set HRsp(LRD${hIdx}) [expr $HRsp(LRD${hIdx}) + $clockTime]
                    set HCpu(LRD${hIdx}) [expr $HCpu(LRD${hIdx}) + $cpu]
                    incr HNum(LRD${hIdx}) 
                }
                set HRsp(Total${hIdx}) [expr $HRsp(Total${hIdx}) + $clockTime]
                set HCpu(Total${hIdx}) [expr $HCpu(Total${hIdx}) + $cpu]
                incr HNum(Total${hIdx}) 
            }
        }
    
        # the rest are calculated for fare quote only
        if {[string compare $reqType "FQ"] == 0} {
            if {[hasKey BI0]} {
                set inclusionCode $xmlField(BI0)
                if {[info exists inclusionCodes($inclusionCode)]} {
                    incr inclusionCodes($inclusionCode)
                } else {
                    set inclusionCodes($inclusionCode) 1
                }
            }
            set findPairs "AC0 AC S07 AC P1Z PV P1Y PL P88 SA P89 SD B70 PT"
            foreach {key id} $findPairs {
                if {[hasKey $key]} {incr count($id)}
            }
            if {[hasKeyValue BI0 ALL]} {incr count(ALL)}
            # count child or infant requested
            # if C, I qualifier presetn or passenger types like CNN, INF, INS
            if {[hasKey P91] || [hasKey P92] || [hasKeyValue B70 "CNN"]
                || [hasKeyValue B70 "INF"] || [hasKeyValue B70 "INS"]} {
                incr count(CI)
            }
            if {[hasKey P87] || [hasMultipleKey B01]} {
                incr count(Shop)
                set cpuTime(Shop) [expr $cpuTime(Shop) + $cpu]
                set rspTime(Shop) [expr $rspTime(Shop) + $clockTime]
            } else {
                incr count(NoShop)
                set cpuTime(NoShop) [expr $cpuTime(NoShop) + $cpu]
                set rspTime(NoShop) [expr $rspTime(NoShop) + $clockTime]
            }
        }
    } else {
        # no XML
        set id [lsearch -exact $FDServices $servicename]
        if {$id >= 0} {
            set reqType [lindex $FDServicesId $id]
            incr count($reqType)
            set cpuTime($reqType) [expr $cpuTime($reqType) + $cpu]
            set rspTime($reqType) [expr $rspTime($reqType) + $clockTime]
        }
        if {[string length $homeAgency] > 0} {
            if {[info exists agentPCC($homeAgency)]} {
                incr agentPCC($homeAgency)
            } else {
                set agentPCC($homeAgency) 1
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
#
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

#
# print results
#   statistics
#
if {$printToLog} {
    set filename "$outputDirectory/FDStatistics.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts $fd "#Date              #Total     %FQ   %RD  %LRD   %RB   %FT   %MP %SDSQ %SDST %ABCS %Shop   %PT   %CI  %ALL   %AC   %SA   %SD   %PV   %PL AveTime  AveCPU"
        puts -nonewline $fd "$printDate"
        puts -nonewline $fd "      "
        set total $count(Total).0
        if {$count(Total) == 0} {set total 1.0}
        foreach {name} $ALLcounters {
            if {[string compare $name Total] == 0} {
                puts -nonewline $fd " [format "%8d" $count(Total)]  "
            } else {                
                set value [expr 100 * $count($name) / $total]
                puts -nonewline $fd " [format "%5.2f" $value]"
            }
         } 
        set total $count(FQ).0
        if {$count(FQ) == 0} {set total 1.0}
        foreach {name} $FQcounters {
            set value [expr 100 * $count($name) / $total]
            puts -nonewline $fd " [format "%5.2f" $value]"
        } 
        if {$count(Total) == 0} {set total 1.0} else {set total $count(Total).0}
        set TotalSeconds $rspTime(Total)
        set TotalCPU $cpuTime(Total)
        puts -nonewline $fd " [format "%7.3f" [expr $TotalSeconds / $total]]"
        puts -nonewline $fd " [format "%7.3f" [expr $TotalCPU / $total]]"
        puts $fd ""
    }
    copyTail "FDStatistics" $fd 1
#   raw counts
    set filename "$outputDirectory/FDCounts.tmp"
    set fd [open $filename "w+"]
    if {! [info exists fd]} {
        puts stderr "Can't create statistics file: $filename"
    } else {
        puts $fd "#Date        #Total       FQ       RD      LRD       RB       FT       MP     SDSQ     SDST     ABCS     Shop       PT       CI      ALL       AC       SA       SD       PV       PL     Long    TotalTime      CPUTime       FQtime        FQcpu       RDtime         RDcpu      LRDtime       LRDcpu       RBtime        RBcpu       FTtime        FTcpu      MPtime        MPcpu     Shop_rsp    Shop_cpu    NoShop_rsp  NoShop_cpu  Long_rsp    Long_cpu"
        puts -nonewline $fd $printDate
        foreach {name} $ALLcounters {
            puts -nonewline $fd " [format "%8d" $count($name)]"
         } 
        foreach {name} $FQcounters {
            puts -nonewline $fd " [format "%8d" $count($name)]"
        } 
        puts -nonewline $fd " [format "%8d" $countLong]"
        foreach {name} $timers {
            puts -nonewline $fd " [format "%12.3f" $rspTime($name)]"
            puts -nonewline $fd " [format "%12.3f" $cpuTime($name)]"
        }
        puts -nonewline $fd " [format "%12.3f%12.3f" $rspTimeLong $cpuTimeLong]"
        puts $fd ""
    }
    copyTail "FDCounts" $fd 1
}

if {$printToStdout} {
    puts ""
    set total $count(Total).0
    if {$count(Total) == 0} {set total 1.0}
    puts "Total = $count(Total)"
    puts "$xmlCount with XML, $noXmlCount without XML"
    set len [llength $counterNames]
    for {set i 1} {$i < $len} {incr i} {
        set id [lindex $counterNames $i]
        set num [format "%7d" $count($id)]
        set percent [format "%5.2f" [expr $count($id) * 100.0 / $total]]
        puts "[format "%6s" $id]: $num $percent% [lindex $counterStrings $i]"
    }
}

#
# print results
#   top 10
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

proc putTop10 {arrayName fileName heading total title} {
    upvar $arrayName data
    global printToLog printToStdout outputDirectory count printDate
    set Top10 [top10 data]
    set separator "#********************************************************"
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
                set line [format "%-5d  %6s %7d   %5.2f" $index "[lindex $pair 0]" $value "[expr 100 * $value / $total]"]
                if {$value > 0} {puts $fd $line}
            }
        }
        copyTail $fileName $fd 0
    }
    if {$printToStdout} {
        puts ""
        puts "Top 10 $arrayName"
        foreach {pair} $Top10  {
            if {[lindex $pair 1] > 0} {
                puts "[lindex $pair 0] = [lindex $pair 1]"
            }
        }
    }
}

set total $count(Total)
set title "Total requests ="
putTop10 cityPairs CityPairs "#rank  CityPair  Count  Percent" $total $title
putTop10 agentPCC PCCs "#rank     PCC   Count  Percent" $total $title
putTop10 inclusionCodes InclusionCodes "#rank  InclCode  Count  Percent" $count(FQ) "Total FQ requests ="

proc printTimers {filename} {
    global printToStdout printToLog outputDirectory
    global timers count rspTime cpuTime printDate
    global countLong cpuTimeLong rspTimeLong
    if {$printToStdout} {
        puts "\nResponse and CPU Time:"
        puts "  Type   Count AveTime   AveCPU"
        foreach {name} $timers {
            if {$count($name) == 0} {set c 1.0} else {set c $count($name)}
            puts -nonewline [format "%6s %7d" $name $count($name)]
            set rTime [expr $rspTime($name) / $c]
            set cTime [expr $cpuTime($name) / $c]
            puts "  [format "%6.3f   %6.3f" $rTime $cTime]"
        }
        if {$countLong == 0} {set c 1} else {set c $countLong}
        puts -nonewline "[format {%6s %7d} {Long} $countLong]"
        set rTime [expr $rspTimeLong / $c]
        set cTime [expr $cpuTimeLong / $c]
        puts "  [format "%6.3f   %6.3f" $rTime $cTime]"
    }
    if {$printToLog} {
        set filenm "$outputDirectory/$filename.tmp"
        set fd [open $filenm "w+"]
        if {! [info exists fd]} {
            puts stderr "Can't create statistics file: $filename"
        } else {
            puts $fd "  Date         Total       Fare Quote   Rule Display    LongRule    BookingCode      FareTax      Mileage       Shopper    Not Shopper    >30 seconds"
            puts $fd "             rsp   cpu     rsp    cpu    rsp    cpu    rsp    cpu    rsp    cpu    rsp    cpu    rsp    cpu   rsp    cpu    rsp    cpu    rsp    cpu"
            puts $fd "           ------------- ------------- ------------- ------------- ------------- ------------- ------------- ------------- ------------- -------------"
#           puts $fd "xxxx-xx-xx 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000 00.000
            puts -nonewline $fd "$printDate"
            foreach {name} $timers {
                if {$count($name) == 0} {set c 1.0} else {set c $count($name)}
                set rTime [expr $rspTime($name) / $c]
                set cTime [expr $cpuTime($name) / $c]
                puts -nonewline $fd " [format "%6.3f %6.3f" $rTime $cTime]"
            }
            if {$countLong == 0} {set c 1} else {set c $countLong}
            set rTime [expr $rspTimeLong / $c]
            set cTime [expr $cpuTimeLong / $c]
            puts -nonewline $fd " [format "%6.3f %6.3f" $rTime $cTime]"
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

printTimers "Performance"
printTimersHtml "Performance"
if {$printToStdout} {
    puts ""
    puts "[array size agentPCC] different agents"
    puts "[array size cityPairs] different city pairs"
    puts "[array size inclusionCodes] different inclusion Codes"
    puts ""

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
    puts "# with response time > 130 sec = $count(overTime)"
    puts "Longest transaction = $maxClockTime seconds"
}

if {$byHour} {
    for {set tp 0} {$tp < [llength $timers]} {incr tp} {
        set timer [lindex $timers $tp]
        puts ""
        puts [lindex $timerStrings $tp]
        puts "Hour Requests AveResponse  AveCpuTime"
        for {set ix 0} {$ix < 24} {incr ix} {
            if {$ix < 10} {set idx "0$ix"} else {set idx $ix}
            puts -nonewline " $idx"
            set n $HNum(${timer}${idx})
            if {$n == 0} {set n 1.0}
            puts -nonewline " [format {%9d} $HNum(${timer}${idx})]"
            set v [expr $HRsp(${timer}${idx}) / $n]
            puts -nonewline "    [format {%8.3f} $v]"
            set v [expr $HCpu(${timer}${idx}) / $n]
            puts "    [format {%8.3f} $v]"
        }
    }
}

exit 0

