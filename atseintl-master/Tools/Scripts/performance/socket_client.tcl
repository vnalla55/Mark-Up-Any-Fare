#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# socket_client.tcl
#   Send transactions to a tseserver socket
#   Transactions are stored in a file one per line or sent via a socket
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set clientTimeout 1000
set retryDelay [expr 60*1000]
set prereadLimit 1000
set errorLimit 1000
set messageLimit 10000

package require Expect

proc printUsage {} {
    global argv0
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\] <hpfile> <xmlfile> \[OPTIONS...\] 
    <hpfile>            host:port string or name of file containing host:port for tseserver
    <xmlfile>           host:port string for transaction server or name of transactions file
Options:
    --verbose           display inputs, outputs and messages
    --help              display usage
    --start=number      start processing requests at request number
    --numreqs=number    number of requests to process
    --log=logfile       log outputs to logfile in addition to stdout
    --all               skip the first time, then repeat the entire file
Server Options:
    --server=port       send transactions to clients
    --clients=number    automatically start n clients
    --clientVerbose=0/1 set verbosity for clients
    --silent            do not start clients in a window
    --progress=number   report progress every n transactions
    --repeat=number     repeat n times
"
}

set allOptions "verbose help start numreqs log server clients clientVerbose progress repeat all silent"
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

proc putLog {message} {
    global logFile messageLimit messageCount verbose
    incr messageCount
    if {($messageCount > $messageLimit) && (! $verbose)} {return}
    puts $message
    if {[string compare $logFile stdout] != 0} {
        catch {puts $logFile $message}
        catch {flush $logFile}
    }
}

proc putErr {message} {
    global logFile messageLimit messageCount verbose
    incr messageCount
    if {$messageCount > $messageLimit && ! $verbose} {return}
    puts stderr $message
    if {[string compare $logFile stdout] != 0} {
        catch {puts $logFile $message]}
        catch {flush $logFile}
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
#    sleep 1
#    return "send: $trx"
    #
    set sock [socket $host $port]
    if {! [info exists sock]} {
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
    set err1 [catch {puts -nonewline $sock "${cmd}${trx}"}]
    set err2 [catch {flush $sock}]
    if {$err1 != 0 || $err2 != 0} {return ""}
    if {$verbose} {putLog "Waiting for response"}
    binary scan [read $sock 4] I headerSize
    binary scan [read $sock 4] I payloadSize 
    set header [read $sock [expr $headerSize - 4]]
    set result [read $sock $payloadSize]
    close $sock
    return $result
}

proc loadBuffer {fd} {
    global prereadLimit xmlBuffer bufferIdx bufferSize xmlPosition eofTag
    set bufferSize 0
    set bufferIdx 0
    if {[eof $fd]} {return 0}
    for {set i 0} {$i < $prereadLimit} {incr i} {
        if {[catch [list set line [gets $fd]]]} {
            break;
        }
        if {[eof $fd]} {set line $eofTag}
        set xmlBuffer($i) $line
        incr bufferSize
    }
    set xmlPosition [tell $fd]
    return $bufferSize
}

proc getTrx {fd} {
    global useSocket trxNumber eofTag repeat fileStart endOfFile
    global args argno inFile errorTag
    global xmlBuffer bufferIdx bufferSize xmlPosition
    if {$endOfFile} {return $eofTag}
    if {$useSocket} {
        catch {puts $fd "Req $trxNumber Pid [pid]"}
        catch {flush $fd}
        if {[catch [list set line [gets $fd]]]} {
            set line $eofTag
        }
    } else {
        if {$bufferIdx >= $bufferSize} {loadBuffer $fd}
        if {$bufferIdx < $bufferSize} {
            set i $bufferIdx
            incr bufferIdx
            return $xmlBuffer($i)
        }
        if {$bufferSize <= 0} {
            if {! [eof $fd]} {
                putLog "Error reading XML file"
                return $errorTag
            }
            set line $eofTag
            incr argno
            if {$argno < [llength $args]} {
                set inTmp [open [lindex $args $argno]]
                if {[info exists inTmp]} {
                    close $inFile
                    set inFile $inTmp
                    set fd $inFile
                    loadBuffer $fd
                } else {
                    puts stderr "Can't open [lindex $args $argno]"
                }
            }
            if {[string compare $line $eofTag] == 0 && $repeat > 1} {
                seek $fd $fileStart
                loadBuffer $fd
                if {$bufferSize <= 0} {
                    set line $eofTag
                } else {
                    set line $xmlBuffer($bufferIdx)
                    incr bufferIdx
                }
                incr repeat -1
                putLog "$repeat cycles remain"
            }
        }
    }
    return $line
}

#  server functions
proc connect {address host port} {
    global clientList
    putLog "connected to: $address $host $port"
    lappend clientList $address
    fileevent $address readable [list sendFirstLine $address]
}

proc sendFirstLine {socket} {
    global verbose trxNumber
    set line [gets $socket]
    if {$verbose} {putLog "$socket: $line at [expr $trxNumber + 1]"}
    if {! [info exists SocketProcess($socket)]} {
        if {[regexp "Pid (\[0-9\]+)$" $line match id]} {
            set SocketProcess($socket) $id
            set ProcessSocket($id) $socket
            putLog "Socket $socket connected to process $id"
        }
    }
    fileevent $socket readable [list sendLine $socket]    
    sendNextLine $socket
}

proc sendLine {socket} {
    global verbose trxNumber
    set line [gets $socket]
    if {$verbose} {putLog "$socket: $line at [expr $trxNumber + 1]"}
    sendNextLine $socket
}

proc sendNextLine {socket} {
    global trxNumber nextLine inFile endOfFile verbose eofTag clientList
    global progress numreqs reqCount repeat fileStart errorTag waitList
    global errorCode retryDelay errorCount errorLimit
    if {[string compare $nextLine $errorTag] == 0} {
        if {[llength $waitList] == 0} {after $retryDelay rereadFile}
        lappend waitList $socket
        return
    }
    if {[catch {puts $socket $nextLine}]} {
        putErr "Socket error on $socket $errorCode"
        incr errorCount
        if {$errorCount > $errorLimit} {
            putErr "Error limit: $errorCount"
            exit 9
        }
        # kill process and start a new one
        return
    }
    if {[catch [list flush $socket]]} {
        putErr "Flush error on $socket $errorCode"
        incr errorCount
        if {$errorCount > $errorLimit} {
            putErr "Error limit: $errorCount"
            exit 9
        }
        # kill process and start a new one
        return
    }
    incr trxNumber
    if {[string compare $nextLine $eofTag] == 0} {
        set endOfFile 1
        puts $socket $eofTag
        close $socket
        set client [lsearch $clientList $socket]
        if {$client >= 0} {set clientList [lreplace $clientList $client $client]}
        putLog "Client $socket is done"
        if {$verbose} {putLog "$clientList"}
        return
    }
    incr reqCount -1
    if {$reqCount <= 0} {
        if {$repeat > 1} {
            seek $inFile $fileStart
            incr repeat -1
            set reqCount $numreqs
            putLog "$repeat cycles remain"
        } else {   
            set nextLine $eofTag
        }
    } else {
        set nextLine [getTrx $inFile]
    }
    if {$progress && [expr $trxNumber % $progress] == 0} {
        if {! $endOfFile} {putLog "Sent $trxNumber"}
    }
}

proc rereadFile {} {
    global waitList retryDelay nextLine inFile inClosed inPosition inFileName
    global rereadCount xmlPosition
    incr rereadCount
    if {[expr $rereadCount % 30] == 1} {putLog "Retry #$rereadCount reading $inFileName"}
    if {[string compare $nextLine $errorTag] != 0} {
        set wait $waitList
        set waitList ""
        foreach socket $waitList {sendNextLine $socket}
        return
    }
    if {$inClosed} {
        if {[info exists inFile]} {unset inFile}
        catch {set inFile [open $inFileName r]}
        if {! [info exists inFile]} {
            after $retryDelay rereadFile
            return
        }
        if {catch {seek $inFile $inPosition}} {
            catch {close $inFile}
            after $retryDelay rereadFile
            return
        }
        set inClosed 0
        putLog "Reopened $inFileName at [clock format [clock seconds] -format "%Y-%m-%d %H:%M:%s"]"
    }
    set nextLine [getTrx $inFile]
    if {[string compare $nextLine $errorTag] == 0} {
        after $retryDelay rereadFile
        if {! $inClosed && ($rereadCount % 15) == 0} {
            if {catch {set inPosition [tell $inFile]}} {set inPosition $xmlPosition}
            if {$inPosition < 0} return
            catch {close $inFile}
            set inClosed 1
            putLog "Closed $inFileName at [clock format [clock seconds] -format "%Y-%m-%d %H:%M:%s"]"
        }
    } else {
        set wait $waitList
        set waitList ""
        foreach socket $waitList {
            sendNextLine $socket
        }
    }
}

# check for input from console
proc checkInput {} {
    global trxNumber repeat endOfFile
    return
    set ch [read stdin 1]
    if {[string length $ch] == 0} {
        puts "Read 0 characters from stdin"
        fileevent stdin readable ""
        return
    }
    puts stderr "Sending $trxNumber repeat = $repeat"
    if {[string match "\[qQ\]" $ch]} {
        set endOfFile 1
        set repeat 0
    }
}

proc printStatus {} {
    global trxNumber repeat
    putLog "Sending $trxNumber repeat = $repeat"
}

proc exitSignal {} {
    global trxNumber endOfFile
    putLog "Exit signal received at $trxNumber"
    set endOfFile 1
}

# read stdout and stderr from clients
proc readProcess {socket id} {
    global verbose
    if {[eof $socket]} {
        close $socket
        return
    }
    if {[catch [list gets $socket] line]} {
        close $socket
        return
    }
    if {$verbose && [string length $line] > 0} {putLog "$id: $line"}
}

#  if a client process fails to exit
proc eofTimeout {} {
    global clientList inFile logFile
    putLog "Timeout at eof"
    foreach client $clientList {
        close $client
        if {[info exists SocketProcess($client)]} {
            set id $SocketProcess($client)
            catch {exec kill -s 9 $id}
            putLog "Killed process $id"
        }
    }
    close $inFile
    if {[string compare $logFile stdout] != 0} {close $logFile}
    exit 1
}
        
# setup
set eofTag "*"
set errorTag "*ERROR*"
set endOfFile 0
set bufferIdx 0
set bufferSize 0
set xmlPosition 0
set errorCount 0
set messageCount 0
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

if {[string compare $log 0] != 0} {
    set logFile [open $log a]
    if {! [info exists logFile]} {
        puts stderr "Can't open log file: $log"
        exit 1
    }
} else {
    set logFile stdout
}

set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$" 
if {! [regexp $hostPortMatch [lindex $args 0] match host port]} {
    set fd [open [lindex $args 0]]
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
    
set useSocket 0
set argno 1
if {[regexp $hostPortMatch [lindex $args $argno] match rcvHost rcvPort]} {
    set useSocket 1
    set inFile [socket $rcvHost $rcvPort]
} else {
    set inFile [open [lindex $args $argno]]
}
if {! [info exists inFile]} {
    puts stderr "Can't open [lindex $args $argno]"
    exit 1
}

# skip to the first transaction
set trxNumber 0
set fileStart 0
if {$start > 1} {
    for {set i 1} {$i < $start} {incr i} {
        incr trxNumber
        getTrx $inFile
    }
    if {! $all} {set fileStart [tell $inFile]}
}

# main
if {$numreqs == 0} {set numreqs [expr 32767 * 65536]}
set reqCount $numreqs
trap printStatus 1
trap exitSignal 2

if {$server} {
    set serverPort $server
    if {$serverPort < 2} {set serverPort 53711}
    set nextLine [getTrx $inFile]
    set clientList ""
    set clientProcess ""
    set waitList ""
    set listen [socket -server connect $serverPort]
    putLog "Server ready"
    putLog "Repeat: $repeat"
    if {$clients > 0 && $clients < 60} {
        if {[info exists options(clientVerbose)]} {
            set v $clientVerbose
        } else {
            set v $verbose
        }
        for {set i 0} {$i < $clients} {incr i} {
            set hostName [exec hostname]
            set target [lindex $args 0]
            if {$silent} {
                set process [spawn $argv0 $target $hostName:$serverPort --verbose=$v --silent]
                set pFile [exp_open]
                set ProcessOutput($process) $pFile
                fconfigure $pFile -blocking 0
                fileevent $pFile readable [list readProcess $pFile $process]
            } else {
                set process [spawn xterm -geometry 60x20 -e $argv0 $target $hostName:$serverPort --verbose=$v]
            }
            putLog "Started client process: $process"
            lappend clientProcess $process
        }
    }
#    fileevent stdin readable checkInput
    vwait endOfFile
    putLog "End of file"
    after [expr 500*1000] eofTimeout
    while {[llength $clientList] > 0} {
        if {$verbose} {putLog "[llength $clientList] clients"}
        vwait clientList
    }
    close $inFile
    if {[string compare $logFile stdout] != 0} {close $logFile}
    exit 0
}

for {set reqNo 0} {$reqNo < $numreqs} {incr reqNo} {
    incr trxNumber
    set trx [getTrx $inFile]
    if {[string compare $trx $eofTag] == 0} {break}
    set loc [string first "<" $trx]
    if {$loc >= 0} {set trx [string range $trx $loc end]}
    if {$verbose} {putLog "Request: $trx"}
    if {! $silent} {putLog "Sending request #$trxNumber"}
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
    if {! $silent} {
        putLog "Received #$trxNumber in [expr ($end - $start)/1000.0] seconds"
    }
    flush stdout
}

close $inFile
if {[string compare $logFile stdout] != 0} {close $logFile}
sleep 2

