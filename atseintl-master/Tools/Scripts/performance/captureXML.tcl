#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# captureXML.tcl
#   Capture XML from a socket and store it in a file
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

set defaultPort 53701

package require Expect

proc printUsage {} {
    global argv0 defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\] port outputFile \[OPTIONS...\]
   or: [file tail $argv0] \[OPTIONS...\]
Options:
    --port=number       port for incoming XMLs (default=$defaultPort, 0=find a port)
    --file=filename     output file (default=stdout)
    --verbose           write a copy to stdout
    --help              display usage
    --response=filename File containing a template for the response
    --autoResponse      Change response name to match request
    --log=filename      Log file for messages
    --progress=number   report progress every n transactions
    --lniata=value      Create link to PKGE (synonym is --i)
"
}

set allOptions "port file verbose help response log progress autoResponse output lniata iata i"
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

set defaultResponse {<PricingResponse><MSG N06="N" Q0K="0" S18="RECEIVED XML"/></PricingResponse>}
set defaultResponse {<PricingResponse><MSG N06="N" Q0K="0" S18="RECEIVED XML %n"/></PricingResponse>}
set defaultResponse {<FareDisplayResponse><MSG N06="N" Q0K="0" S18="RECEIVED XML %n"/></FareDisplayResponse>}

proc bgerror {error} {
    global errorCode errorInfo
    puts stderr $error
    puts stderr $errorCode
    puts stderr $errorInfo
}

proc putLog {message} {
    global logFile outFile verbose logToFile
    if {[string compare $outFile stdout] != 0} {
        puts $message
    }
    if {$logToFile} {
        catch {puts $logFile $message}
        catch {flush $logFile}
    }
}

proc sendResponse {sock xml} {
    set payloadSize [string length $xml]
    set hdr "REST00010000"
    set hsize [string length $hdr]
    set headerSize [expr $hsize + 4]
    set header "[binary format I $payloadSize]$hdr"
    set cmd [binary format IIa$hsize $headerSize $payloadSize $hdr]
    puts -nonewline $sock "${cmd}${xml}"
    flush $sock
}

proc receiveXML {sock} {
    binary scan [read $sock 4] I headerSize
    binary scan [read $sock 4] I payloadSize 
    set header [read $sock [expr $headerSize - 4]]
    set result [read $sock $payloadSize]
    puts "headerSize = $headerSize, payloadSize = $payloadSize"
    puts "header = '$header'"
    puts "xml = $result"
    return $result
}

proc connect {address host port} {
    global clientList args verbose
    if {$verbose} {putLog "connected to: $address $host $port"}
    fconfigure $address -transalation binary
    fileevent $address readable [list readXML $address]
}

proc readXML {socket} {
    global outFile resultXML xmlCount progress verbose autoResponse
    set xml [receiveXML $socket]
    incr xmlCount
    set xml [string range $xml 8 end]
    puts $outFile $xml
    flush $outFile
    if {$verbose} {putLog "Received: $xml"}
    set response $resultXML
    if {$autoResponse} {
        if {[regexp "^<FAREDISPLAYREQUEST" $xml]} {
            regsub "^<\[A-Za-z\]+Response" $response "<FareDisplayResponse" response
            regsub "^<\[A-Za-z\]+RESPONSE" $response "<FareDisplayResponse" response
        } elseif {[regexp "^<(\[A-Za-z\]+)Request" $xml match id]} {
            regsub "^<\[A-Za-z\]+Response" $response "<${id}Response" response
            regsub "^<\[A-Za-z\]+RESPONSE" $response "<${id}Response" response
            regsub "</\[A-Za-z\]+Response" $response "</${id}Response" response
            regsub "</\[A-Za-z\]+RESPONSE" $response "</${id}Response" response
        } elseif {[regexp "^<(\[A-Za-z\]+)REQUEST" $xml match id]} {
            regsub "^<\[A-Za-z\]+Response" $response "<${id}RESPONSE" response
            regsub "^<\[A-Za-z\]+RESPONSE" $response "<${id}RESPONSE" response
            regsub "</\[A-Za-z\]+Response" $response "</${id}RESPONSE" response
            regsub "</\[A-Za-z\]+RESPONSE" $response "</${id}RESPONSE" response
        }
    }
    regsub "%n" $response $xmlCount r
    if {$verbose} {putLog "    Sent: $r"}
    sendResponse $socket "${r}\000"
    flush $socket
    close $socket
    if {$progress > 0 && [expr $xmlCount % $progress] == 0} {
        putLog "$xmlCount"
    }
}

# check for input from console
proc checkInput {} {
    global xmlCount file done
    set ch [read stdin 1]
    if {[string length $ch] == 0} {
        putLog "Read 0 characters from stdin"
        fileevent stdin readable ""
        return
    }
    puts stderr "Received $xmlCount XMLs to $file"
    if {[string match "\[qQ\]" $ch]} {
        set done 1
    }
}

proc printStatus {} {
    global xmlCount
    putLog "Received %xmlCount XMLs"
}

proc exitSignal {} {
    global xmlCount endOfFile done
    putLog "Exit signal received at $xmlCount"
    set done 1
}

# setup
set args [setOptions $allOptions]
if {! [info exists options(lniata)]} {
    if {[info exists options(iata)]} {
        set options(lniata) $options(iata)
    } elseif {[info exists options(i)]} {
        set options(lniata) $options(i)
    }
}
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
if {[llength $args] > 0} {
    foreach arg $args {
        if {[regexp "^\[0-9\]+$" $arg]} {
            set port $arg
        } else {
            set file $arg
        }
    }
}
if {$port == 0 && ! [info exists options(port)]} {
    set port $defaultPort
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
if {[string compare $file 0] == 0} {set file $output}
if {[string compare $file 0] == 0} {
    set outFile stdout
    set file stdout
} else {
    set outFile [open $file a]
    if {! [info exists outFile]} {
        puts stderr "Can't open output file: $file"
        exit 1
    }
}
if {[string compare $response 0] == 0} {
    set resultXML $defaultResponse
    if {! [info exists options(autoResponse)]} {set autoResponse 1}
} else {
    set fd [open $response r]
    if  {! [info exists fd]} {
        puts stderr "Can't open response file: $response"
        exit 1
    }
    set resultXML [gets $fd]
    close $fd
}

set xmlCount 0
set done 0
fileevent stdin readable checkInput
set listen [socket -server connect $port]
set config [fconfigure $listen]
set portName $port
if {[set idx [lsearch -exact $config "-sockname"]] >= 0} {
    set sockname [lindex $config [expr $idx + 1]]
    if {[llength $sockname] >= 3} {set portName [lindex $sockname 2]}
} 
puts "Listening on port: $portName"
if {[string compare $lniata 0] != 0} {
    set lniata [string toupper $lniata]
    if {! [regexp {[0-9A-F]+} $lniata]} {
        puts stderr "Invalid LNIATA: $lniata"
    } else {
        set name "/opt/IORs/tseserver${lniata}.ior"
        set fd [open $name w]
        if {[info exists fd]} {
            puts -nonewline $fd "[exec hostname]:$portName"
            close $fd
            set lniataFile $name
        } else {
            puts stderr "Can't create IOR file: $name"
        }
    }
}

trap printStatus 1
trap exitSignal 2

vwait done
putLog "Done"
putLog "$xmlCount XMLs stored"
if {[string compare $outFile stdout] != 0} {close $outFile}
if {$logToFile} {close $logFile}
if {[info exists lniataFile]} {file delete $lniataFile}
exit 0

