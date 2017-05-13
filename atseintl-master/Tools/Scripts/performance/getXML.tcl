#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# getXML.tcl
#   Extract xml requests from the database and store in a file.
# Author: Jim Sinnamon
# Copyright: Sabre 2007
# ------------------------------------------------------------------

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
    for {set i 0} {$i < [llength $pathList]} {incr i} {
        set libdir [lindex $pathList $i]
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
    --host=hostname     Use a FSRESPONSE database
    --table=tablename   Name of the table in the database
    --node=nodeid       Node name (application server)
    --service=service   Service id (e.g. INTL% or FAREDSP1)
    --pcc=pccCode       One or more pseudo city codes
    --day=dayOfMonth    Table suffix (--day=02 uses FSRESPONSE02)
    --port=portNumber   TCP port for the database (default=3306)
    --limit=maxRows     Limit the number of transactions
    --printXML=1        Print the decoded xml to stdout
    --printTags=1       Count the XML tags and print
    --output=filename   Output file
    --skip=number       Skip items
    --minCPU=seconds    Minimum cpu time
    --maxCPU=seconds    Maximum cpu time
    --minRSP=seconds    Minimum response time
    --maxRSP=seconds    Maximum response time
    --matchLimit=n      Number of matches to cpu time
    --noNewline=n       Remove newlines from the xml
    --findString=text   String that must be in the xml
    --axess             Special query for axess
    --special           Custom query
Special options:
    --node=FareQuote    Select all fareQuote servers
    --node=Pricing      All pricing servers
    --node=ShoppingIS   ShoppingIS servers
    --node=ShoppingMIP  ShoppingMIP servers
    --node=Shopping     All shopping servers
"
}

proc setOptions {} {
    global options argv
    set optionNames "host table node service pcc day port limit printXML printTags output"
    set optionNames "$optionNames skip maxCPU minCPU matchLimit noNewline findString"
    set optionNames "$optionNames maxRSP minRSP axess special"
    for {set i 0} {$i < [llength $argv]} {incr i} {
        set tag [string range [lindex $argv $i] 0 1]
        set opt [split [lindex $argv $i] "="]
        if {[llength $opt] == 2 && [string compare $tag "--"] == 0} {
            set optName "[string range [lindex $opt 0] 2 end]"
            set value [split [lindex $opt 1] ","]
            if {[info exists options($optName)]} {
                set options($optName) "$options($optName) $value"
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

set FareQuoteServers "('piclp002', 'piclip003', 'piclp006', 'piclp007', 'piclp008', 'piclp143', 'piclp144', 'piclp168', 'piclp219', 'piclp220', 'piclp221', 'piclp222', 'piclp223', 'piclp224', 'piclp225', 'piclp226', 'piclp260', 'pimlp038', 'pimlp039', 'pimlp040', 'pimlp041', 'pimlp042')"

set ShoppingIS "'piclp146', 'piclp157', 'piclp158', 'piclp159', 'piclp160', 'piclp195', 'piclp196', 'piclp197', 'piclp198', 'piclp199', 'piclp200', 'piclp201', 'piclp202', 'piclp203', 'piclp204', 'piclp205', 'piclp206', 'piclp207', 'piclp208', 'piclp209', 'piclp210', 'piclp211', 'piclp212', 'pimlp008', 'pimlp009', 'pimlp010', 'pimlp011', 'pimlp012', 'pimlp035', 'pimlp036', 'pimlp037', 'pimlp059', 'pimlp060', 'pimlp061', 'pimlp063', 'pimlp064', 'pimlp065', 'pimlp066', 'pimlp067', 'pimlp077', 'pimlp078'"

set ShoppingMIP "'piclp172', 'piclp173', 'piclp174', 'piclp175', 'piclp176', 'piclp177', 'piclp178', 'piclp179', 'piclp180', 'piclp181', 'piclp182', 'piclp183', 'piclp184', 'piclp185', 'piclp186', 'piclp187', 'piclp188', 'piclp189', 'piclp190', 'piclp191', 'piclp192', 'piclp193', 'piclp194', 'piclp213', 'piclp214', 'piclp218', 'piclp227', 'piclp228', 'piclp229', 'piclp230', 'piclp231', 'piclp236', 'piclp237', 'piclp238', 'piclp239', 'piclp240', 'piclp241', 'piclp242', 'piclp243', 'piclp244', 'piclp245', 'piclp246', 'piclp247', 'piclp248', 'piclp249', 'piclp250', 'piclp251', 'piclp252', 'piclp253', 'piclp261', 'piclp262', 'piclp263', 'piclp264', 'piclp265', 'piclp266', 'piclp267', 'piclp268', 'piclp269', 'piclp270', 'piclp271', 'piclp272', 'piclp273', 'piclp274', 'piclp275', 'piclp276', 'piclp277', 'piclp278', 'piclp279', 'piclp280', 'piclp281', 'piclp282', 'piclp283', 'piclp284', 'piclp285', 'piclp286', 'piclp287', 'piclp288', 'piclp289', 'piclp290', 'piclp291', 'piclp292', 'piclp293', 'piclp294', 'piclp295', 'piclp296', 'piclp297', 'piclp564', 'piclp565', 'piclp566', 'piclp567', 'piclp568', 'piclp569', 'piclp570', 'piclp571', 'piclp572', 'piclp573', 'piclp574', 'piclp575', 'piclp576', 'piclp577', 'piclp578', 'pimlp013', 'pimlp014', 'pimlp015', 'pimlp016', 'pimlp017', 'pimlp018', 'pimlp019', 'pimlp020', 'pimlp021', 'pimlp022', 'pimlp023', 'pimlp024', 'pimlp025', 'pimlp026', 'pimlp027', 'pimlp028', 'pimlp029', 'pimlp030', 'pimlp031', 'pimlp032', 'pimlp033', 'pimlp034', 'pimlp050', 'pimlp051', 'pimlp052', 'pimlp053', 'pimlp054', 'pimlp055', 'pimlp056', 'pimlp057', 'pimlp079', 'pimlp080', 'pimlp081', 'pimlp082', 'pimlp083', 'pimlp084'"
set ShoppingISServers "(${ShoppingIS})"
set ShoppingMIPServers "(${ShoppingMIP})"
set ShoppingServers "(${ShoppingIS}, ${ShoppingMIP})"

set reqCounters "PricingRequest CurrencyConversionRequest MileageRequest AltPricingRequest SelectionRequest Long"

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
set saveCount 0

setOptions
set theTime [expr [clock seconds] - 86400]
set targetDay [clock format $theTime -format %d]
set day $targetDay
set printDate [clock format $theTime -format "%Y-%m-%d"]
if {! [info exists options(printXML)]} {set options(printXML) 0}
set printXML [expr ! [regexp {^[nN0]} $options(printXML)]]
if {! [info exists options(printTags)]} {set options(printTags) 0}
set printTags [expr ! [regexp {^[nN0]} $options(printTags)]]

set minCPU 0
set maxCPU 1000
set minRSP 0
set maxRSP 1000
set skipCount 0
set removeNewLine 0
set axess 0
set special 0
if {[info exists options(minCPU)]} {set minCPU $options(minCPU)}
if {[info exists options(maxCPU)]} {set maxCPU $options(maxCPU)}
if {[info exists options(minRSP)]} {set minRSP $options(minRSP)}
if {[info exists options(maxRSP)]} {set maxRSP $options(maxRSP)}
if {[info exists options(skip)]} {set skipCount $options(skip)}
if {[info exists options(noNewline)]} {set removeNewLine $options(noNewline)}
if {[info exists options(axess)]} {set axess $options(axess)}
if {[info exists options(special)]} {set special $options(special)}

if {[info exists options(output)]} {
    set outFl [open $options(output) a]
    if {! [info exists outFl]} {
        puts stderr "Can't open output file: $options(output)"
        exit 1
    }
} else {
    set outFl stdout
}

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
    set pseudoField homeagency
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
            if {$axess || $special} {
                set table "BASEINSTR$options(day)"
                set pseudoField homeagency
            }
            set targetDay $options(day)
        } else {
            set table "FSRINSTR$targetDay"
            if {$axess || $special} {
                set table "BASEINSTR$targetDay"
                set pseudoField homeagency
            }
        }
    }
} else {
    puts stderr "--host is required"
    printUsage
    exit 1
}
if {$targetDay != $day} {
    set d1 [string trimleft $targetDay 0]
    set d2 [string trimleft $day 0]
    set theTime [expr $theTime + 86400 * ($d1 - $d2)]
    set printDate [clock format $theTime -format "%Y-%m-%d"]
}
    
set DBindex 0

set db [mysqlconnect -host $host -port $port -user hybfunc -db ATSEHYB2]
if { ! [info exists db] } {
    puts stderr "can't connect to mysql database"
    exit 2
}
puts stderr "connected to ATSEHYB2 on $host"
set where ""
if {[info exists options(node)]} {
    set node $options(node)
    if {[string first "%" $node] >=0} {
        set where "$where and fs.nodeid like '$node'"
    } else {
        if {[string compare [string tolower $node] "farequote"] == 0} {
            set where "$where and fs.nodeid in $FareQuoteServers"
        } elseif {[string compare [string tolower $node] "pricing"] == 0} {
            set where "$where and fs.nodeid in $PricingServers"
        } elseif {[string compare [string tolower $node] "shoppingis"] == 0} {
            set where "$where and fs.nodeid in $ShoppingISServers"
        } elseif {[string compare [string tolower $node] "shoppingmip"] == 0} {
        } elseif {[string compare [string tolower $node] "shopping"] == 0} {
            set where "$where and fs.nodeid in $ShoppingServers"
        } else {
            set where "$where and fs.nodeid='$node'"
        }
    }
}
if {[info exists options(service)]} {
    if {[string first "%" $options(service)] >=0} {
        set where "$where and fs.servicename like '$options(service)'"
    } elseif {[llength $options(service)] > 1} {
        append where " and fs.servicename in ("
        foreach nm $options(service) {
            append where "'$nm',"
        }
        regsub ",\$" $where ")" where
    } else {
        set where "$where and fs.servicename='$options(service)'"
    }
}
if {[info exists options(pcc)]} {
    puts stderr "options(pcc) = '$options(pcc)'"
    if {[llength $options(pcc)] > 1} {
        set len [llength $options(pcc)]
        set where "$where and fs.$pseudoField in ("
        for {set pccNum 0} {$pccNum < $len} {incr pccNum} {
            if {$pccNum != 0} {set where "$where ,"}
            set where "$where \'[lindex $options(pcc) $pccNum]\'"
        }
        set where "${where})"
    } else {
        if {[string first "%" $options(pcc)] >=0} {
            set where "$where and fs.$pseudoField like '$options(pcc)'"
        } else {
            set where "$where and fs.$pseudoField='$options(pcc)'"
        }
    }
}
if {[info exists options(limit)]} {
    set maxRows [expr $options(limit) + $skipCount]
} else {
    set maxRows 2000000
}
if {[info exists options(matchLimit)]} {
    set matchLimit $options(matchLimit)
} else {
    set matchLimit $maxRows
}
if {[info exists options(skip)]} {set skipCount $options(skip)}

set fields "fs.nodeid,fs.servicename,trim(xml.inputcmd),fs.cpuused,fs.elapsedtime"
set using "using (servicename,nodeid,transactionid,clienttransactionid)"
set join "left outer join XMLREQUEST$targetDay as xml"
if {$axess} {
    append fields ",fs.pseudocity, fs.aaacity,fs.lniata,fs.transactionid,fs.transactiondatetime"
    append where " and fs.nodeid in ('picli404', 'picli408', 'pimli003') and fs.aaacity in ('A0NC', 'J8S4', '4ZJ')"
    append where " and fs.pseudocity not in ('HDQ') and fs.servicename not in ('INTDWPI1', 'INTLWPI1', 'FAREDISP', 'RULEDISP')"
} elseif {$special} {
    append fields ",fs.pseudocity, fs.aaacity,fs.lniata,fs.transactionid,fs.transactiondatetime"
    append where " and transactiondatetime >= '2008-06-27 07:15:00' and transactiondatetime <= '2008-06-27 07:45:00'"
} else {
    append fields ",fs.agentcity"
}

set maxCpu 0
set minCpu 1000000

puts stderr "using table $table"
append where " and xml.inputcmd is not null"
set where "where [string range $where 4 end]"
set limit "limit $maxRows"
#######
# set where "where fs.servicename = 'INTLWPI1' and fs.nodeid = 'piclp013' and fs.transactiondatetime < '2008-06-02 07:55:28' and fs.transactiondatetime > '2008-06-02 07:51'"
# set table BASEINSTR02
# regsub agentcity $fields pseudocity fields
######
puts stderr "select $fields from $table as fs $join $using $where $limit"
set rows [mysqlsel $db "select $fields from $table as fs $join $using $where $limit"]
puts stderr "$rows rows selected"
set rowNumber 0
set endData [expr $rowNumber >= $rows]

while {$endData == 0} {
    set row [mysqlnext $db]
    set nodeid [lindex $row 0]
    set servicename [lindex $row 1]
    set inputcmd [string trim [lindex $row 2]]
    set cpuused [lindex $row 3]
    set timeUsed [lindex $row 4]
    set pcc [lindex $row 5]
    if {$axess || $special} {
        set aaacity [lindex $row 6]
        set lniata [lindex $row 7]
        set transactionid [lindex $row 8]
        set transactiondatetime [lindex $row 9]
    }

    if {[string length [string trim $cpuused]] == 0} {set cpuused 0}
    if {[string length [string trim $timeUsed]] == 0} {set timeUsed 0}
    set cpu [expr $cpuused / 1000000.0]
    set clockTime [expr $timeUsed / 1000000.0]
    if {$cpu > 0.0 || $clockTime > 0.0} {
        set cpuTime(Timed) [expr $cpuTime(Timed) + $cpu]
        set rspTime(Timed) [expr $rspTime(Timed) + $clockTime]
        incr countTimed
        if {$clockTime > 30.0} {
            incr count(Long)
            set cpuTime(Long) [expr $cpuTime(Long) + $cpu]
            set rspTime(Long) [expr $rspTime(Long) + $clockTime]
        }
        if {$cpu > $maxCpu} {set maxCpu $cpu}
        if {$cpu < $minCpu} {set minCpu $cpu}
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
                set maxRows 2000000
            }
            set db [mysqlconnect -host $host -port $port -user hybfunc -db ATSEHYB2]
            if { ! [info exists db] } {
                puts stderr "can't connect to mysql database on $host"
                set rows 0
            } else {
                puts stderr "connected to ATSEHYB2 on $host"
                puts stderr "using table $table"
                set limit "limit $maxRows"
                set rows [mysqlsel $db "select $fields from $table $where $limit"]
                if {$rows > 0} {puts stderr "$rows rows selected"}
            }
        }
        set rowNumber 0
        set endData [expr $rowNumber >= $rows]
    }
    incr countTotal
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
    if {($countTotal > $skipCount) && ($cpu >= $minCPU) && ($cpu <= $maxCPU) && ($clockTime >= $minRSP) && ($clockTime <= $maxRSP)} {
    # any required filtering
        set complete 1
        if {[info exists options(findString)]} {
            if {[string first $options(findString) $inputcmd] < 0} {set complete 0}
        }
        if {$complete && $removeNewLine} {
            regsub -all "\n" $inputcmd " " inputcmd
        }
        if {$complete} {
            if {$axess || $special} {
                puts -nonewline $outFl "$transactiondatetime nodeid=\"$nodeid\" "
                puts -nonewline $outFl "servicename=\"$servicename\" "
                puts -nonewline $outFl "cpu=\"$cpuused\" elapsed=\"$timeUsed\" "
                puts -nonewline $outFl "pcc=\"$pcc\" aaacity=\"$aaacity\" "
                puts -nonewline $outFl "lniata=\"$lniata\" "
            }
            puts $outFl $inputcmd
            incr saveCount
            if {$saveCount >= $matchLimit} {break}
        }
    }
}

puts "minCPU = $minCPU"
puts "maxCPU = $maxCPU"
puts "minRSP = $minRSP"
puts "maxRSP = $maxRSP"
puts "skipCount = $skipCount"
puts "cpu = $cpu"
puts "match limit = $matchLimit"

#
# done reading
#
if {$useDatabase} {
    if {[info exists db]} {mysqlclose $db}
}
if {[string compare $outFl "stdout"] != 0} {close $outFl}

#
#  -- print results --
#     procedures
#

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

#
# print results
#
puts ""
puts "Total requests: $countTotal"
puts "     Saved XML: $saveCount"
puts "   Maximum CPU: $maxCpu"
puts "   Minimum CPU: $minCpu"
if {$countTimed > 0} {
    puts "   Average CPU: [expr $cpuTime(Timed) / $countTimed]"
}

exit 0


