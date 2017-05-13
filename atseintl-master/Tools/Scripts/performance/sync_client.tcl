#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# sync_client.tcl
#   Send transactions to two tseservers
#   Transactions are stored in a file one per line or sent via a socket
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set clientTimeout 1000
set prereadLimit 1000
set useDebug 0
set logLimit 10000
set logCount 0

package require Expect

proc printUsage {} {
    global argv0
    puts stderr "
Usage(client): [file tail $argv0] \[OPTIONS...\] <tseserver> \[OPTIONS...\]
Usage(server): [file tail $argv0] \[OPTIONS...\] <tseserver> <tseserver> <xmlfile> \[OPTIONS...\]
    <tseserver>         host:port string or name of file containing host:port for tseserver
    <xmlfile>           base name of transactions file
Options:
    --verbose           display inputs, outputs and messages
    --help              display usage
    --start=number      start processing requests at request number
    --numreqs=number    number of requests to process
    --log=logfile       log outputs to logfile in addition to stdout
Server Options:
    --server=port       send transactions to clients
    --clients=number    automatically start n clients
    --clientVerbose=0/1 set verbosity for clients
    --window            start clients in a window
    --progress=number   report progress every n transactions
    --delete            delete old XMLs as they are consumed
"
}

set allOptions "verbose help start numreqs log server clients clientVerbose progress window delete"
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

proc dbg {string} {
    global useDebug
    if {$useDebug} {puts $string}
}

proc putLog {message} {
    global logFile logToFile
    global logCount logLimit
    incr logCount
    if {$logCount >= $logLimit} {
        if {$logCount == $logLimit} {
            puts "Log message limit at $logCount"
            puts stderr "Log message limit at $logCount"
            if {$logToFile} {
                puts $logFile "Log message limit at $logCount"
                flush $logFile
            }
        }
        return
    }
    puts $message
    if {$logToFile} {
        puts $logFile $message
        flush $logFile
    }
}

proc putErr {message} {
    global logFile logToFile
    global logCount logLimit
    incr logCount
    if {$logCount >= $logLimit} {
        if {$logCount == $logLimit} {
            incr logCount -1
            puts stderr $message
            puts stderr "Log message limit"
            putLog $message
        }
        return
    }
    puts stderr $message
    if {$logToFile} {
        puts $logFile $message
        flush $logFile
    }
}

proc bgerror {error} {
    global errorCode errorInfo
    putErr $error
    putErr $errorCode
    putErr $errorInfo
}

proc sendTrx {host port trx} {
    global verbose
    #      for testing
#    if {[string first 407 $host] >= 0} {
#        sleep 5
#    } else {
#        sleep 2
#    }
#    puts "sendTrx $host:$port"
#    return "send: $trx"
    #
    set err [catch {set sock [socket $host $port]}]
    if {$err || (! [info exists sock])} {
        puts stderr "Can't open tseserver socket to: $host:$port"
        exit 2
    }
    fconfigure $sock -translation binary
    set payloadSize [string length $trx]
    set hdr "RESP00010000"
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]
    puts -nonewline $sock "${cmd}${trx}"
    flush $sock
    if {$verbose} {putLog "Waiting for response"}
    binary scan [read $sock 4] I headerSize
    binary scan [read $sock 4] I payloadSize 
    set header [read $sock [expr $headerSize - 4]]
    set result [read $sock $payloadSize]
    close $sock
    return $result
}

proc loadBuffer {} {
    global prereadLimit xmlBuffer bufferIdx bufferSize xmlPosition inFile eofTag
    set bufferSize 0
    set bufferIdx 0
    if {[eof $inFile]} {return 0}
    for {set i 0} {$i < $prereadLimit} {incr i} {
        if {[catch [list set line [gets $inFile]]]} {
            break;
        }
        if {[eof $inFile]} {break}
        set xmlBuffer($i) $line
        incr bufferSize
    }
    set xmlPosition [tell $inFile]
    return $bufferSize
}

proc getTrx {} {
    global useSocket trxNumber eofTag endOfFile fileReady dataReady inFile
    global newFile newerFile fileTrx inFileName
    global xmlBuffer bufferIdx bufferSize xmlPosition
    set dataReady 0
    if {$endOfFile} {return $eofTag}
    if {! [info exists inFile]} {return $eofTag}
    if {$useSocket} {
        dbg "Get transaction $trxNumber from socket $inFile"
        if {$trxNumber < 10} {
            catch {puts $inFile "Req $trxNumber Pid [pid]"}
            dbg "Send 'Req $trxNumber Pid [pid]' to $inFile"
        } else {
            catch {puts $inFile "Req $trxNumber"}
        }
        catch {flush $inFile}
        if {[catch [list set line [gets $inFile]]]} {
            set line $eofTag
        }
        if {[eof $inFile]} {set line $eofTag}
    } else {
        dbg "Get transaction $trxNumber from file $inFile"
        if {! $fileReady} {
            openXML
            if {! $fileReady} {return $eofTag}
        }
        if {$bufferIdx >= $bufferSize} {loadBuffer}
        if {$bufferIdx < $bufferSize} {
            set i $bufferIdx
            incr bufferIdx
            set dataReady 1
            return $xmlBuffer($i)
        }
        if {[catch {set line [gets $inFile]}]} {
            putLog "Error reading xml file: $inFileName"
            nextFile
            if {! $fileReady} {return $eofTag}
            if {[catch {set line [gets $inFile]}]} {return $eofTag}
        }
        if {[eof $inFile]} {
            nextFile
            if {! $fileReady} {return $eofTag}
            if {[catch {set line [gets $inFile]}]} {return $eofTag}
            if {[eof $inFile]} {return $eofTag}
        }
    }
    dbg "getTrx: returning [string length $line] chars"
    set dataReady 1 
    return $line
}

#  server functions
proc connect0 {address host port} {
    global clientList args
    putLog "connected to: $address $host $port"
    lappend clientList $address
    set groupOfSocket($address) 0
    set processOfSocket($address) 0
    fileevent $address readable [list sendFirstLine $address 0]
}

proc connect1 {address host port} {
    global clientList args
    putLog "connected to: $address $host $port"
    lappend clientList $address
    set groupOfSocket($address) 1
    set processOfSocket($address) 0
    fileevent $address readable [list sendFirstLine $address 1]
}

proc sendFirstLine {socket group} {
    global verbose trxNumber ready
    set line [gets $socket]
    if {$verbose} {putLog "$socket: $line at [expr $trxNumber + 1]"}
    if {! [info exists SocketProcess($socket)]} {
        if {[regexp "Pid (\[0-9\]+)$" $line match id]} {
            set socketOfProcess($id) $socket
            set processOfSocketl($socket) $id
            putLog "Socket $socket connected to process $id"
        }
    }
    fileevent $socket readable [list sendLine $socket $group]
    lappend ready($group) $socket
    checkReady
}

proc sendLine {socket group} {
    global verbose trxNumber ready
    if {[eof $socket]} {
        putLog "socket eof on $socket"
        close $socket
        return
    }
    if {[gets $socket line] < 0} {
        putLog "gets < 0 on $socket"
        close $socket
        return
    }
    if {$verbose} {putLog "$socket: $line at [expr $trxNumber + 1]"}
    lappend ready($group) $socket
    checkReady
}

proc checkReady {} {
    global trxNumber nextLine inFile endOfFile verbose clientList
    global progress ready dataReady numreqs errorCode fileTrx
    global newerFile newFile
    dbg "checkReady for '$ready(0)' and '$ready(1)'"
    if {$endOfFile} {
        foreach socket [concat $ready(0) $ready(1)] {
            catch {puts $socket $eofTag}
            catch {flush $socket}
            catch {close $socket}
            set client [lsearch $clientList $socket]
            if {$client >= 0} {set clientList [lreplace $clientList $client $client]}
            putLog "Client $socket is done"
        }
        set ready(0) ""
        set ready(1) ""
        if {$verbose} {putLog "$clientList"}
        return
    }
    dbg "dataReady: $dataReady"
    while {[llength $ready(0)] > 0 && [llength $ready(1)] > 0} {
        if {! $dataReady} {
            set nextLine [getTrx]
            if {! $dataReady} {return}
        }
        for {set i 0} {$i < 2} {incr i} {
            set socket [lindex $ready($i) 0]
            set ready($i) [lrange $ready($i) 1 end]
            if {[catch {puts $socket $nextLine}]} {
                putErr "Socket error on $socket $errorCode"
                restart $socket
            }
            if {[catch [list flush $socket]]} {
                putErr "Flush error on $socket $errorCode"
                restart $socket
            }
        }
        incr trxNumber
        incr fileTrx
        if {$fileTrx > 10000 && (($fileTrx % 1000) == 0)} {
            if {[file isfile $newerFile] && [file isfile $newFile]} {
                nextFile
            }
        }
        if {$progress && [expr $trxNumber % $progress] == 0} {putLog "Sent $trxNumber"}
        set nextLine [getTrx]
        incr numreqs -1
        if {$numreqs <= 0} {set endOfFile 1}
        update
    }
}

proc nextFileName {fileName} {
    global argno args
    if {$argno < [llength $args]} {
        set name [lindex $args $argno]
        incr argno
        return $name
    }
    if {! [regexp "^(.*)\\.(\[0-9\]+)$" $fileName match prefix index]} {
        set index 0
        set prefix "$fileName"
    }
    incr index
    return "${prefix}.${index}"
}

# next xml file
proc nextFile {} {
    global inFile inFileName fileReady delete
    global oldFile olderFile newFile newerFile fileTrx
    if {[info exists inFile]} {
        catch "close $inFile"
        unset inFile
    }
    if {$delete && [string length $olderFile] > 0} {
        catch [exec rm $olderFile]
    }
    set olderFile $oldFile
    set oldFile $inFileName
    set inFileName $newFile
    set newFile $newerFile
    set newerFile [nextFileName $newFile]
    openXML
    if {! $fileReady} {
        putLog "Waiting for $inFileName"
        after 60000 checkXML
    }
    set fileTrx 0
}

proc openXML {} {
    global inFile inFileName fileReady verbose
    if {[file isfile $inFileName]} {
        set inFile [open $inFileName r]
        if {[info exists inFile]} {
            set fileReady 1
            putLog "Opened xml file: $inFileName"
        } else {
            putErr "Open failed on $inFileName"
            set fileReady 0
        }
    } else {
        set fileReady 0
    }
}

# check for more XML
proc checkXML {} {
    global fileReady nextLine
    if {! $fileReady} {openXML}
    if {$fileReady} {
        set nextLine [getTrx]
        checkReady
    } else {
        after 60000 checkXML
    }
}

proc restart {socket} {
    global processOfSocket groupOfProcess args endOfFile
    global ProcessClient ClientTarget clientList processList processOutput
    global hostName serverPort v target
    set process processOfSocket($socket)
    set group groupOfProcess($process)
    logErr "Shutting down socket $socket for group $group to $args($group)
    closeClient $socket
    if {$endOfFile} {return}
    logErr "Restarting socket $socket for group $group to $args($group)
    if {$window} {
        set process [spawn xterm -geometry 60x20 -e $argv0 $target($group) $hostName:$serverPort($t) --verbose=$v --window]
    } else {
        set process [spawn $argv0 $target($t) $hostName:$serverPort($group) --verbose=$v]
    }
    set pFile [exp_open]
    set processOutput($process) $pFile
    set groupOfProcess($process) $group
    fconfigure $pFile -blocking 0
    fileevent $pFile readable [list readProcess $pFile $process]
    putLog "Started client process: $process"
    lappend processList $process
}

proc closeClient {socket} {
    global processOfSocket ProcessClient ClientTarget clientList processList processOutput
    global groupOfProcess
    catch "close $socket"
    if {[info exists processOfSocket($socket)]} {
        set process processOfSocket($socket)
        catch {close $processOutput($process)}
        unset processOutput($process)
        catch {exec kill -s 9 $process}
        if {[info exists groupOfProcess($process)]} unset groupOfProcess($process)
        unset processOfSocket($socket)
        set idx [lsearch -exact $processList $process]
        if {$idx >= 0} {set processList [lreplace $processList $idx $idx]}
    }
    set idx [lsearch -exact $clientList $socket]
    if {$idx >= 0} {set clientList [lreplace $clientList $idx $idx]}
}

# check for input from console
proc checkInput {} {
    global trxNumber endOfFile inFileName ready fileTrx
    set ch [read stdin 1]
    if {[string length $ch] == 0} {
        putLog "Read 0 characters from stdin"
        fileevent stdin readable ""
        return
    }
    puts stderr "Sending $trxNumber from $inFileName #$fileTrx"
    puts stderr "Ready Lists: \($ready(0)\) \($ready(1)\)"
    if {[string match "\[qQ\]" $ch]} {
        set endOfFile 1
        checkReady
    }
}

proc printStatus {} {
    global trxNumber inFileName ready fileTrx
    putLog "Sending $trxNumber from $inFileName #$fileTrx"
    puts stderr "Ready Lists: \($ready(0)\) \($ready(1)\)"
}

proc exitSignal {} {
    global trxNumber endOfFile
    putLog "Exit signal received at $trxNumber"
    set endOfFile 1
    checkReady
}

# read stdout and stderr from clients
proc readProcess {socket id} {
    global verbose readProcCount
    if {[eof $socket]} {
        close $socket
        return
    }
    if {[catch [list gets $socket line] result]} {
        close $socket
        return
    }
    if {$result <= 0} {
        putLog "$result returned from gets on $socket id:$id"
        close $socket
    }
    if {$readProcCount < 100} {
        putLog "$id ([string length $line]): $line"
        incr readProcCount
    } else {
        close $socket
    }
    if {$verbose && [string length $line] > 0} {putLog "$id: $line"}
}

#  if a client process fails to exit
proc eofTimeout {} {
    global clientList inFile logFile logToFile
    putLog "Timeout at eof"
    foreach client $clientList {
        catch {close $client}
        if {[info exists processOfSocket($client)]} {
            set id $processOfSocket($client)
            catch {exec kill -s 9 $id}
            putLog "Killed process $id"
        }
    }
    catch {close $inFile]}
    if {$logToFile} {close $logFile}
    exit 1
}
        
# setup
set eofTag "*"
set errorTag "*ERROR*"
set endOfFile 0
set bufferIdx 0
set bufferSize 0
set args [setOptions $allOptions]
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    } else {
        set $option 0
    }
}
if {$help} {
    printUsage
    exit 0
}
if {[llength $args] < 2} {
    printUsage
    exit 1
}

set isServer [expr [string compare $server 0] != 0]
set isClient [expr ! $isServer]
if {$isServer && [llength $args] < 3} {
    putErr "Wrong number of arguments for server"
    printUsage
    exit 1
}

if {[string compare $log 0] != 0} {
    set logFile [open $log a]
    if {! [info exists logFile]} {
        puts stderr "Can't open log file: $log"
        exit 1
    }
    set logToFile 1
} else {
    set logToFile 0
}

set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$" 
if {! [regexp $hostPortMatch [lindex $args 0] match host port]} {
    set fd [open [lindex $args 0]
    if {! [info exists fd]} {
        puts stderr "host:port or ior file not found: [lindex $args 0]"
        exit 1
    }
    set line ""
    set line [gets $fd]
    close $fd
    if { ! [regexp $hostPortMatch $line match host port]} {
        puts stderr "Invalid ior file: [lindex $args 0]"
        exit 1
    }
}
if {$isServer && ! [regexp $hostPortMatch [lindex $args 1] match host port]} {
    puts "Invalid host:port for 2nd server"
    printUsage
    exit 1
}
    
set useSocket 0
set argno [expr $isServer + 1]
set inFileName [lindex $args $argno]
if {[regexp $hostPortMatch $inFileName match rcvHost rcvPort]} {
    set useSocket 1
    set inFile [socket $rcvHost $rcvPort]
    set fileReady 1
    dbg "Opened input socket: $inFile"
} else {
    openXML
}
incr argno
set olderFile ""
set oldFile ""
set fileTrx 0
if {! $useSocket} {
    set newFile [nextFileName $inFileName]
    set newerFile [nextFileName $newFile]
}

# skip to the first transaction
set trxNumber 0
if {$start > 1} {
    for {set i 1} {$i < $start} {incr i} {
        incr trxNumber
        getTrx
    }
}

# main
if {$numreqs == 0} {set numreqs [expr 32767 * 65536]}
trap printStatus 1
trap exitSignal 2
if {$isServer} {
    if {[llength $server] >= 2} {
        for {set i 0} {$i < 2} {incr i} {set serverPort($i) [lindex $server $i]}
    } elseif {$server > 1} {
        set serverPort(0) $server
        set serverPort(1) [expr $server + 1]
    } else { 
        set serverPort(0) 53711
        set serverPort(1) 53712
    }
    set clientList ""
    set processList ""
    set readProcCount 0
    set listen(0) [socket -server connect0 $serverPort(0)]
    set listen(1) [socket -server connect1 $serverPort(1)]
    set nextLine [getTrx]
    putLog "Server ready"
    if {$clients > 0 && $clients < 60} {
        if {[info exists options(clientVerbose)]} {
            set v $clientVerbose
        } else {
            set v $verbose
        }
        set hostName [exec hostname]
        for {set i 0} {$i < 2} {incr i} {
            set target($i) [lindex $args $i]
            set ready($i) ""
        }
        for {set i 0} {$i < $clients} {incr i} {
            for {set t 0} {$t < 2} {incr t} {
                if {$window} {
                    set process [spawn xterm -geometry 60x20 -e $argv0 $target($t) $hostName:$serverPort($t) --verbose=$v --window]
                } else {
                    set process [spawn $argv0 $target($t) $hostName:$serverPort($t) --verbose=$v]
                }
                set pFile [exp_open]
                set processOutput($process) $pFile
                set groupOfProcess($process) $t
                fconfigure $pFile -blocking 0
                fileevent $pFile readable [list readProcess $pFile $process]
                putLog "Started client process: $process"
                lappend processList $process
            }
        }
    }
    fileevent stdin readable checkInput
    vwait endOfFile
    putLog "End of file"
    after [expr 500*1000] eofTimeout
    while {[llength $clientList] > 0} {
        if {$verbose} {putLog "[llength $clientList] clients"}
        vwait clientList
    }
    if {[info exists inFile]} {close $inFile}
    if {$logToFile} {close $logFile}
    exit 0
}

for {set i 0} {$i < $numreqs} {incr i} {
    incr trxNumber
    set trx [getTrx]
    if {[string compare $trx $eofTag] == 0} {break}
    set loc [string first "<" $trx]
    if {$loc >= 0} {set trx [string range $trx $loc end]}
    if {$verbose} {putLog "Request: $trx"}
    if {$window} {putLog "Sending request #$trxNumber"}
    set start [clock clicks -milliseconds]
    set result [sendTrx $host $port TransNum${trx}]
    set end [clock clicks -milliseconds]
    if {$verbose} {
        set r [split $result "<"]
        putLog "Response:"
        foreach item $r {
            if {[string length $item] > 0} {putLog "<$item"}
        }
    }
    if {$window} {
        putLog "Received #$trxNumber in [expr ($end - $start)/1000.0] seconds"
    }
}

if {[info exists inFile]} {close $inFile}
if {$logToFile} {close $logFile}

