#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# warmupHistorical.tcl
#   Send transactions to a historical server to initialize the cache
#   XML for transactions is read from the instrumentation database
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set useDebug 1
set consolePort 5006
set port 53701
set databasePort 3306
set database pimhi007
# CAT-33 = INTLCT33
set serviceNames "('INTLHIST', 'FAREHIST', 'RULEHIST', 'FARMHIST', 'INTLEXNH', 'INTLEXNC', 'INTLEXHH', 'INTLEXHC','INTLCT31','INTLCT33')"
# A3 = INTLCT31
#set serviceNames "('INTLHIST', 'FAREHIST', 'RULEHIST', 'FARMHIST', 'INTLEXNH', 'INTLEXNC', 'INTLEXHH', 'INTLEXHC','INTLCT31')"
# A2 Port Exchange = INTLEXNH INTLEXNC INTLEXHH INTLEXHC
#set serviceNames "('INTLHIST', 'FAREHIST', 'RULEHIST', 'FARMHIST', 'INTLEXNH', 'INTLEXNC', 'INTLEXHH', 'INTLEXHC')"
#set serviceNames "('BKGCHIST', 'DCCAHIST', 'FAREHIST', 'INTLHIST', 'MILGHIST', 'RULEHIST')"
set tableBase "BASEINSTR"
set transactions 10000
set loadLimit 3.0
set transactionLimit 10000
set lateTransactions 1000
set lateElapsed 6.0
set lateActive 3
set lateLoad 2.0
set timeLimit 0
set retryDelay 20
set clients 10
set appConsoleTimeout 15
set appConsoleInterval 60
set trxTimeout 150
set pollInterval 10
set exitWhenWarm 0
set logFile "warmupHistorical.log"
set maxLogFile [expr 1024*1024*10]
set defaultXML "warmupHistorical.xml"
set mysqlLibPath "/opt/atseintl/3rdParty/lib/mysql_shared-4.0.22"
set cacheList "FAREHISTORICAL"
set cacheMinimum   "10000"
set cacheLowLoad   "4000"
set cacheFrequency 2

package require Expect

proc printUsage {} {
    global argv0 consolePort port databasePort transactions loadLimit
    global tableBase database transactionLimit timeLimit clients exitWhenWarm
    global lateTransactions lateElapsed lateActive lateLoad logFile
    puts stderr "
Usage(client): [file tail $argv0] \[OPTIONS...\] <tseserver> \[OPTIONS...\]
    <tseserver>         host:port for teseerver (example: picli404:53701)
Options:
    --verbose              display inputs, outputs and messages
    --help                 display usage
    --host=hostname        Name or ip address of the server
    --port=number          tcp port for xml (default=$port)
    --consolePort=number   tcp port for the application console (default=$consolePort)
    --database=name        The instrumentation database for XMLs (default=$database)
    --databasePort=number  The instrumentation database TCP port (default=$databasePort)
    --day=number           Day of the month for ${tableBase}xx (default=yesterday)
    --transactions=number  Target transaction count on the server (default=$transactions)
    --loadLimit=number     Maximum server load in transactions/second (default=$loadLimit)
    --transactionLimit=n   Maximum transactions from database (default=$transactionLimit)
    --timeLimit=seconds    Maximum clock time (default=$timeLimit)
    --clients=number       Maximum number of simultaneous transactions (default=$clients)
    --skip=number          Skip n XMLs (default=TRX count on server)
    --progress=number      Report progress every n transactions
    --exitWhenWarm         Terminate when transaction count is reached (default=$exitWhenWarm)
    --logFile=filename     file for status messages (default=$logFile)
    --xmlFile=filename     File containing XML
    --lateTransactions=n   Switch to a lower rate at n transactions (default=$lateTransactions)
    --lateElapsed=n        Switch to a lower rate at when elapsed time below n (default=$lateElapsed)
    --lateActive=n         The lower limit on concurrent transactions (default=$lateActive)
    --lateLoad=n           The lower limit on server transactions/sec (default=$lateLoad)        
"
}

set allOptions "verbose help host port consolePort database databasePort day transactions"
append allOptions " loadLimit transactionLimit timeLimit clients skip progress logFile exitWhenWarm"
append allOptions " xmlFile lateTransactions lateElapsed lateActive lateLoad printXML"

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
            if {[info exists options($optName)]} {
                lappend options($optName) $value
            } else {
                set options($optName) $value
            }
            if {[string first $optName $allOptions] < 0} {
                putError "Unknown option: $optName"
                printUsage
                exit 1
            }
        } else {
            lappend args $opt
        }
    }
    return $args
}

proc loadMySql {} {
    global env mysqlLibPath
    set libFound 0
    set file libmysqltcl2.51
    # change to libmysqltcl3.03 when you upgrade to mysql 4.1 or higher
    set ext [info sharedlibextension]
    set libdir "."
    if {[info exists env(HOME)]} {
        lappend libdir "$env(HOME)/bin"
        lappend libdir "$env(HOME)/lib"
        lappend libdir "$env(HOME)/opt/mysqltcl-2.51"
    }
    lappend libdir "/opt/atseintl/FareDisplay/mysqltcl-2.51"
    set libFound 0
    foreach dir $libdir {
        if {[file isfile $dir/${file}$ext]} {
            if {[info exists env(LD_LIBRARY_PATH)]} {append env(LD_LIBRARY_PATH) ":"}
            append env(LD_LIBRARY_PATH) "$dir"
            append env(LD_LIBRARY_PATH) ":$mysqlLibPath"
            if {! [catch {load $dir/${file}$ext}]} {
                set libFound 1
                break;
            }
        }
    }
    if {! $libFound} {
        foreach dir [split $env(LD_LIBRARY_PATH) ":"] {
            if {[file isfile $dir/${file}$ext]} {
                append env(LD_LIBRARY_PATH) ":$dir"
                if {! [catch {load $dir/${file}$ext}]} {
                    set libFound 1
                    break
                }
            }
        }
    }
    return $libFound
}

proc putLog {message} {
    global log verbose
    if {[string compare $log stdout] != 0} {
        puts $log $message
        flush $log
        if {$verbose} {puts $message}
    } else {
        puts "$message"
        flush stdout
    }
}

proc putError {message} {
    global log
    puts stderr $message
    if {[string compare $log stdout] != 0} {
        puts $log $message
        flush $log
    }
}

proc dbg {string} {
    global useDebug
    if {$useDebug} {puts $string}
}

proc bgerror {error} {
    global errorCode errorInfo
    putError $error
    putError $errorCode
    putError $errorInfo
}   

proc printStatus {} {
    global trxCur rate trxActive trxSent socketList clients activeClients readTime
    global elapsedRate cpuRate transactions
    set now [clock seconds]
    set timeStamp [clock format $now -format "%Y-%m-%d %H:%M:%S"]
    putLog "At $timeStamp $trxCur TRX, [format {%6.3f} $rate] TPS, $trxActive Concurrent"
    putLog "Ave Elapsed = [format {%6.3f} $elapsedRate] Ave CPU = [format {%6.3f} $cpuRate]"
    set toGo [expr $transactions - $trxCur]
    if {$toGo < 0} {set toGo 0}
    putLog "Sent = $trxSent, Target = $transactions, To Go = $toGo, running = $activeClients"
    for {set i 0} {$i < $clients} {incr i} {
        if {$socketList($i) != 0} {
            putLog "  Client $i at [expr $now - $readTime($i)] seconds"
        }
    }
    return 0
}   
    
proc exitSignal {} { 
    global trxCur done clients socketList readTime
    printStatus
    putLog "Exit signal received at $trxCur"
    set now [clock seconds]
    for {set i 0} {$i < $clients} {incr i} {
        if {$socketList($i) != 0} {
            putLog "Terminating client $i at [expr $now - $readTime($i)] seconds"
            timeout $i
        }
    }
    set done 1
}

proc sendAppConsole {fd cmd} {
    global appConsoleStatus appConsoleBusy appConsoleError appConsoleSendTime
    global appConsoleTimeout
    set appConsoleStatus ""
    set payloadSize 0
    set hdr $cmd
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]
    set appConsoleError [catch {puts -nonewline $fd $cmd}]
    if {$appConsoleError} {
        putError "Error sending to application console"
        return -1
    }
    set appConsoleError [catch {flush $fd}]
    set appConsoleBusy 1
    set appConsoleSendTime [clock seconds]
    fileevent $fd readable [list receiveAppConsole $fd]
    after [expr $appConsoleTimeout * 1000] checkAppConsole
    return 0
}

proc receiveAppConsole {fd} {
    global appConsoleStatus appConsoleBusy appConsoleError appConsoleTime
    global lastConsoleTime appConsoleInterval
    global expectCache cacheStatus cacheTime cacheCounter cacheFrequency cacheReads
    set appConsoleError [catch {set hdr [read $fd 4]}]
    if {$appConsoleError || [string length $hdr] < 4} {
        putError "Error reading application console: headerSize"
        set appConsoleError 1
        fileevent $fd readable ""
        return -1
    }
    binary scan $hdr I headerSize
    set appConsoleError [catch {set hdr [read $fd 4]}]
    if {$appConsoleError || [string length $hdr] < 4} {
        putError "Error reading application console: payloadSize"
        set appConsoleError 1
        fileevent $fd readable ""
        return -1
    }
    binary scan $hdr I payloadSize
    set appConsoleError [catch {set header [read $fd [expr $headerSize - 4]]}]
    if {$appConsoleError} {
        putError "Error reading application console: header"
        fileevent $fd readable ""
        return -1
    }
    set appConsoleError [catch {set payload [read $fd $payloadSize]}]
    if {$appConsoleError || [string length $payload] != $payloadSize} {
        putError "Error reading application console: payload"
        set appConsoleError 1
        fileevent $fd readable ""
        return -1
    }
    set appConsoleBusy 0
    fileevent $fd readable ""
    after cancel checkAppConsole
    if {[regexp "^<STATS" $payload]} {
        if {$expectCache} {putError "Expected cache, received stats"}
        set appConsoleTime [clock seconds]
        set appConsoleStatus [string trim $payload "\000"]
        if {$appConsoleTime != $lastConsoleTime} {calculateRate}
        after [expr $appConsoleInterval * 1000] {sendAppConsole $console "RFSH00010000"}
        incr cacheCounter
        if {$cacheCounter >= $cacheFrequency || $cacheReads == 0} {
            set cacheCounter 0
            sendAppConsole $fd "TATS00010000"
            set expectCache 1
            incr cacheReads
        }
    } else {
        if {! $expectCache} {putError "Expected stats, received cache"}
        set cacheStatus [string trim $payload "\000"]
        set cacheTime [clock seconds]
        set expectCache 0
        checkCache
    }
    return $payload
}

proc connectAppConsole {host consolePort} {
    global console appConsoleBusy appConsoleError
    set appConsoleBusy 0
    if {[info exists console]} {
        catch {close $console}
        unset console
    }
    if {[catch {set console [socket $host $consolePort]}]} {
        putError "Can't connect to AppConsole on $host:$consolePort"
        set appConsoleError 1
        return -1
    }
    if {! [info exists console]} {
        putError "Can't connect to AppConsole on $host:$consolePort"
        set appConsoleError 1
        return -1
    }
    putLog "Connected to application console on $host:$consolePort"
    if {[sendAppConsole $console "RFSH00010000"] < 0} {
        putError "Can't send to AppConsole on $host:$consolePort"
        set appConsoleError 1
        return -1
    }
    return 0
}

proc checkAppConsole {} {
    global host consolePort console appConsoleTime lastConsoleTime retryDelay
    global appConsoleStatus appConsoleBusy appConsoleError appConsoleTimeout
    global appConsoleSendTime
    if {$appConsoleError} {
        if {[connectAppConsole $host $consolePort] < 0} {
            set appConsoleError 1
            after [expr $retryDelay * 1000] checkAppConsole
            return -1
        }
        after [expr $appConsoleTimeout * 1000] checkAppConsole
        return 0
    }
    set now [clock seconds]
    if {$appConsoleBusy} {
        if {($now - $appConsoleSendTime) > $appConsoleTimeout} {
            if {[connectAppConsole $host $consolePort] < 0} {
                set appConsoleError 1
                after [expr $retryDelay * 1000] checkAppConsole
                return -1
            }
        }
        after [expr $appConsoleTimeout * 1000] checkAppConsole
        return 0
    }
    if {$appConsoleTime != $lastConsoleTime} {calculateRate}
    return 0
}

proc calculateRate {} {
    global appConsoleTime lastConsoleTime appConsoleStatus appConsoleInterval 
    global trxCur trxLast newTrx warm restartTrx trxSent activeClients trxCustomer
    global trxNumber rate trxActive verbose progress elapsedBelow lateElapsed
    global lastElapsed lastCpu elapsedRate cpuRate sampleCount trxList elapsedList
    global loadLimit maxActive earlyLoad clients cacheSize lastRestartTrx
    set trxActive [getConcurrent $appConsoleStatus]
    set trxCustomer [expr $trxActive + 1 - $activeClients]
    if {$appConsoleTime - $lastConsoleTime >= $appConsoleInterval} {
        set trxCur [getTrxCount $appConsoleStatus]
        if {$trxCur < $trxLast} {
            # server must have restarted
            putLog "Server restart: Trx count from $trxLast to $trxCur"
            set trxNumber $trxCur 
            set trxLast 0
            set restartTrx $trxSent
            set warm 0
            set sampleCount 0
            set maxActive $clients
            set loadLimit $earlyLoad
            set trxCustomer $maxActive
            set elapsedList ""
            set trxList ""
            set lastRestartTrx $trxCur
        }
        set rate [expr 1.0 * ($trxCur - $trxLast) / ($appConsoleTime - $lastConsoleTime)]
        set elapsed [getElapsedTime $appConsoleStatus]
        set cpu [getCpuTime $appConsoleStatus]
        if {$trxCur > $trxLast} {
            set elapsedRate [expr 1.0 * ($elapsed - $lastElapsed)/($trxCur - $trxLast)]
            set cpuRate [expr 1.0 * ($cpu - $lastCpu)/($trxCur - $trxLast)]
        } else {
            set elapsedRate 0
            set cpuRate 0
        }
        set trxLast $trxCur 
        set lastElapsed $elapsed
        set lastCpu $cpu
        set newTrx 0
        set lastConsoleTime $appConsoleTime
        if {$elapsedRate > 0 && $elapsedRate < $lateElapsed} {
            incr elapsedBelow
        } else {
            set elapsedBelow 0
        }
        if {$verbose || $progress > 0} {
            set timeStamp [clock format $appConsoleTime -format "%Y-%m-%d %H:%M:%S"]
            set msg  "At $timeStamp $trxCur TRX, [format {%6.3f} $rate] TPS, $trxActive Concurrent"
            append msg ", Ave Elapsed = [format {%6.3f} $elapsedRate]"
            append msg ", Ave CPU = [format {%6.3f} $cpuRate]"
            append msg ", Fare=$cacheSize(FAREHISTORICAL)"
            if {$sampleCount >= 5} {
                set et [lindex $elapsedList 0]
                set tx [lindex $trxList 0]
                set elapsedList [lrange $elapsedList 1 4]
                set trxList [lrange $trxList 1 4]
                if {$trxCur != $tx} {
                    append msg ", Elapsed(5) = [format {%6.3f} [expr 1.0 * ($elapsed - $et)/($trxCur - $tx)]]"
                }
            }
            incr sampleCount
            lappend elapsedList $elapsed
            lappend trxList $trxCur
            putLog $msg
        }
    }
}

proc checkCache {} {
    global cacheList cacheSize cacheMin cacheLow
    global warm cacheLate transactions transactionsLow
    global transactionsFull trxCur lateTransactions maxActive clients
    global loadLimit earlyLoad maxActive lastRestartTrx
    global cacheStatus cacheTime lastCacheTime cacheList cacheOK cacheLate 
    if {$cacheTime == $lastCacheTime} {return}
    set lastCacheTime $cacheTime
    set lines [split $cacheStatus |]
    set cacheCount [expr [llength $lines] / 5]
    for {set i 0} {$i < $cacheCount} {incr i} {
        set idx [expr $i * 5]
        set data [lrange $lines $idx [expr $idx + 4]]
        set name [lindex $data 0]
        if {[lsearch -exact $cacheList $name] >= 0} {
            set cacheSize($name) [lindex $data 2]
        }
    }
    set OK 1
    set Late 1
    set okPercent 0
    set latePercent 0
    foreach name $cacheList {
        if {$cacheSize($name) < $cacheMin($name)} {
            set OK 0
            set percent [expr (1.0 * ($cacheMin($name) - $cacheSize($name))) / $cacheMin($name)]
            if {$percent > $okPercent} {set okPercent $percent}
        }
        if {$cacheSize($name) < $cacheLow($name)} {
            set Late 0
            set percent [expr (1.0 * ($cacheLow($name) - $cacheSize($name))) / $cacheLow($name)]
            if {$percent > $latePercent} {set latePercent $percent}
        }
    } 
    set fullPass [expr $Late && ($trxCur - $lastRestartTrx) >= $transactionsFull]
    if {($trxCur >= $transactions) && (! $OK) && (! $fullPass)} {
        set warm 0
        putLog "Cache Size: warmup resumed"
        if {$Late} {
            set transactions [expr $trxCur + int($okPercent * $transactionsFull) + 1]
            set lateTransactions $trxCur
        } else {
            set transactions [expr $trxCur + int($okPercent * $transactionsFull) + 1]
            set lateTransactions [expr $trxCur + int($latePercent * $transactionsLow) + 1]
            set maxActive $clients
            set loadLimit $earlyLoad
            set lastRestartTrx $trxCur
        }
    }
    set cacheOK $OK
    set cacheLate $Late
}

proc getTrxCount {stats} {
    set trxLoc [string first "NM=\"TRX\"" $stats]
    if {$trxLoc < 0} {return 0}
    set t [string range $stats $trxLoc end]
    set endLoc [string first "/>" $t]
    if {$endLoc > 0} {set t [string range $t 0 $endLoc]}
    set tags [split $t " "]
    set trx 0
    foreach tag $tags {
        if {[regexp "OK=\"(\[0-9]+)\"" $tag match val]} {
            set trx [expr $trx + $val]
        } elseif {[regexp "ER=\"(\[0-9]+)\"" $tag match val]} {
            set trx [expr $trx + $val]
        }
    }
    return $trx
}

proc getConcurrent {stats} {
    if {[regexp "CT=\"(\[0-9]+)\"" $stats match val]} {
        return $val
    }
    return 16
}

proc getElapsedTime {stats} {
    set trxLoc [string first "NM=\"TRX\"" $stats]
    if {$trxLoc < 0} {return 0}
    set t [string range $stats $trxLoc end]
    set endLoc [string first "/>" $t]
    if {$endLoc > 0} {set t [string range $t 0 $endLoc]}
    if {[regexp "ET=\"(\[0-9\\.\]+)\"" $t match value]} {
        return $value
    }
    return 0
}

proc getCpuTime {stats} {
    if {[regexp "CP=\"(\[0-9\\.\]+)\"" $stats match value]} {
        return $value
    }
    return 0
}

proc sendXML {xml client} {
    global host port verbose trxSent newTrx activeClients
    global socketList readState readCount readSize readBuffer readTime
    if {$verbose} {putLog "Sending $xml"}
    set err [catch {set sock [socket $host $port]}]
    if {$err || (! [info exists sock])} {
        putError "Can't open tseserver socket to: $host:$port"
        return -1
    }
    fconfigure $sock -translation binary
    set payloadSize [string length $xml]
    set hdr "RESP00010000"
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]
    set err [catch {puts -nonewline $sock ${cmd}${xml}}]
    if {$err} {
        putError "Socket error sending XML " $client"
        catch {close $sock}
        return -1
    }
    if {[catch "flush $sock"]} {
        putError "Socket error sending XML in flush : $client"
        catch {close $sock}
        return -1
    }
    incr trxSent
    incr newTrx
    incr activeClients
    if {$verbose} {putLog "Waiting for response : $client"}
    set socketList($client) $sock
    set readState($client) "size"
    set readCount($client) 0
    set readSize($client) 8
    set readBuffer($client) ""
    set readTime($client) [clock seconds]
    fconfigure $sock -blocking 0
    fileevent $sock readable [list readResponse $sock $client]
    return 0
}

proc readResponse {socket client} {
    global socketList readState readCount readSize readBuffer
    global verbose activeClients readIndex xmlList readTime
    if {[eof $socket]} {
        dbg "End of file on $socket $client state=$readState($client) count=$readCount($client) size=$readSize($client)"
        dbg "XML # $readIndex($client) elapsed Time = [expr [clock seconds] - $readTime($client)]"
        set readState($client) "error"
        timeout $client
        return 0
    }
    if {[string compare $readState($client) "size"] == 0} {
        set txt ""
        set err [catch {set txt [read $socket $readSize($client)]}]
        if {$err} {
            putError "Error reading response header: $client"
            set readState($client) "error"
            timeout $client
            return 0
        }
        append readBuffer($client) $txt
        if {[string length $readBuffer($client)] < 8} {
            set readSize($client) [expr 8 - [string length $readBuffer($client)]]
            return 0
        }
        binary scan $readBuffer($client) I payloadSize
        binary scan [string range $readBuffer($client) 4 end] I headerSize
        set readBuffer($client) ""
        set readSize($client) [expr $headerSize + $payloadSize - 4]
        set readCount($client) $payloadSize
        set readState($client) "txt"
    }
    if {! [fblocked $socket]} {
        set txt ""
        set err [catch {set txt [read $socket $readSize($client)]}]
        if {$err} {
            putError "Error reading response payload: $client"
            set readState($client) "error"
            timeout $client
            return 0
        }
        append readBuffer($client) $txt
        set readSize($client) [expr $readSize($client) - [string length $txt]]
        if {$readSize($client) <= 0} {
            set readState($client) "idle"
            fileevent $socket readable ""
            catch "close $socket"
            set socketList($client) 0
            incr activeClients -1
            set loc [expr [string length $readBuffer($client)] - $readCount($client)]
            set result [string trim [string range $readBuffer($client) $loc end] "\000"]
            if ($verbose) {
                putLog "Response:"
                foreach line [split $result "<"] {
                    if {[string length $line] > 0} {putLog "<$line"}
                }
            }
            nextXML $client
            return $result
        }
    }
    return 0
}

proc nextXML {client} {
    global rate loadLimit trxActive maxActive trxCur activeClients transactions trxCustomer
    global trxNumber progress xmlList trxSent newTrx xmlCount timeOut readIndex
    global restartTrx
    if {[clock seconds] >= $timeOut} {return 0}
    set trx [expr $trxCur + $trxActive + $newTrx]
    set activeLimit $maxActive
    if {$rate >= $loadLimit} {set activeLimit [expr $maxActive / 2]}
    if {($trxCustomer + $activeClients) < $activeLimit && $trx < $transactions} {
        set idx [expr $trxNumber % $xmlCount]
        set xml "TransNum[lindex $xmlList $idx]"
        set readIndex($client) $idx
        if {[sendXML $xml $client] == 0} {
            incr trxNumber
            if {$progress > 0 && ($trxSent % $progress) == 0} {
                if {$restartTrx != 0} {
                    putLog "Sent $trxSent XMLs [expr $trxSent - $restartTrx] since restart"
                } else {
                    putLog "Sent $trxSent XMLs"
                }
            }
            return 1
        }
        return -1
    }
    return 0
}

proc checkResponse {} {
    global verbose activeClients clients trxTimeout done exitWhenWarm pollInterval warm
    global socketList readState readTime trxCur trxActive maxActive transactions newTrx
    global timeOut trxCustomer lateTransactions lateElapsed lateActive lateLoad elapsedBelow
    global loadLimit earlyLoad cacheOK cacheLate
    set now [clock seconds]
    for {set idx 0} {$idx < $clients} {incr idx} {
        if {[string compare $readState($idx) "error"] == 0} {timeout $socketList($idx)}
        if {$socketList($idx) != 0 && [string compare $readState($idx) "idle"] != 0} {
            if {$now - $readTime($idx) > $trxTimeout} {
                timeout $idx
                set timeStamp [clock format $now -format "%Y-%m-%d %H:%M:%S"]
                putLog "Timeout at $timeStamp elapsed = [expr $now - $readTime($idx)]"
            }
        }
    }
    set active [expr $trxCustomer + $activeClients]
    set sendCount [expr $maxActive - $active]
    set toGo [expr $transactions - $trxCur - $newTrx - $trxActive]
    if {$sendCount > $toGo} {set sendCount $toGo}
    if {$now >= $timeOut} {set sendCount 0}
    for {set idx 0} {$idx < $clients} {incr idx} {
        if {$sendCount <= 0} {break}
        if {$socketList($idx) == 0 && [string compare $readState($idx) "idle"] == 0} {
            nextXML $idx
            incr sendCount -1
        }
    }
    if {($trxCur >= $transactions) && $cacheOK && (! $warm)} {
        set timeStamp [clock format $now -format "%Y-%m-%d %H:%M:%S"]
        putLog "Warmed up at: $timeStamp with $trxCur transactions"
        set warm 1
    }
    if {$cacheLate} {
        if {($lateTransactions > 0 && $trxCur > $lateTransactions) || ($lateElapsed > 0 && $elapsedBelow >= 5)} {
            if {$loadLimit != $lateLoad && $maxActive != $lateActive} {
                putLog "Switching to the lower rate: Concurrent <= $lateActive, load <= $lateLoad TPS"
                set loadLimit $lateLoad
                set maxActive $lateActive
            }
        }
    }
    if {$toGo <= 0 && $activeClients <= 0 && $exitWhenWarm} {set done 1}
    if {$now >= $timeOut && $activeClients <= 0} {
        putLog "Time expired at [clock format $now -format "%Y-%m-%d %H:%M:%S"]"
        set done 1
    }
    after [expr $pollInterval * 1000] checkResponse
}

proc timeout {client} {
    global socketList readState activeClients
    set socket $socketList($client)
    if {[string compare $socket 0] != 0} {
        fileevent $socket readable ""
        catch "close $socket"
        set socketList($client) 0
        incr activeClients -1
    }
    set readState($client) "idle"
}

proc editXML {xml} {
    set start [string first "<" $xml]
    if {$start >= 0} {set xml [string range $xml $start end]}
    regsub "C20=\"\[A-Za-z\]\[A-Za-z0-9\]*\"" $xml "C20=\"WARMUP\"" xml
    return $xml
}

# load XMLs from the database
proc loadFromDatabase {database databasePort transactions} {
    global day serviceNames transactionLimit
    global tableBase verbose xmlList
    set dbList ""    
    foreach dbName $database {
        if {[catch {set db [mysqlconnect -host $dbName -port $databasePort -user hybfunc -db ATSEHYB2]}]} {
            putError "can't connect to mysql database on $dbName"
            continue
        }
        if { ! [info exists db] } {
            putError "can't connect to mysql database"
        } else {
            putLog "connected to ATSEHYB2 on $dbName"
            lappend dbList $db
        }
    }
    if {[llength $dbList] == 0} {return 0}
    set fields "fs.servicename,fs.nodeid,fs.transactiondatetime,trim(xml.inputcmd)"
    set using "using (servicename,nodeid,transactionid,clienttransactionid)"
    set where "where fs.servicename in $serviceNames and xml.inputcmd is not null"
    set order "order by transactiondatetime"
    set xmlCount 0
    if {$day == 0} {
        set yesterday [expr [clock seconds] - 86400]
        set day [string trimleft [clock format $yesterday -format "%d"] "0"]
    }
    set dbTransactions $transactions
    if {$transactionLimit > 0 && $transactionLimit < $dbTransactions} {
        set dbTransactions $transactionLimit
    }
    set startDay $day
    while {$xmlCount < $dbTransactions} {
        set dayOfMonth [format "%02d" $day]
        set table "${tableBase}$dayOfMonth as fs"
        set join "left outer join XMLREQUEST$dayOfMonth as xml"
        foreach db $dbList {
            set limit "limit [expr $dbTransactions - $xmlCount]"
            if {$verbose} {putLog "select $fields from $table $join $using $where $order $limit"}
            set rows [mysqlsel $db "select $fields from $table $join $using $where $order $limit"] 
            putLog "Read $rows rows from ${tableBase}$dayOfMonth"
            for {set rowNo 0} {$rowNo < $rows} {incr rowNo} {
                set row [mysqlnext $db]
                set servicename [lindex $row 0]
                set nodeid [lindex $row 1]
                set datetime [lindex $row 2]
                set inputcmd [string trim [lindex $row 3]]
                lappend xmlList [editXML $inputcmd]
                incr xmlCount
            }
            if {$xmlCount >= $dbTransactions} {break}
        }
        incr day -1
        if {$day <= 0} {set day 31}
        if {$day == $startDay} {break}
    }
    foreach db $dbList {mysqlclose $db}
    putLog "Loaded $xmlCount XMLs"
    return $xmlCount
}

# Load XMLs from a flat file, such as tserequest.log
proc loadFromFile {fileName transactions} {
    global transactionLimit xmlList
    if {! [file isfile $fileName]} {
        putError "Can't find file: $fileName"
        return -1
    }
    set fd [open $fileName r]
    if {! [info exists fd]} {
        putError "Can't open file: $fileName"
        return -1
    }
    set limit $transactions
    if {$transactionLimit > 0 && $transactionLimit < $limit} {
        set limit $transactionLimit
    }
    set xmlCount 0
    for {set i 0} {$i < $limit} {incr i} {
        set line [gets $fd]
        if {[eof $fd]} break;
        lappend xmlList [editXML $line]
        incr xmlCount
    }
    catch "close $fd"
    putLog "Loaded $xmlCount XMLs from $fileName"
    return $xmlCount
}

# Write XMLs to a flat file
proc writeXML {fileName} {
    global xmlList
    if {[regexp "^\[0-9\]*$" $fileName]} {return -1}
    set fd [open $fileName w]
    if {! [info exists fd]} {
        putError "Can't open file: $fileName"
        return -1
    }
    foreach xml $xmlList {
        catch {puts $fd $xml}
    }
    catch "close $fd"
    return 0
}

# start of Main
# setup and options processing
set log stdout
set args [setOptions $allOptions]
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    } elseif {! [info exists $option]} {
        set $option 0
    }
}
if {$help} {
    printUsage
    exit 0
}
set timeOut [expr [clock seconds] + $timeLimit]
if {$timeLimit == 0} {set timeOut [expr $timeOut + 86400*365*2]}
set maxActive $clients
set earlyLoad $loadLimit

if {[llength $args] > 0} {
    regexp "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$" [lindex $args 0] match host port
}
if {[string compare $host 0] == 0 || [string compare $port 0] == 0} {
    putError "host and port are required"
    printUsage
    exit 1
}
if {[string compare $logFile 0] != 0} {
    if {[file exists $logFile] && [expr [file size $logFile] > $maxLogFile]} {
        if {[file isfile ${logFile}.1]} {
            catch {file -force delete ${logFile}.1}
        }
        catch {file -force rename $logFile ${logFile}.1}
    }
    set log [open $logFile a]
    if {! [info exists log]} {
        putError "Can't open log file: $logFile"
        exit 1
    }
} else {
    set log stdout
}

# load the XMLs
set xmlCount 0
if {[string compare $xmlFile 0] != 0} {
    set xmlCount [loadFromFile $xmlFile $transactions]
} else {
    if {[loadMySql]} {
        set xmlCount [loadFromDatabase $database $databasePort $transactions]
    } else {
        putError "can't find mysql library: libmysqltcl2.51.[info sharedlibextension]"
    }
    if {$xmlCount < $transactions && [file isfile $defaultXML]} {
        set xmlCount [expr $xmlCount + [loadFromFile $defaultXML [expr $transactions - $xmlCount]]]
    }
}
if {$xmlCount <= 0} {
    putError "No XML available"
    exit 4
}
if {[info exists options(printXML)]} {
    writeXML $options(printXML)
}

set initToZero "trxSent trxActive trxLast trxCur activeClients newTrx warm restartTrx"
append initToZero " lastCpu cpuRate lastElapsed elapsedRate trxCustomer"
append initToZero " sampleCount trxList elapsedList elapsedBelow cacheReads"
append initToZero " expectCache cacheStatus cacheTime cacheCounter lastCacheTime"
append initToZero " cacheOK cacheLate lastRestartTrx"
foreach v $initToZero {set $v 0}
set idx 0
foreach v $cacheList {
    set cacheSize($v) 0
    set cacheMin($v) [lindex $cacheMinimum $idx]
    set cacheLow($v) [lindex $cacheLowLoad $idx]
    incr idx
}
set transactionsFull $transactions
set transactionsLow $lateTransactions

# start application console
set lastConsoleTime 0
set appConsoleTime 0
if {[connectAppConsole $host $consolePort] < 0} {
    set appConsoleError 1
    after [expr $retryDelay * 1000] checkAppConsole
}
vwait trxCur
if {$trxCur >= $transactions} {
    set warm 1
    if {$exitWhenWarm} {
        putLog "Already warmed up with $trxCur transactions"
        exit 0
    }
}
# wait for two samples to calculate the transaction rate
set rate 1000
vwait rate
    
set done 0
if {$skip > 0} {
    set trxNumber $skip
} else {
    set trxNumber $trxCur
}
set lastRestartTrx $trxCur
set now [clock seconds]
for {set i 0} {$i < $clients} {incr i} {
    set socketList($i) 0
    set readState($i) "idle"
    set readCount($i) 0
    set readSize($i) 0
    set readBuffer($i) ""
    set readTime($i) now
    set readIndex($i) 0
}
set activeClients 0
trap printStatus 1
trap exitSignal 2
# start sending XMLs
checkResponse
vwait done
# cleanup and exit
catch "close $console"
if {[string compare $log stdout] != 0} {close $log}
exit 0
