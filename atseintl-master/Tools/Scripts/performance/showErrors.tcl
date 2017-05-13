#!/usr/bin/tclsh
#
# ------------------------------------------------------------------
# showErrors.tcl
#   Use the app console interface to retrieve cache statistics
# Author: Jim Sinnamon
# Copyright: Sabre 2007
# ------------------------------------------------------------------

set defaultPort 5006
proc printUsage {} {
    global argv0 defaultPort
    puts stderr "
Usage: [file tail $argv0] \[OPTIONS...\]
   or: [file tail $argv0] host:port \[OPTIONS...\]
Options:
    --host=hostname       Host name e.g. atsela04
    --port=portNumber     TCP port for tseserver (default=$defaultPort)
    --errorText=filename  Text for error codes
"
}

proc setOptions {} {
    global options argv
    set allOptions "host port errorText errTable"
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

proc defaultErrorNames {} {
    global errorTxt
    set errorTxt(0000) NO_ERROR
    set errorTxt(4001) NO_QUOTABLE_FARES
    set errorTxt(4002) INVALID_INCLUSION_CODE
    set errorTxt(4003) INVALID_PAX_TYPE_CODE
    set errorTxt(4004) INVALID_WEB_MULTICARRIER
    set errorTxt(4005) INVALID_WEB_AND_PAXTYPE
    set errorTxt(4006) OUTPUT_SIZE_TOO_BIG
    set errorTxt(4007) NO_USER_PREFERENCE_FOUND
    set errorTxt(4008) CXR_NOT_SPECIFIED_FARES_FOR_INCL_CODE
    set errorTxt(4009) PRCRD_FARE_BASIS_NOT_FOUND
    set errorTxt(4010) LONGRD_FARE_BASIS_NOT_FOUND
    set errorTxt(4011) SHORTRD_FARE_BASIS_NOT_FOUND
    set errorTxt(4012) FARE_BASIS_NOT_FOUND
    set errorTxt(4901) MP_WITH_ROUTING_FARE
    set errorTxt(4902) MP_OW_ON_RT_FARE
    set errorTxt(4903) MP_HR_ON_OW_FARE
    set errorTxt(5002) NO_FARE_REQUESTED
    set errorTxt(5005) PRICING_REST_BY_GOV
    set errorTxt(5011) NO_FARE_FOR_CLASS_USED
    set errorTxt(5012) CITY_PAIR_RESTRICTED_BY_FBM
    set errorTxt(5013) CANNOT_COMPUTE_TAX
    set errorTxt(5017) AIRPORT_CODE_NOT_IN_SYS
    set errorTxt(5018) FARE_NOT_IN_SYS
    set errorTxt(5019) CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS
    set errorTxt(5020) JT_RESTRICTED_RTG_INVALID
    set errorTxt(5023) TRIP_EXCEEDS_MPM_SOM
    set errorTxt(5025) MAX_NUMBER_COMBOS_EXCEEDED
    set errorTxt(5026) FREE_SURCHARGED_STOPOVER_MAY_APPLY
    set errorTxt(5029) TOO_MANY_SURFACE_SEGS
    set errorTxt(5030) NO_DATE_IN_OPEN_SEG
    set errorTxt(5031) CHECK_LINE_OF_FLIGHT
    set errorTxt(5032) STOPOVER_SURCHARGE_MAY_APPLY
    set errorTxt(5033) NO_RULES_FOR_PSGR_TYPE_OR_CLASS
    set errorTxt(5034) NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT
    set errorTxt(5035) FIRST_SEG_OPEN
    set errorTxt(5036) RETRY_IN_ONE_MINUTE
    set errorTxt(5037) NO_TIMES_IN_OA_SEG
    set errorTxt(5038) SYSTEM_ERROR
    set errorTxt(5041) FARE_RESTRICTED_FROM_PRICING
    set errorTxt(5043) INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM
    set errorTxt(5044) PRICING_AT_PEAK_USAGE
    set errorTxt(5045) MIXED_CLASS_TRIP
    set errorTxt(5046) MAX_PERMITTED_MILEAGE_NOT_AVAIL
    set errorTxt(5047) MAX_SEGS_EXCEEDED
    set errorTxt(5049) TOTAL_FARE_TOO_LARGE
    set errorTxt(5050) SYSTEM_DATA_ERROR
    set errorTxt(5052) FAILED_DUE_TO_COMBO_RESTRICTIONS
    set errorTxt(5053) NO_COMBINABLE_FARES_FOR_CLASS
    set errorTxt(5054) CHAR_FOR_TAT_TM
    set errorTxt(5056) MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING
    set errorTxt(5057) CANNOT_FORMAT_TAX
    set errorTxt(5059) NBR_PSGRS_EXCEEDS_OA_AVAIL
    set errorTxt(5060) NO_FARE_VALID_FOR_PSGR_TYPE
    set errorTxt(5062) EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS
    set errorTxt(5064) CHECK_FLIGHT
    set errorTxt(5091) FARE_CALC_TOO_LARGE_FOR_ATB
    set errorTxt(5092) CANNOT_CALC_SD_FARE
    set errorTxt(5094) TICKET_DESIGNATOR_NOT_ALLOWED
    set errorTxt(5095) WPNJ_NOT_VALID_ON_THIS_ITIN
    set errorTxt(5096) CANNOT_CALCULATE_CURRENCY
    set errorTxt(5097) CANNOT_ROUND_CURRENCY
    set errorTxt(5099) BREAK_FARE_INVALID
    set errorTxt(5100) RETRY
    set errorTxt(5101) NEED_COMMISSION
    set errorTxt(5104) OPEN_JAW_MAY_APPLY
    set errorTxt(5105) FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS
    set errorTxt(5106) FARE_BASIS_NOT_AVAIL
    set errorTxt(5111) TRANSACTION_THRESHOLD_REACHED
    set errorTxt(5112) WORLDFARE_AT_PEAK_USE
    set errorTxt(5113) WORLDFARE_UNAVAILABLE
    set errorTxt(5114) TKT_DES_FAILE_RULES_CHECK
    set errorTxt(5115) TKT_DES_RECORD_RETRIEVAL_ERROR
    set errorTxt(5116) PROCESSING_ERROR_DETECTED
    set errorTxt(5119) PENALTY_DATA_INCOMPLETE
    set errorTxt(5124) LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED
    set errorTxt(5125) WRONG_NUMBER_OF_FARES
    set errorTxt(5126) DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL
    set errorTxt(5127) ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT
    set errorTxt(5129) OPEN_RETURN_REQUIRED
    set errorTxt(5130) NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY
    set errorTxt(5131) CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY
    set errorTxt(5132) CANNOT_TAX_INSERT_TAX_AFTER_TX
    set errorTxt(5133) MULTI_DISCOUNTS_EXIST
    set errorTxt(5134) NO_VALID_DISCOUNT_PUB_FARE
    set errorTxt(5136) SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE
    set errorTxt(5137) DATA_ERROR_DETECTED
    set errorTxt(5138) NO_NOPNROPTIONS_FOR_TRX
    set errorTxt(5139) NO_PNR_CREATED
    set errorTxt(5140) NO_FARE_FOR_CLASS
    set errorTxt(5142) CANNOT_PRICE_AS_REQUESTED
    set errorTxt(5146) NO_FARES_RBD_CARRIER
    set errorTxt(5148) NEGOTIATED_FARES_APPLY
    set errorTxt(5150) NO_FARES_FOUND_FOR_FARE_COMPONENT
    set errorTxt(5151) CODESHARE_PROCESSING_ERROR
    set errorTxt(5153) NO_MATCH_FOR_FARE_COMPONENT
    set errorTxt(5154) INVALID_FARE_BASIS_FOR_CARRIER
    set errorTxt(5155) REQ_CARRIER_HAS_NO_FARES
    set errorTxt(5156) EXCEED_LENGTH_UNABLE_TO_CALCULATE_FARE
    set errorTxt(5157) EXCEED_LENGTH_UNABLE_TO_CONVERT_CURRENCY
    set errorTxt(5158) NO_APPLICABLE_YY_FARES
    set errorTxt(5159) UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE
    set errorTxt(5160) BAD_FARE_PATH_TOTAL_NUC_AMOUNT
    set errorTxt(5161) EMPTY_PRICING_UNIT
    set errorTxt(5162) EMPTY_FARE_USAGE
    set errorTxt(5163) EMPTY_TRAVEL_SEG
    set errorTxt(5164) INVALID_GLOBAL_DIRECTION_REQUESTED
    set errorTxt(5165) FAILED_TO_CONNECT_RULE_TEXT_SERVER
    set errorTxt(5166) INVALID_CORP_ID
    set errorTxt(5167) UNABLE_TO_PRICE_ISSUE_SEPARATE_TICKETS
    set errorTxt(5168) MISSING_NUC_RATE
    set errorTxt(5169) FLIGHTS_CONTINUITY
    set errorTxt(5170) NET_FARES
    set errorTxt(5500) ISSUE_SEPARATE_TICKET
    set errorTxt(5501) UTAT_NET_SELLING_AMOUNTS_CONFLICT
    set errorTxt(5502) UTAT_MULTIPLE_TOUR_CODES
    set errorTxt(5503) UTAT_TOUR_CODE_NOT_FOUND
    set errorTxt(5504) UTAT_INVALID_TEXT_BOX_COMBO
    set errorTxt(5505) UTAT_COMMISSIONS_NOT_COMBINABLE
    set errorTxt(5506) NET_REMIT_FARE_PHASE_FOUR
    set errorTxt(5507) UNABLE_TO_PROCESS_NEG_FARE_DATA
    set errorTxt(5508) NO_NET_FARE_AMOUNT
    set errorTxt(5509) INVALID_COMBO_NO_NET_FARE_DISPLAY
    set errorTxt(5511) UNABLE_AUTO_TKT_INV_COMM_AMOUNT
    set errorTxt(5512) UNABLE_AUTO_TKT_INV_NET_REMIT_FARE
    set errorTxt(5600) PSG_TYPE_ERROR
    set errorTxt(5601) FARE_APPLICABLE_FOR_ONEWAY_ONLY
    set errorTxt(5602) FARE_APPLICABLE_FOR_ROUNDTRIP_ONLY
    set errorTxt(5603) FARE_NOT_PERMITTED_FOR_PRICING
    set errorTxt(6001) INVALID_JOURNEY_RECORD
    set errorTxt(6002) INVALID_INPUT
    set errorTxt(6004) INVALID_DUTY_CODE
    set errorTxt(6005) NO_ITIN_SEGS_FOUND
    set errorTxt(6006) PNR_DATABASE_ERROR
    set errorTxt(6007) NEED_PREFERRED_CARRIER
    set errorTxt(6009) MAX_PREF_CARRIERS_EXCEEDED
    set errorTxt(6012) MAX_PASSENGERS_EXCEEDED
    set errorTxt(6013) INVALID_SAME_PREF_AND_NON_PREF_CARRIER
    set errorTxt(6014) ENTER_ACTIVE_SEGS
    set errorTxt(6015) CONFLICTING_OPTIONS
    set errorTxt(6016) INVALID_SEG_TYPE_IN_PNR_RETIEVAL
    set errorTxt(6017) PNR_SEGS_NOT_FOUND
    set errorTxt(6018) CHECK_SEG_CONTINUITY
    set errorTxt(6022) INVALID_TERMINAL_TYPE
    set errorTxt(6024) MULTI_CONNECT
    set errorTxt(6027) DUPLICATE_X_AND_O_QUALIFIER
    set errorTxt(6028) CANNOT_USE_SEG_NUMBER
    set errorTxt(6030) BOOK_SEPARATE_PNR
    set errorTxt(6031) MAX_FOUR_SEATS
    set errorTxt(6034) SPECIFY_PUBLIC_OR_PRIVATE
    set errorTxt(6039) NO_DIAGNOSTIC_TO_DISPLAY
    set errorTxt(6040) NO_CONVERTED_FARE_BASIS_CODE
    set errorTxt(6041) NO_PQ_ITEMS_FOR_GIVEN_ORDINAL
    set errorTxt(6042) FARENET_PROCESSING_NOT_YET_AVAIL
    set errorTxt(6043) CALL_DIRECT
    set errorTxt(6050) NO_FLIGHTS_FOUND
    set errorTxt(6051) INVALID_CLASS
    set errorTxt(6052) NO_COMBOS
    set errorTxt(6053) INVALID_FORMAT
    set errorTxt(6054) NO_TRIP
    set errorTxt(6055) CANNOT_SELL_SEG
    set errorTxt(6057) MUST_BE_PRICED_FIRST
    set errorTxt(6060) NO_FLIGHT_ITEM_BLOCKS
    set errorTxt(6062) FIND_ERR
    set errorTxt(6063) NO_FLIGHT_ITEM_SORT_LIST_BLOCKS
    set errorTxt(6064) NO_FLIGHT_COMB_BLOCKS
    set errorTxt(6065) INVALID_JRFCT_PARMS
    set errorTxt(6070) QJRA_TRAN_VECTOR_ERR
    set errorTxt(6071) QJRB_TRAN_VECTOR_ERR
    set errorTxt(6072) NO_DIRECTS_NONSTOPS
    set errorTxt(6080) TOO_MANY_COMBOS
    set errorTxt(6083) CORPORATE_PRICING_ACTIVE
    set errorTxt(6088) ALT_CITIES_INVALID_FOR_ARNK
    set errorTxt(6089) ALT_DATES_INVALID_FOR_ARNK
    set errorTxt(6090) TRIP_DURATION_INVALID_FOR_ARNK
    set errorTxt(6091) MAX_CONNECTION_TIME_INVALID_FOR_ARNK
    set errorTxt(6092) MAX_TRAVEL_TIME_INVALID_FOR_ARNK
    set errorTxt(6093) ALT_CITIES_NOT_ALLOWED
    set errorTxt(6094) INVALID_SEG_NUMBER
    set errorTxt(6095) INVALID_CITY_AIRPORT_CODE
    set errorTxt(6096) NEED_SEPARATOR
    set errorTxt(6097) SEG_DOES_NOT_MATCH_CITY_AIRPORT
    set errorTxt(6098) NO_CITY_AIRPORT_FOR_ALTS
    set errorTxt(6099) ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT
    set errorTxt(6100) ALT_CITY_AIRPORT_AND_MILEAGE_RANGE
    set errorTxt(6101) MILEAGE_RANGE_INVALID
    set errorTxt(6102) INVALID_ALT_CITY_AIRPORT_CODE
    set errorTxt(6103) INTERNATIONAL_ALT_CITY_AS_PSEUDO
    set errorTxt(6104) MAX_ALT_AIRPORT_CITIES_EXCEEDED
    set errorTxt(6105) DUPLICATE_BOARD_OFF_POINT_IN_SEG
    set errorTxt(6106) SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES
    set errorTxt(6107) INCLUDE_CONNECTING_SEGS
    set errorTxt(6108) INVALID_NUMBER_FOR_ALT_DATES
    set errorTxt(6109) COMBINED_CONSEC_NONCONSEC_ALT_DATES
    set errorTxt(6110) INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES
    set errorTxt(6111) MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED
    set errorTxt(6112) DURATION_ONLY_VALID_FOR_ALT_DATES
    set errorTxt(6113) DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES
    set errorTxt(6114) DURATION_IS_INVALID
    set errorTxt(6115) DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS
    set errorTxt(6116) MAX_CONNECTION_TIME_EXCEEDED
    set errorTxt(6117) MAX_TRAVEL_TIME_EXCEEDED
    set errorTxt(6118) PRIOR_DATE_DEPART_NOT_CHECKED
    set errorTxt(6119) NO_PRIVATE_FARES_VALID_FOR_PASSENGER
    set errorTxt(6120) NO_PUBLIC_FARES_VALID_FOR_PASSENGER
    set errorTxt(6122) PAXTYPE_NOT_ALLOWED_WITH_FARE_TYPE_PRICING
    set errorTxt(6124) NO_CORPORATE_NEG_FARES_EXISTS
    set errorTxt(6200) NO_VALID_DATAPAIR_FOUND
    set errorTxt(6201) NO_VALID_FLIGHT_DATE_FOUND
    set errorTxt(6202) NO_VALID_FARE_FOUND
    set errorTxt(6203) EMPTY_ALT_DATES_PAIRS_MAP
    set errorTxt(7001) NETWORK_EXCEPTION
    set errorTxt(7002) MEMORY_EXCEPTION
    set errorTxt(7003) DCA_CORBA_EXCEPTION
    set errorTxt(7050) DB_CURSOR_OPEN_ERROR
    set errorTxt(7051) DB_CURSOR_FETCH_ERROR
    set errorTxt(7052) DB_CURSOR_CLOSE_ERROR
    set errorTxt(7053) DB_INSERT_ERROR
    set errorTxt(7054) DB_UPDATE_ERROR
    set errorTxt(7055) DB_SELECT_ERROR
    set errorTxt(7070) DB_BEGIN_WORK_ERROR
    set errorTxt(7071) DB_COMMIT_WORK_ERROR
    set errorTxt(7072) DB_TIMEOUT_ERROR
    set errorTxt(8001) SSG_FILE_OPEN_ERROR
    set errorTxt(8002) SSG_DATABASE_ERROR
    set errorTxt(8003) SSG_RECORD_ID_ERROR
    set errorTxt(8004) SSG_PROCESSING_STARTED
    set errorTxt(8005) SSG_PROCESSING_COMPLETED
    set errorTxt(8006) CNP_PROCESSING_COMPLETED_EOF
    set errorTxt(8007) EQP_PROCESSING_FILE_NUMBER
    set errorTxt(8008) CNP_DELETE_OLD_FAILED
    set errorTxt(8009) SSG_DELETE_FAILED
    set errorTxt(8010) SSG_ITEM_DELETED
    set errorTxt(8011) SSG_UPDATE_FAILED
    set errorTxt(8012) SSG_ITEM_UPDATED
    set errorTxt(8013) SSG_ADD_FAILED
    set errorTxt(8014) SSG_ITEM_ADDED
    set errorTxt(8015) SSG_DUPLICATE_FOUND
    set errorTxt(8016) SSG_SYSTEM_SUSPENDED
    set errorTxt(8017) SSG_PROCESSING_RESTARTED
    set errorTxt(8018) SSG_INPUT_FILE_MISSING
    set errorTxt(8019) CNP_PROCESSING_COMPLETED_NEF
    set errorTxt(8020) SSG_CHECKPOINT_FAILED
    set errorTxt(8030) SSG_MCT_MCTREGIONS_SQL_ERROR
    set errorTxt(8031) SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR
    set errorTxt(8032) SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR
    set errorTxt(8033) SSG_MCT_INVALID_ENTRY_CODE
    set errorTxt(8034) SSG_MCT_INVALID_ACTION_CODE
    set errorTxt(8037) SSG_MCT_FILES_SWITCHED
    set errorTxt(8038) SSG_MCT_FILES_NOT_SWITCHED
    set errorTxt(8039) SSG_MCT_SWITCH_ERROR
    set errorTxt(8041) SSG_MCT_CACHE_NOTIFY_ERROR
    set errorTxt(8042) SSG_MCT_MVS_LOAD_STARTED
    set errorTxt(8043) SSG_MCT_MVS_LOAD_COMPLETE
    set errorTxt(8044) SSG_MCT_MVS_LOAD_ERROR
    set errorTxt(8046) SSG_MCT_MVSFILE_LOADER_SUSPENDED
    set errorTxt(8047) SSG_SCHEDULE_BATCH_LOAD_STARTED
    set errorTxt(8048) SSG_SCHEDULE_BATCH_LOAD_START_FAILED
    set errorTxt(8049) SSG_SCHEDULE_BATCH_LOAD_FAILED
    set errorTxt(8050) SSG_SCHEDULE_BATCH_LOAD_COMPLETED
    set errorTxt(8051) SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR
    set errorTxt(8052) SSG_SCHEDULE_BATCH_PROCESSING_ERROR
    set errorTxt(8055) SSG_SCHEDULE_INVALID_ARGS_ERROR
    set errorTxt(8056) SSG_SCHEDULE_APPLICATION_INIT_ERROR
    set errorTxt(8057) SSG_SCHEDULE_APPLICATION_LOGIC_ERROR
    set errorTxt(8060) STARTED
    set errorTxt(8061) COMPLETED
    set errorTxt(8062) FAILED
    set errorTxt(8063) SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR
    set errorTxt(8070) SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR
    set errorTxt(8090) SSG_QUERY_CHECKPOINT_FAILED
    set errorTxt(8091) SSG_UPDATE_CHECKPOINT_FAILED
    set errorTxt(8501) BSR_INVALID_DATA
    set errorTxt(8502) BSR_PROCESSING_START
    set errorTxt(8503) BSR_ACTION_CODE
    set errorTxt(8504) BSR_PROCESSING_END
    set errorTxt(8505) BSR_EMU_NATION_IGNORED
    set errorTxt(8506) BSR_DB_ERROR
    set errorTxt(8550) UNABLE_TO_MATCH_FARE
    set errorTxt(8551) UNABLE_TO_MATCH_REISSUE_RULES
    set errorTxt(8552) REISSUE_RULES_FAIL
    set errorTxt(8553) NUMBER_OF_REISSUES_RESTRICTED
    set errorTxt(9000) FARE_CURRENCY_OVERRIDE_NOT_VALID
    set errorTxt(9001) ATAE_RETURNED_NO_BOOKING_CODES
    set errorTxt(9002) RESTRICTED_CURRENCY
    set errorTxt(9003) AGENT_CURRENCY_CODE_MISSING
    set errorTxt(9004) AGENT_PCC_NON_EXISTENT
    set errorTxt(9005) INVALID_CURRENCY_CODE_REQUESTED
    set errorTxt(9006) DSS_RETURNED_NO_SCHEDULES
    set errorTxt(9400) SAX_PARSER_FAILURE
    set errorTxt(9501) LMT_ISSUE_SEP_TKTS_EXCEED_NUM_DEPT_ARR
    set errorTxt(9502) LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR
    set errorTxt(9503) LMT_ISSUE_SEP_TKTS_EXCEED_NUM_DEPT_ARR_JAL
    set errorTxt(9601) MIN_FARE_MISSING_DATA
    set errorTxt(9997) REQUEST_TIMEOUT
    set errorTxt(9998) SYSTEM_EXCEPTION
    set errorTxt(9999) UNKNOWN_EXCEPTION
}

set args [setOptions]
if {[llength $args] > 1} {
    printUsage
    exit 1
}
if {[llength $args] == 1} {
    set hostPortMatch "^(\[A-Za-z0-9\\.\]+):(\[0-9\]+)$"
    if {! [regexp $hostPortMatch [lindex $args 0] match host port]} {
        set host [lindex $args 0]
    }
}

if {[info exists options(host)]} {set host $options(host)}
if {[info exists options(port)]} {set port $options(port)}
if {! [info exists host]} {
    puts "Missing host name"
    printUsage
    exit 1
}
if {! [info exists port]} {set port $defaultPort}

if {[info exists options(errorText)]} {
    set txt [open $options(errorText) r]
    if {[info exists txt]} {
        set errMatch {([A-Z][A-Za-z_]*)[ \t]*=[ \t]*([0-9]+)}
        while {! [eof $txt]} {
            set line [gets $txt]
            if {[eof $txt]} {break}
            if {[regexp $errMatch $line match name code]} {
                set errorTxt($code) $name
            }
        }
        close $txt
        if {[info exists options(errTable)]} {
            foreach code [lsort [array names errorTxt]] {
                puts "set errorTxt($code) $errorTxt($code)"
            }
        }
    }
} else {
    defaultErrorNames
}

set fd [socket $host $port]
if {! [info exists fd]} {
    puts "Could not connect to $host:$port"
    exit 1
}
puts "Connected to $host:$port"
set errorList [readAppConsole $fd "ERCT00010000"]
set pairs [split $errorList |]
puts "  Code    Count  Name"
foreach {code count} $pairs {
    if {[string length $code] > 0} {
        puts -nonewline [format "%6s %8s" $code $count]
        if {[info exists errorTxt($code)]} {
            puts -nonewline "  $errorTxt($code)"
        }
        puts ""
    }
}
