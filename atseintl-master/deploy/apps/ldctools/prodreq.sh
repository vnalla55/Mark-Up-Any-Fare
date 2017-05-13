#!/bin/bash

# ensure we are running within the same directory
# where we are located 
cd $(dirname $(which ${0}))

. ./env.vars

cd ${ENV_VARS_LOCATION}

declare produser=$(whoami)
declare targetdir="."
declare socketLog="tserequest.log.1"
declare mipDir="/opt/atseintl/shopping"
declare isDir="/opt/atseintl/shoppingis"
declare v2pricDir="/opt/atseintl/pricingv2"
declare histDir="/opt/atseintl/historical"
declare newLogCombined=""

declare -i total_files=0
declare -i total_txns=0

declare -a mip=(piclp016 piclp017 piclp018 piclp058 piclp059 piclp060 \
                piclp061 piclp062 piclp063 piclp064 piclp065 piclp067 \
                piclp153 piclp154 piclp155 piclp156 piclp168 piclp184 \
                piclp185 piclp186 piclp187 piclp188 piclp244 piclp245 \
                piclp246 piclp247 piclp260 piclp261 piclp262 piclp263 \
                piclp264 piclp265 piclp266 piclp267 piclp268 piclp269 \
                piclp270 piclp271 piclp272 piclp273 piclp274 piclp275 \
                piclp276 piclp277 piclp326 piclp331)

declare -a mip200=(piclp408 piclp409 piclp410 piclp411 piclp412 piclp413 \
                   piclp414 piclp415 piclp416 piclp417 piclp418 piclp419 \
                   piclp420 piclp421 piclp422 piclp423 piclp424 piclp425 \
                   piclp426 piclp427 piclp428 piclp429 piclp430 piclp431 \
                   piclp432 piclp433 piclp434 piclp435 piclp436 piclp450 \
                   piclp451 piclp452 piclp453 piclp454 piclp455 piclp456 \ 
                   piclp457 piclp458 piclp459 piclp460 piclp461 piclp462 \
                   piclp463 piclp464 piclp465 piclp466 piclp467 piclp468 \
                   piclp469 piclp470 piclp471 piclp472 piclp473 piclp474 \
                   piclp475 piclp476 piclp477 piclp478 piclp479 piclp480 \
                   piclp481 piclp482 piclp483 piclp484 piclp485 piclp486 \
                   piclp487 piclp488 piclp489 piclp490 piclp491 piclp492 \
                   piclp493 piclp494 piclp495 piclp496 piclp497 piclp498 \
                   piclp499 piclp500 piclp501 piclp502 piclp503 piclp504 \
                   piclp505 piclp506 piclp507 piclp508 piclp509 piclp510 \
                   piclp511 piclp512 piclp513 piclp514 piclp515 piclp516 \
                   piclp517 piclp518 piclp519 piclp520 piclp521 piclp522 \
                   piclp523 piclp524 piclp530 piclp531 piclp532 piclp533 \
                   piclp534 piclp535 piclp536 piclp537 piclp538 piclp539 \
                   piclp540 piclp541 piclp542 piclp543 piclp544 piclp545 \
                   piclp546 piclp547 piclp548 piclp549 piclp550 piclp551 \
                   piclp552 piclp553 piclp554 piclp555 piclp556 piclp557 \
                   piclp558 piclp559 piclp560 piclp561 piclp562 piclp563 \
                   pimlp400 pimlp401 pimlp402 pimlp403 pimlp404 pimlp405 \
                   pimlp406 pimlp407 pimlp408 pimlp409 pimlp410 pimlp411 \
                   pimlp412 pimlp413 pimlp414 pimlp415 pimlp416 pimlp417 \
                   pimlp418 pimlp419)

declare -a is=(piclp146 piclp157 piclp158 piclp159 piclp160 piclp195 \
               piclp196 piclp197 piclp198 piclp199 piclp200 piclp201 \
               piclp202 piclp203 piclp204 piclp205 piclp206 piclp207 \
               piclp208 piclp209 piclp210 piclp211 piclp212 pimlp008 \
               pimlp009 pimlp010 pimlp011 pimlp012 pimlp035 pimlp036 \
               pimlp037 pimlp059 pimlp060 pimlp061 pimlp063 pimlp064 \
               pimlp065 pimlp066 pimlp067 pimlp077 pimlp078)

declare -a is200=(piclp400 piclp401 piclp402 piclp403 piclp404 piclp405 \
                  piclp406 piclp407 piclp408 piclp409 piclp410 piclp411 \
                  piclp437 piclp438 piclp439 piclp440 piclp441 piclp442 \
                  piclp443 piclp444 piclp445 piclp446 piclp447 piclp448 \
                  piclp449)

declare -a v2pric=(piclp004 piclp005)

# these are actually cert nodes since we don't
# have historical in prod yet
declare -a hist=(piilc402 piilc403 pinlc116)

declare -i mipSize=${#mip[*]}
declare -i mip200Size=${#mip200[*]}
declare -i isSize=${#is[*]}
declare -i is200Size=${#is200[*]}
declare -i v2pricSize=${#v2pric[*]}
declare -i histSize=${#hist[*]}

declare INFO="INFO"
declare ERROR="ERROR"
declare MIP="shopping"
declare IS="shoppingis"
declare V2P="pricingv2"
declare HIST="historical"
declare DATE_FORMAT="%Y-%m-%d %H:%M:%S"

function output
{
  local sev=$1
  shift

  local msg="$*"
  echo "$(date +"${DATE_FORMAT}") - ${sev} - ${msg}"
}

function getSocketLog
{
   local    nodeType=$1
   local    server=$2
   local -i t_limit=$3
   
   local -i error=0

   local    szcomp=""
   local    szuncomp=""
   local    directory=""
   local    newLog="${targetdir}/${nodeType}.${server}.tserequest.log"
   local    xmlroot=""
   local    extract=""
  
   case "${nodeType}" in
     "${MIP}"  ) directory=$mipDir    ; xmlroot="shopping" ;;
     "${IS}"   ) directory=$isDir     ; xmlroot="shopping" ;;
     "${V2P}"  ) directory=$v2pricDir ; xmlroot="pricing"  ;;
     "${HIST}" ) directory=$histDir   ; xmlroot="pricing"  ;;
     *         ) return 1 ;;
   esac 

   # just to make sure we can connect
   output $INFO "Connecting to [$server]" 
   ssh ${produser}@${server} 'ls' > /dev/null 2>&1
   error=$?
   if [ $error -ne 0 ] ; then
     output $ERROR "Failure in connecting to [$server]!" 
     return $error
   fi
     
   output $INFO "Connected to [$server] OK." 

   if [ ! -e ${targetdir} ] ; then
     mkdir -p ${targetdir}
   fi

   if [ ${total_files} -eq 0 ] ; then
     rm -f ${newLogCombined}
   fi

   rm -f ${newLog}
   rm -f ${newLog}.tmp

   output $INFO "Getting request log [$directory/$socketLog] from [$server]" 
   scp ${produser}@${server}:${directory}/${socketLog} ${newLog} > /dev/null 2>&1
   error=$?
   if [ $error -ne 0 ] ; then
     output $ERROR "Failure retrieving ${server}:${directory}/${socketLog}!" 
     return $error
   fi

   case "${xmlroot}" in
     "shopping") extract="cat ${newLog} | grep -v '[{}][{}][{}]' | awk '{gsub(/<\\/ShoppingRequest>/, \"</ShoppingRequest>~\");print;}' | tr -d '\\n' | tr '~' '\\n' > ${newLog}.tmp" ;;
     "pricing" ) extract="cat ${newLog} | sed 's/20[0-9][0-9].*Request Received - //g' > ${newLog}.tmp " ;;
     *         ) return 1 ;;
   esac
   output $INFO "Extracting xml requests from [$newLog]" 
   eval ${extract}
   error=$?
   rm -f ${newLog}
   if [ $error -ne 0 ] ; then
     output $ERROR "Failure extracting xml requests from [${newLog}]!" 
     return $error
   fi

   local -i trancount=$(wc -l ${newLog}.tmp | awk '{print $1}') 
   local -i tranneeded=$(expr ${t_limit} - ${total_txns}) 

   output $INFO "Appending xml requests to [$newLogCombined]" 
   if [ ${trancount} -lt ${tranneeded} ] ; then
     cat ${newLog}.tmp | gzip >> ${newLogCombined}  
     total_txns=$(expr ${total_txns} + ${trancount})
   else
     local cmd="head -${tranneeded} ${newLog}.tmp | gzip >> ${newLogCombined}"
     eval ${cmd}
     total_txns=$(expr ${total_txns} + ${tranneeded})
   fi
   rm -f ${newLog}.tmp

   total_files=$(expr ${total_files} + 1)
   return $error
}

function process_nodes
{
  local    nodeType=${1}
  local -i t_limit=$(expr ${2})
  local -i f_limit=$(expr ${3})
  local -a nodelist ;

  case "${nodeType}" in
    "${MIP}"  ) nodelist=("${mip[@]}")    ;;
    "${IS}"   ) nodelist=("${is[@]}")     ;;
    "${V2P}"  ) nodelist=("${v2pric[@]}") ;;
    "${HIST}" ) nodelist=("${hist[@]}") ;;
    *         ) return 1 ;;
  esac 

  newLogCombined="${targetdir}/${nodeType}.tserequest.log.gz"
   
  local -i count=${#nodelist[*]}
  local -i idx=0
 
  while [ ${idx} -lt ${count} -a ${total_files} -lt ${f_limit} -a ${total_txns} -lt ${t_limit} ] ; do
    getSocketLog ${nodeType} ${nodelist[${idx}]} ${t_limit} 
    output $INFO "Total Files Procesed: ${total_files}"
    output $INFO "Total Transactions Procesed: ${total_txns}"
    idx=$(expr ${idx} + 1)
  done

  local outfile="${targetdir}/${nodeType}.${total_txns}.tserequest.log.gz"
  mv ${newLogCombined} ${outfile}

  output $INFO "Local transaction file is: ${outfile}"
}

function usage
{
  echo ""
  echo "USAGE: prodreq.sh [options] <pricingv2|shopping|shoppingis|historical>"
  echo ""
  echo "  where [options] is one or more of the following:"
  echo ""
  echo "    -count <n>      where count is the number of requests"
  echo "                    desired; default = 5000"
  echo ""
  echo "    -user <userid>  where userid is the one that will be used"
  echo "                    for logging in to production box(s) for"
  echo "                    gathering requests; default is the current"
  echo "                    user [$(whoami)]" 
  echo ""
  echo "    -loglimit <n>   where n is the limit on how many request "
  echo "                    log files will be processed; default is "
  echo "                    unlimited (until the number of requests"
  echo "                    desired is satisfied)."
  echo ""
  exit 1
}

function main
{
  local app="${APP}"
  local t_limit=5000
  local f_limit=999999999

  echo "" 

  while [ -n "$1" ] ; do
    case ${1} in
      "-count"   ) shift ; t_limit=${1} ;;
      "-loglimit") shift ; f_limit=${1} ;;
      "-user"    ) shift ; produser=${1} ;;
      *          ) app=$(echo ${1} | tr "[:upper:]" "[:lower:]") ;;
    esac
    shift
  done

  if [ -z "${app}" ] ; then
    output $ERROR "No application type specified."
    usage 
  fi

  case "${app}" in
    "${MIP}"|"${IS}"|"${V2P}"|"${HIST}") output $INFO "Application Type: ${app}" ;;
    *) output $ERROR "Unrecognized application type [${app}]!" ; usage ;;
  esac 

  if [ -z "$(echo ${t_limit} | grep -o -E '[0-9]*')" ] ; then
    output $ERROR "Transaction limit not numeric!" 
    usage 
  fi
  output $INFO "Transaction Limit: ${t_limit}"

  if [ -z "$(echo ${f_limit} | grep -o -E '[0-9]*')" ] ; then
    output $ERROR "File limit not numeric!" 
    usage 
  fi
  output $INFO "File Limit: ${f_limit}"

  process_nodes ${app} ${t_limit} ${f_limit}
}

main $*
