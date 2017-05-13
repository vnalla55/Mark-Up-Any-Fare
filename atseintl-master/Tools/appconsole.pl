#!/usr/bin/perl
use IO::Socket;

local $| = 1;

%serverStats = ();

################################################################################
#
# output TYPE, LINE
#
#   Writes out LINE along with the current time to either STDOUT or STDERR
#   depending on TYPE. TYPE can either be INFO or ERROR.  INFO is directed to 
#   STDOUT and ERROR is directed to STDERR.  If TYPE is neither INFO or ERROR
#   the line will be directed to STDERR.
#
#   ex.  output(INFO, "Information to output to STDOUT");
#
################################################################################

use constant INFO => 0;
use constant ERROR => 1;

sub output
{
   my ($type, $line) = @_;
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

   $year += 1900;
   $mon  += 1;

   my $log_time = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $year,$mon,$mday,$hour,$min,$sec);

   if ($type == INFO)
   {
      printf STDOUT "%s | %s\n", $log_time , $line;
   }
   else
   {
      printf STDERR "%s | ERROR! %s\n", $log_time, $line;
   }

   return;
}

################################################################################
#
################################################################################

$HEADER_TEMPLATE = "N N A4 A4 A4";
$FULL_HEADER_LEN = 20;
$HEADER_LEN      = 16;
$HEADER_VER      = '0001';
$HEADER_REV      = '0000';
$DELIM           = '|';

$DEFAULT_PORT    = '5001';

$CMD_STOP          = 'DOWN';
$CMD_STATS         = 'RFSH';
$CMD_SET_LOG_LEVEL = 'SLOG';
$CMD_GET_LOG_LEVEL = 'GLOG';
$CMD_COUNTER_RESET = 'CNRT';
$CMD_DETAILS       = 'ETAI';
$CMD_REDEPLOY      = 'PLOY';
$CMD_ACTIVATE      = 'ACTI';
$CMD_CACHE_STATS   = 'TATS';
$CMD_COMPR_CACHE_STATS = "CCST";      # ./appconsole.pl <host> <port> CCSTATS <CACHE>
$CMD_TO_ELAPSED    = 'LAPS';
$CMD_CACHE_FLUSH   = 'FLSH';
$CMD_START_PROFILE = 'SPRF';
$CMD_FLUSH_PROFILE = 'FPRF';
$CMD_CLEAR_MEMSTAT = 'CMEM';
$CMD_GET_MEMSTAT   = 'GMEM';
$CMD_DB            = 'DBCN';
$CMD_DCFG          = 'DCFG';
$CMD_ERROR_COUNTS  = 'ERCT';
$CMD_SERVER_CPU    = "SCPU";
$CMD_SERVER_CONFIG = "CNFG";         # ./appconsole.pl <host> <port> SERVER_CONFIG
$CMD_QUERY_ELAPSED = "QELT";
$CMD_RC_HEALTH_CHECK = "RCHC";       # ./appconsole.pl <host> <port> RCHEALTHCHECK <rchost>/<rcport>
$CMD_RC_DISABLE    = "RCDS";         # ./appconsole.pl <host> <port> RCDISABLE
$CMD_RC_ENABLE     = "RCEN";         # ./appconsole.pl <host> <port> RCENABLE [<rchost>/<rcport> [shost/sport]]
$CMD_RC_PERSISTENT = "RCPT";         # ./appconsole.pl <host> <port> RCPERSISTENT
$CMD_RC_NONPERSISTENT = "RCNP";      # ./appconsole.pl <host> <port> RCNONPERSISTENT
$CMD_RC_LDC_DELAY = "RCLD";          # ./appconsole.pl <host> <port> RCLDCDELAY <delay>
$CMD_RC_THREAD_POOL_SIZE = "RCTP";   # ./appconsole.pl <host> <port> RCTHREADPOOLSIZE <sz>
$CMD_RC_MAX_NUMBER_CLIENTS = "RCMC"; # ./appconsole.pl <host> <port> RCMAXNUMBERCLIENTS <number>
$CMD_RC_ENABLE_HEALTHCHECK = "RCEH"; # ./appconsole.pl <host> <port> RCENABLEHEALTHCHECK <[Y|N]>
$CMD_RC_HEALTHCHECK_TIMEOUT = "RCHT";# ./appconsole.pl <host> <port> RCHEALTHCHECKTIMEOUT <to>
$CMD_RC_ASYNCHRONOUS_HEALTHCHECK = "RCAH";# ./appconsole.pl <host> <port> RCASYNCHRONOUSHEALTHCHECK <[Y|N]>
$CMD_RC_HEALTHCHECK_PERIOD = "RCHP"; # ./appconsole.pl <host> <port> RCHEALTHCHECKPERIOD <period>
$CMD_RC_QUEUE_TOLERANCE = "RCQT";    # ./appconsole.pl <host> <port> RCQUEUETOLERANCE <tolerance>
$CMD_RC_SLAVE_CONNECT_TO = "RCSC";   # ./appconsole.pl <host> <port> RCSLAVECONNECTTO <to>
$CMD_RC_LINGER = "RCLR";             # ./appconsole.pl <host> <port> RCLINGER <[Y|N]>
$CMD_RC_LINGER_TIME = "RCLT";        # ./appconsole.pl <host> <port> RCLINGERTIME <time>
$CMD_RC_GET_PARAMETERS = "RCGP";     # ./appconsole.pl <host> <port> RCGETPARAMETERS
$CMD_RC_KEEP_ALIVE = "RCKA";         # ./appconsole.pl <host> <port> RCKEEPALIVE <[Y|N]>
$CMD_RC_GET_CACHES = "RCGC";         # ./appconsole.pl <host> <port> RCGETCACHES
$CMD_RC_USE_CLIENT_SPECIFIED_TIMEOUT = "RCST";# ./appconsole.pl <host> <port> RCUSEREQUESTTO <[Y|N]>
$CMD_RC_CLIENT_PROCESSING_TIMEOUT = "RCRT";   # ./appconsole.pl <host> <port> RCPROCESSTIMEOUT <to>
$CMD_DISPOSE_CACHE = "DSPC";         # ./appconsole.pl <host> <port> DSPCACHE <CACHE>/<fraction>
$CMD_RC_CHECK_BASELINE = "RCCB";     # ./appconsole.pl <host> <port> RCCHECKBASELINE <[Y|N]>
$CMD_RC_IGNORE_DB_MISMATCH = "RCID"; # ./appconsole.pl <host> <port> RCIGNOREDBMISMATCH <[Y|N]>
$CMD_RC_CACHE_UPDATE_DETECTION_INTERVAL = "CUDI";# ./appconsole.pl <host> <port> RCCACHEUPDATEDETECTION <interval>
$CMD_RC_CLIENT_POOL_SAMPLING_INTERVAL = "CPSI";  # ./appconsole.pl <host> <port> RCCLIENTPOOLSAMPLINGINT <interval>
$CMD_RC_CLIENT_POOL_ADJUST_INTERVAL = "CPAI";    # ./appconsole.pl <host> <port> RCCLIENTPOOLADJUSTINT <interval>
$CMD_RC_MASTER_ALL_DATA_TYPES = "RCMA";          # ./appconsole.pl <host> <port> RCMASTERALL <[Y|N]>
$CMD_RC_STATS_SAMPLING = "RCSS";                 # ./appconsole.pl <host> <port> RCSTATSSAMPLING <interval>
$CMD_RC_STATS_LOGGING_INTERVAL = "RCLI";         # ./appconsole.pl <host> <port> RCSTATSLOGGINGINT <interval>
$CMD_RC_IDLE_MASTER_TIMEOUT = "RCIM";   # ./appconsole.pl <host> <port> RCIDLEMASTERTO <parm>
$CMD_RC_IDLE_CLIENT_TIMEOUT = "RCIC";   # ./appconsole.pl <host> <port> RCIDLESLAVETO <parm>
$CMD_RC_SERVER_RECEIVE_TIMEOUT = "RCSR";# ./appconsole.pl <host> <port> RCMASTERRECEIVETO <parm>
$CMD_RC_SERVER_SEND_TIMEOUT = "RMST";   # ./appconsole.pl <host> <port> RCMASTERSENDTO <parm>
$CMD_RC_CLIENT_SEND_TIMEOUT = "CSTO";   # ./appconsole.pl <host> <port> RCSLAVESENDTO <parm>
$CMD_RC_SET_MIN_CLIENTS = "RSMC";       # ./appconsole.pl <host> <port> RCMINCLIENTSRATIO <parm>
$CMD_RC_RESET_PARAMETERS = "RSTP";      # ./appconsole.pl <host> <port> RCRESETPARAMETERS
$CMD_RC_DEBUG = "RCDG";                 # ./appconsole.pl <host> <port> RCDEBUG <[Y|N]>

sub sendReceive
{
   my ($host, $port, $cmd, $payload) = @_;
   my $recvMesg = "";
   my $message = "";
   my $data    = "";   

   $socket = IO::Socket::INET->new( Proto => 'tcp', PeerAddr => $host, PeerPort => $port,);
   if ($socket)
   {
      my $payloadLen = length($payload);

      $message = pack($HEADER_TEMPLATE, $HEADER_LEN, $payloadLen, $cmd, $HEADER_VER, $HEADER_REV);
      if ($payloadLen > 0)
      {
         $message .= $payload;
      }

      $socket->send($message);

      $socket->recv($header, $FULL_HEADER_LEN);
      ($headerLen, $payloadLen, $command, $version, $revision) = unpack($HEADER_TEMPLATE, $header);

      $toRecv = $payloadLen;

      while ($toRecv > 0)
      {
         $socket->recv($data, $toRecv);
         $recvMesg .= $data;
         $toRecv -= length($data);
      }

      close($socket);
   }
   else
   {
      output(ERROR, "Unable to connect to $host:$port");
   }

   return $recvMesg;
}

################################################################################
#
################################################################################

$numArgs = $#ARGV + 1;

if($numArgs < 3)
{
   print "\nappconsole.pl - invalid arguments\n";
   print "   ex. appconsole.pl [host] [port] [cmd] [pay]\n";
   print "       where host = hostname to connect to\n";
   print "             port = port number to connect to.\n";
   print "             cmd  = command to send to the server. Case Sensitive.\n";
   print "                    STATS    - Retreive server statistics.\n";
   print "                    CSTATS   - Retreive cache statistics.\n";
   print "                    DETAILS  - Retreive server details.\n";
   print "                    ELAPSED  - Retreive elapsed times.\n";
   print "                    STARTPR  - Start profiling.\n";
   print "                    FLUSHPR  - Flush profile data.\n";
   print "                    CLEARMEM - Clear memory statistics.\n";
   print "                    GETMEM   - Get memory statistics.\n";
   print "                    DB       - Get DB Connections.\n";
   print "             pay  = payload to send with command. optional.\n";
   exit;
}

$host = $ARGV[0];
$port = $ARGV[1];
$cmd  = $ARGV[2];
$payload = "";

if($numArgs > 3)
{
   $payload = $ARGV[3];
}

if ($cmd eq "STATS")
{
   $commandToSend = $CMD_STATS;
}
elsif ($cmd eq "CSTATS")
{
   $commandToSend = $CMD_CACHE_STATS;
}
elsif ($cmd eq "CCSTATS")
{
   $commandToSend = $CMD_COMPR_CACHE_STATS;
}
elsif ($cmd eq "DSPCACHE")
{
   $commandToSend = $CMD_DISPOSE_CACHE;
}
elsif ($cmd eq "DB")
{
   $commandToSend = $CMD_DB;
}
elsif ($cmd eq "DETAILS")
{
   $commandToSend = $CMD_DETAILS;
}
elsif ($cmd eq "ELAPSED")
{
   $commandToSend = $CMD_TO_ELAPSED;
}
elsif ($cmd eq "STARTPR")
{
   $commandToSend = $CMD_START_PROFILE;
}
elsif ($cmd eq "FLUSHPR")
{
   $commandToSend = $CMD_FLUSH_PROFILE;
}
elsif ($cmd eq "CLEARMEM")
{
   $commandToSend = $CMD_CLEAR_MEMSTAT;
}
elsif ($cmd eq "GETMEM")
{
   $commandToSend = $CMD_GET_MEMSTAT;
}
elsif ($cmd eq "STOP")
{
   $commandToSend = $CMD_STOP;
}
elsif ($cmd eq "DCFG")
{
   $commandToSend = $CMD_DCFG; 
}
elsif ($cmd eq "ERCT")
{
   $commandToSend = $CMD_ERROR_COUNTS; 
}
elsif ($cmd eq "SERVER_CPU")
{
   $commandToSend = $CMD_SERVER_CPU;
}
elsif ($cmd eq "SERVER_CONFIG")
{
   $commandToSend = $CMD_SERVER_CONFIG;
}
elsif ($cmd eq "QUERY_ELAPSED")
{
   $commandToSend = $CMD_QUERY_ELAPSED;
}
elsif ($cmd eq "RCHEALTHCHECK")
{
   $commandToSend = $CMD_RC_HEALTH_CHECK;
}
elsif ($cmd eq "RCDISABLE")
{
   $commandToSend = $CMD_RC_DISABLE;
}
elsif ($cmd eq "RCENABLE")
{
   $commandToSend = $CMD_RC_ENABLE;
}
elsif ($cmd eq "RCPERSISTENT")
{
   $commandToSend = $CMD_RC_PERSISTENT;
}
elsif ($cmd eq "RCNONPERSISTENT")
{
   $commandToSend = $CMD_RC_NONPERSISTENT;
}
elsif ($cmd eq "RCLDCDELAY")
{
   $commandToSend = $CMD_RC_LDC_DELAY;
}
elsif ($cmd eq "RCGETPARAMETERS")
{
   $commandToSend = $CMD_RC_GET_PARAMETERS;
}
elsif ($cmd eq "RCGETCACHES")
{
   $commandToSend = $CMD_RC_GET_CACHES;
}
elsif ($cmd eq "FLSH")
{
   $commandToSend = $CMD_CACHE_FLUSH;
}
elsif ($cmd eq "RCTHREADPOOLSIZE")
{
   $commandToSend = $CMD_RC_THREAD_POOL_SIZE;
}
elsif ($cmd eq "RCMAXNUMBERCLIENTS")
{
   $commandToSend = $CMD_RC_MAX_NUMBER_CLIENTS;
}
elsif ($cmd eq "RCHEALTHCHECKTIMEOUT")
{
   $commandToSend = $CMD_RC_HEALTHCHECK_TIMEOUT;
}
elsif ($cmd eq "RCASYNCHRONOUSHEALTHCHECK")
{
   $commandToSend = $CMD_RC_ASYNCHRONOUS_HEALTHCHECK;
}
elsif ($cmd eq "RCENABLEHEALTHCHECK")
{
   $commandToSend = $CMD_RC_ENABLE_HEALTHCHECK;
}
elsif ($cmd eq "RCHEALTHCHECKPERIOD")
{
   $commandToSend = $CMD_RC_HEALTHCHECK_PERIOD;
}
elsif ($cmd eq "RCQUEUETOLERANCE")
{
   $commandToSend = $CMD_RC_QUEUE_TOLERANCE;
}
elsif ($cmd eq "RCSLAVECONNECTTO")
{
   $commandToSend = $CMD_RC_SLAVE_CONNECT_TO;
}
elsif ($cmd eq "RCLINGER")
{
   $commandToSend = $CMD_RC_LINGER;
}
elsif ($cmd eq "RCLINGERTIME")
{
   $commandToSend = $CMD_RC_LINGER_TIME;
}
elsif ($cmd eq "RCKEEPALIVE")
{
   $commandToSend = $CMD_RC_KEEP_ALIVE;
}
elsif ($cmd eq "RCUSEREQUESTTO")
{
   $commandToSend = $CMD_RC_USE_CLIENT_SPECIFIED_TIMEOUT;
}
elsif ($cmd eq "RCPROCESSTIMEOUT")
{
   $commandToSend = $CMD_RC_CLIENT_PROCESSING_TIMEOUT;
}
elsif ($cmd eq "RCCHECKBASELINE")
{
   $commandToSend = $CMD_RC_CHECK_BASELINE;
}
elsif ($cmd eq "RCIGNOREDBMISMATCH")
{
   $commandToSend = $CMD_RC_IGNORE_DB_MISMATCH;
}
elsif ($cmd eq "RCCACHEUPDATEDETECTION")
{
   $commandToSend = $CMD_RC_CACHE_UPDATE_DETECTION_INTERVAL;
}
elsif ($cmd eq "RCCLIENTPOOLSAMPLINGINT")
{
   $commandToSend = $CMD_RC_CLIENT_POOL_SAMPLING_INTERVAL;
}
elsif ($cmd eq "RCCLIENTPOOLADJUSTINT")
{
   $commandToSend = $CMD_RC_CLIENT_POOL_ADJUST_INTERVAL;
}
elsif ($cmd eq "RCMASTERALL")
{
   $commandToSend = $CMD_RC_MASTER_ALL_DATA_TYPES;
}
elsif ($cmd eq "RCSTATSSAMPLING")
{
   $commandToSend = $CMD_RC_STATS_SAMPLING;
}
elsif ($cmd eq "RCSTATSLOGGINGINT")
{
   $commandToSend = $CMD_RC_STATS_LOGGING_INTERVAL;
}
elsif ($cmd eq "RCIDLEMASTERTO")
{
   $commandToSend = $CMD_RC_IDLE_MASTER_TIMEOUT;
}
elsif ($cmd eq "RCIDLESLAVETO")
{
   $commandToSend = $CMD_RC_IDLE_CLIENT_TIMEOUT;
}
elsif ($cmd eq "RCMASTERRECEIVETO")
{
   $commandToSend = $CMD_RC_SERVER_RECEIVE_TIMEOUT;
}
elsif ($cmd eq "RCMASTERSENDTO")
{
   $commandToSend = $CMD_RC_SERVER_SEND_TIMEOUT;
}
elsif ($cmd eq "RCSLAVESENDTO")
{
   $commandToSend = $CMD_RC_CLIENT_SEND_TIMEOUT;
}
elsif ($cmd eq "RCMINCLIENTSRATIO")
{
   $commandToSend = $CMD_RC_SET_MIN_CLIENTS;
}
elsif ($cmd eq "RCRESETPARAMETERS")
{
   $commandToSend = $CMD_RC_RESET_PARAMETERS;
}
elsif ($cmd eq "RCDEBUG")
{
   $commandToSend = $CMD_RC_DEBUG;
}
else
{
   print "\nInvalid Command!\n";
   exit;
}

$message = sendReceive($host, $port, $commandToSend, $payload);
if ($message eq "")
{
   print "\nEmpty response from server!\n\n";
}
else
{
   print "\n$message\n\n";
}
