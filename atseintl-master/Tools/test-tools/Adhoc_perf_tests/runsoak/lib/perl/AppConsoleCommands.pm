package AppConsoleCommands;

use IO::Socket;

use vars qw($HEADER_TEMPLATE, $FULL_HEADER_LEN, $HEADER_LEN, $HEADER_VER,
            $HEADER_REV, $CMD_SSTATS, $CMD_CSTATS, $CMD_ELAPSED);

# Global constats used for sending app console commands
my $HEADER_TEMPLATE  = "N N A4 A4 A4";
my $FULL_HEADER_LEN  = 20;
my $HEADER_LEN       = 16;
my $HEADER_VER       = "0001";
my $HEADER_REV       = "0000";
my $CMD_SSTATS       = 'RFSH';
my $CMD_CSTATS       = 'TATS';
my $CMD_ELAPSED      = 'LAPS';
my $CMD_ERROR_COUNTS = 'ERCT';
my $CMD_CACHE_MEMORY = 'MEMC';
my $CMD_DETAILS      = "ETAI";
my $CMD_STOP         = "DOWN";

sub new
{
    my $package = shift;
    return bless({}, $package);
}

# Get Stats from app console
sub getStats
{
    my $self = shift;
    my ($host, $port) = @_;
    my $trxCount  = 0;
    my $errCount  = 0;
    my $cpu       = 0;
    my $elapsed   = 0;
    my $vmemory    = 0;
    my $dbCount   = 0;
    my $dbElapsed = 0;
    my $dbHost    = 0;
    my $itins     = 0;
    my $responseSize = 0;

    $stats = $self->sendReceive($host, $port, $CMD_SSTATS, "");
    if ($stats =~ m/^<STATS.*?CP="(.*?)" IT="(.*?)".*? DB="(.*?)".*? RS="(.*?)" DQ="(.*?)" VM="(.*?)" DH="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)" ET="(.*?)".*?RS="(.*?)"/)
    {
        $cpu = $1;
        $itins = $2;
        $dbCount = $3 + $5;
        $rmemory = $4;
        $vmemory = $6;
        $dbHost = $7;
        $trxCount = $8 + $9;
        $errCount = $9;
        $elapsed = $10;
        $responseSize = $11;
    }
    $elapsedstats = $self->sendReceive($host, $port, $CMD_ELAPSED, "");
    if ($elapsedstats =~ m/^<ELAPSED.*? DS="(.*?)"/)
    {
        $dbElapsed = $1;
    }
    return( $trxCount, $cpu, $elapsed, $vmemory, $dbCount, $dbElapsed, $dbHost,
            $itins, $responseSize, $errCount, $rmemory );
}

# Get Detailed Stats from app console
sub getDetailedStats
{
    my $self = shift;
    my ($host, $port) = @_;
    my $trxCount  = 0;
    my $errCount  = 0;
    my $cpu       = 0;
    my $elapsed   = 0;
    my $vmemory    = 0;
    my $rmemory   = 0;
    my $dbCount   = 0;
    my $dbElapsed = 0;
    my $fcElapsed = 0;
    my $fvElapsed = 0;
    my $poElapsed = 0;
    my $iaElapsed = 0;
    my $dbHost    = 0;
    my $itins     = 0;
    my $responseSize = 0;

    $stats = $self->sendReceive($host, $port, $CMD_SSTATS, "");
    if ($stats =~ m/^<STATS.*?CP="(.*?)" IT="(.*?)".*? DB="(.*?)".*? RS="(.*?)" DQ="(.*?)" VM="(.*?)" DH="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)" ET="(.*?)".*?RS="(.*?)"/)
    {
        $cpu = $1;
        $itins = $2;
        $dbCount = $3 + $5;
        $rmemory = $4;
        $vmemory = $6;
        $dbHost = $7;
        $trxCount = $8 + $9;
        $errCount = $9;
        $elapsed = $10;
        $responseSize = $11;
    }
    $elapsedstats = $self->sendReceive($host, $port, $CMD_ELAPSED, "");
    #TODO - add code to get FCO, IA, PO, FVO elapsed stats
    if ($elapsedstats =~ m/^<ELAPSED FC="(.*?)" FV="(.*?)" PO="(.*?)".*? IA="(.*?)".*? DS="(.*?)"/)
    {
        $fcElapsed = $1;
        $fvElapsed = $2;
        $poElapsed = $3;
        $iaElapsed = $4;
        $dbElapsed = $5;
    }
    return( $trxCount, $cpu, $elapsed, $vmemory, $dbCount, $dbElapsed, $dbHost,
            $itins, $responseSize, $errCount, $fcElapsed, $fvElapsed,
            $poElapsed, $iaElapsed, $rmemory );
}

# get DB HOST/SERVICE NAME
sub getDBInfo
{
    my $self = shift;
    my ($host, $port) = @_;
    my $dbHost    = "";

    my $stats = $self->sendReceive($host, $port, $CMD_SSTATS, "");
    if ($stats =~ m/^<STATS.*?CP="(.*?)" IT="(.*?)".*? DB="(.*?)".*? DQ="(.*?)" VM="(.*?)" DH="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)" ET="(.*?)".*?RS="(.*?)"/)
    {
        $dbHost = $6;
    }
    return $dbHost;
}

# Get Error Counts
sub getErrorCounts
{
    my $self = shift;
    my ($host, $port) = @_;
    return $self->sendReceive($host, $port, $CMD_ERROR_COUNTS, "");
}

sub getErrorCountsMap
{
    my $self = shift;
    my ($host, $port) = @_;
    my $errorCounts = $self->getErrorCounts($host, $port);
    my %errMap = ();
    @tokens = split(/\|/, $errorCounts);
    $numtokens = @tokens;
    $numtokens--;
    for (my $index = 0; $index < $numtokens; $index += 2)
    {
        $errMap{@tokens[$index]} = @tokens[$index+1];
    }
    return %errMap;
}

# Method to send app console command and receive response
sub sendReceive
{
   my $self = shift;
   my ($host, $port, $cmd, $payload) = @_;
   my $recvMesg = "";
   my $message  = "";
   my $data     = "";   

   $socket = IO::Socket::INET->new( Proto => 'tcp', PeerAddr => $host,
                                    PeerPort => $port,);
   if ($socket)
   {
      my $payloadLen = length($payload);
      $message = pack($HEADER_TEMPLATE, $HEADER_LEN, $payloadLen, $cmd,
                      $HEADER_VER, $HEADER_REV);
      if ($payloadLen > 0)
      {
         $message .= $payload;
      }

      $socket->send($message);
      $socket->recv($header, $FULL_HEADER_LEN);
      ($headerLen, $payloadLen, $command, $version, $revision) =
           unpack($HEADER_TEMPLATE, $header);

      $toRecv = $payloadLen;
      while ($toRecv > 0)
      {
         $socket->recv($data, $toRecv);
         $recvMesg .= $data;
         $toRecv -= length($data);
      }

      close($socket);
      return $recvMesg;
   }
   return undef;
}

# get cache statistics
sub getCacheStats
{
    my $self = shift;
    my ($host, $port) = @_;
    my $cstats = $self->sendReceive($host, $port, $CMD_CSTATS, "");

    my @cstatsarr = split(/\|/, $cstats);
    my $size = @cstatsarr;
    my $numentries = $size/5 - 1; #there are 5 elements for each cache entry
    my $index = 0;
    my $results = "";
    while($index < $numentries)
    {
        my $cachename = $cstatsarr[$index*5];
        my $cachecount = $cstatsarr[$index*5+2];
        $index++;
        next if($cachename =~ m/HISTORICAL/);
        next if($cachecount eq 0);
        $results .= $cachename . "|" . $cachecount . "\n";
    }

    return $results;
    #return "";
}

sub getCacheMemoryStats
{
    my $self = shift;
    my ($host, $port) = @_;
    my $cstats = $self->sendReceive($host, $port, $CMD_CACHE_MEMORY, "");

    my $result = $cstats;
    return $result;
}

sub getBaselineName
{
    my $self = shift;
    my ( $host, $port ) = @_;

    my $details = $self->sendReceive($host, $port, $CMD_DETAILS, "");
    my @tokens = split(/\|/, $details);
    my $baselineName = $tokens[2];
    #my $len = length $baselineName;
    #substr($baselineName, $len-1) = "";
    return $baselineName;
}

sub shutdownServer
{
    my $self = shift;
    my ( $host, $port ) = @_;
    my $result = $self->sendCommand($host, $port, $CMD_STOP, "");
    if(defined($result)) {
        return 1;
    }
    return 0;
}

sub isRunning
{
    my $self = shift;
    my ( $host, $port ) = @_;
    my $cstats = $self->sendReceive($host, $port, $CMD_CACHE_MEMORY, "");
    if(defined($cstats)) {
        return 1;
    }
    return 0;
}

sub sendCommand
{
    my $self = shift;
    my ($host, $port, $command, $payload) = @_;
    my $cstats = $self->sendReceive($host, $port, $command, $payload);

    my $result = $cstats;
    return $result;
}

1;
__END__
