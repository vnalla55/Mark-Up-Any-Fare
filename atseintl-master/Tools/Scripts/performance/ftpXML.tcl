#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# ftpXML.tcl
#   Retrieve xml from a tserequest log file
# Author: Jim Sinnamon
# Copyright: Sabre 2008
# ------------------------------------------------------------------

package require Expect

set host piclp004
set delay 7200
set startFile "tserequest.log.1"
set logFile "ftp.log"
set group pricing
set remoteFile "tserequest.log.1"
set retryLimit 3
set retryDelay 600
set help 0
set maxFiles 6

set PricingServers "piclp004 piclp005 piclp009 piclp010 piclp011 piclp012 piclp013 piclp161 piclp162 piclp163 piclp164 piclp165 piclp166 piclp167 piclp215 piclp216 piclp217 piclp254 piclp255 piclp256 piclp257 piclp258 piclp259 pimlp001 pimlp002 pimlp003 pimlp004 pimlp005 pimlp006 pimlp007 pimlp043 pimlp044"

set timeout 60
set retryCount 0
proc printUsage {} {
    global argv0 host delay startFile logFile
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
Options:
    --host=name         starting host ($host)
    --delay=seconds     delay between transfers ($delay)
    --startFile=name    name of the first output file ($startFile)
    --remoteFile=name   name of the file to fetch from the remote server
    --logFile=filename  name of the log file ($logFile)
    --altFile=name      alternative file if removeFile is not found
"
}

set allOptions "host delay startFile remoteFile altFile logFile help"
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

proc putLog {message} {
    global log
    puts $message
    if {[info exists log]} {puts $log $message}
}

proc putErr {message} {
    global log
    puts stderr $message
    if {[info exists log]} {puts $log $message}
}

proc checkInput {} {
    global startFile host done
    set ch [read stdin 1]
    if {[string length $ch] == 0} {
        puts "Read 0 characters from stdin"
        fileevent stdin readable ""
        return
    }
    putLog "Next file is $startFile from $host"
    if {[string match "\[qQ\]" $ch]} {
        set done 1
    }
}

proc printStatus {} {
    global startFile host
    putLog "Next file is $startFile from $host"
}

proc exitSignal {} {
    global startFile host done
    putLog "Exit signal received"
    putLog "Next file is $startFile from $host"
    set done 1
}   

proc bgerror {error} {
    global errorCode errorInfo
    putErr $error
    putErr $errorCode
    putErr $errorInfo
}

proc nextFileName {fileName} {
    if {! [regexp "^(.*)\\.(\[0-9\]+)$" $fileName match prefix index]} {
        set index 0
        set prefix "$fileName"
    }
    incr index
    return "${prefix}.${index}"
}

proc lastFileName {fileName} {
    if {! [regexp "^(.*)\\.(\[0-9\]+)$" $fileName match prefix index]} {
        set index 0
        set prefix "$fileName"
    }
    incr index -1
    if {$index <= 0} {return $prefix}
    return "${prefix}.${index}"
}
    
proc getPW {host} {
    global env
    set dfltPW ""
    set PW ""
    if {[info exists env(HOME)]} {
        set pfile "$env(HOME)/bin/.door"
    } else {
        set pfile ".door"
    }
    set fd [open $pfile r]
    if {[info exists fd]} {
        while {! [eof $fd]} {
            set line [gets $fd]
            if {[eof $fd]} {break}
            if {[llength $line] == 1} {
                set dfltPW [string trim $line]
            } elseif {[llength $line] == 2} {
                if {[lindex $line 0] == $host} {set PW [lindex $line 1]}
            }
        }
        close $fd
    }
    if {[string length $PW] == 0} {set PW $dfltPW}
    return $PW
}

proc getFile {host file output} {
    global spawn_id group options
#
#puts "get $file from $host to $output"
#return
#
    set PW [getPW $host]
    set spawnid [spawn ftp $host]
    putLog "spawn process $spawnid"
    if {$spawnid == 0} {return -1}
    expect {
        timeout {
            putLog "No login prompt from ftp $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "eof at login from ftp $host"
            catch {send "quit\r"}
            return -1
        } "Name (*):" {}
    }
    catch {send "sg955813\r"}
    expect {
        timeout {
            putLog "No password prompt from ftp $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "eof at password from ftp $host"
            catch {send "quit\r"}
            return -1
        } "Password:" {
        }
    }
    send "$PW\r"
    expect {
        timeout {
            putLog "Login timeout at $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "eof after password from $host"
            catch {send "quit\r"}
            return -1
        } "Login incorrect." {
            putLog "Login incorrect at $host"
            catch {send "quit\r"}
            nextHost
            return -1
        } "Login failed." {
            putLog "Login failed at $host"
            catch {send "quit\r"}
            nextHost
            return -1
        } "ftp> " {
        }
    }
    set dir2 [string tolower $group]
    set dir1 "[string toupper [string index $group 0]][string range $group 1 end]"
    send "cd /opt/atseintl/$dir1/$dir2\r"
    expect {
        timeout {
            putLog "Directory change timed out on $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "Directory change eof on $host"
            catch {send "quit\r"}
            return -1
        } "Failed" {
            putLog "Directory change failed on $host"
            catch {send "quit\r"}
            return -1
        }  "Directory successfully changed." {
        }
    }
    expect {
        timeout {
            putLog "ftp timeout on $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "ftp eof on $host"
            catch {send "quit\r"}
            return -1
        } "ftp> " {
        }
    }
    set noFile 0
    send "get $file $output\r"
    expect {
        -timeout 300
        timeout {
            putLog "ftp get timed out on $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "ftp eof in get on $host"
            catch {send "quit\r"}
            return -1
        } "Failed" {
            putLog "ftp get failed on $host"
            if {[info exists options(altFile)]} {
                set noFile 1
            } else {
                catch {send "quit\r"}
                return -1
            }
        } "Error" {
            putLog "ftp get error on $host"
            if {[info exists options(altFile)]} {
                set noFile 1
            } else {
                catch {send "quit\r"}
                return -1
            }
        }  "OK" {
        }
    }
    if {$noFile} {
        puts "try alt"
        if {[tryAlt $spawn_id $options(altFile) $output] < 0} {
            catch {send "quit\r"}
            return -1
        }
    }
    expect {
        timeout {
            putLog "Timeout after get on $host"
            catch {send "quit\r"}
            return -1
        } eof {
            putLog "eof after get on $host"
            catch {send "quit\r"}
            return -1
        } "ftp> " {
        }
    }
    catch {send "quit\r"}
    expect {
        timeout {
            putLog "Timeout on exit $host"
            return -1
        } "Goodbye" { 
        }
    }
    putLog [wait] 
    return 0
}

proc tryAlt {spawnid file output} {
    global host
    expect {
        -i $spawnid
        -timeout 2
        timeout {
            putLog "ftp timeout on $host"
        } eof {
            putLog "ftp eof on $host"
            return -1
        } "ftp> " {
        }
    }
    puts "get $file $output\r"
    send -i $spawnid "\r"
    send -i $spawnid "get $file $output\r"
    expect {
        -i $spawnid
        -timeout 300
        timeout {
            putLog "ftp get timed out on $host for $file"
            return -1
        } eof {
            putLog "ftp eof in get on $host for $file"
            return -1
        } "Failed" {
            putLog "ftp get failed on $host for $file"
            return -1
        } "Error" {
            putLog "ftp get error on $host for $file"
            return -1
        }  "OK" {
        }
    }
    return 0
}
    
proc nextHost {} {
    global serverIndex PricingServers
    incr serverIndex
    if {$serverIndex >= [llength $PricingServers]} {set serverIndex 0}
    return [lindex $PricingServers $serverIndex]
}

proc nextFile {} {
    global host delay remoteFile startFile 
    global retryCount retryLimit retryDelay maxFiles
    set name $startFile
    for {set i 0} {$i < $maxFiles} {incr i} {
        set name [lastFileName $name]
        if {[file -isfile $name]} {break}
    }
    if {$i >= $maxFiles} {
        after [expr $retryDelay * 1000] nextFile
    }
    if {[getFile $host $remoteFile $startFile] < 0} {
        puts "[wait -nowait]"
        incr retryCount
        if {$retryCount > $retryLimit} {
            set host [nextHost]
            set retryCount 0
        }
        putLog "Set retry $retryCount on $startFile from $host"
        after [expr $retryDelay * 1000] nextFile
        return
    }
    set retryCount 0
    set startFile [nextFileName $startFile]
    set host [nextHost]
    after [expr $delay * 1000] nextFile
}
    
setOptions $allOptions
foreach option $allOptions {
    if {[info exists options($option)]} {
        set $option $options($option)
    }
}

if {$help} {
    printUsage
    exit 0
}

if {[regexp "^\[0-9\]+$" $host]} {
    set serverIndex $host
} else {
    set serverIndex [lsearch -exact $PricingServers $host]
}
if {$serverIndex < 0 || $serverIndex >= [llength $PricingServers]} {
    set serverIndex 0
}

if {[string compare $logFile 0] != 0} {
    set log [open $logFile a]
    if {! [info exists log]} {
        puts stderr "Can't open log file: $logFile"
        exit 1
    }
}

nextFile
fileevent stdin readable checkInput
trap printStatus 1
trap exitSignal 2

vwait done
exit 0

