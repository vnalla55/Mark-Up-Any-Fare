#! /usr/bin/perl
use strict vars;
use warnings;
use IO::Socket;

my $port = -1;
my @hostList = ();
my @userTables = ();
my $defaultList = [()];
my $allTablesRequested = 0;
my $listOnly = 0;
my $doImport = 0;
my $groupRequested = "";
my $env = "";

my $usage = "usage:\nflush.pl {-h host -p port {-l | --list-only}} | {--pricing | --faredisplay | --tax | --shopping | --historical | --group [groupname]} {--all-tables | [table...] }";

if (! -f $ENV{"HOME"} . "/bin/groupmembers.pl" ) {
  $doImport = 1;
}

while ($_ = shift @ARGV)
{
    if (lc $_ eq '--shopping')
    {
        $groupRequested = 'shopping';
    }
    elsif (lc $_ eq '--faredisplay')
    {
        $groupRequested = 'faredisplay';
    }
    elsif (lc $_ eq '--tax')
    {
        $groupRequested = 'tax';
    }
    elsif (lc $_ eq '--pricing')
    {
        $groupRequested = 'pricing';
    }
    elsif (lc $_ eq '--historical')
    {
        $groupRequested = 'historical';
    }
    elsif (lc $_ eq '--group')
    {
        $groupRequested = lc shift @ARGV;
    }
    elsif (lc $_ eq '--env')
    {
        $env = lc shift @ARGV;
    }
    elsif (lc $_ eq '--prod')
    {
        $env = "prod";
    }
    elsif (lc $_ eq '--int')
    {
        $env = "int";
    }
    elsif (lc $_ eq '--cert')
    {
        $env = "cert";
    }
    elsif ($_ eq '-h' || $_ eq '--host')
    {
        my $host = shift @ARGV;
        push @hostList, $host;
    }
    elsif ($_ eq '-p' || $_ eq '--port')
    {
        if ($port >= 0)
        {
          print STDERR "Port number has already been specified as $port.\n";
          die $usage;
        }
        $port = shift @ARGV;
    }
    elsif ($_ eq '--help' || $_ eq '--usage')
    {
        print "$usage\n";
        exit 0;
    }
    elsif ($_ eq '--all-tables')
    {
        $allTablesRequested = 1;
    }
    elsif ($_ eq '--list-only' || $_ eq '-l')
    {
        $listOnly = 1;
    }
    elsif ($_ eq '--import')
    {
        $doImport = 1;
    }
    elsif ($_ eq '--no-import')
    {
        $doImport = 0;
    }
    elsif (/-.*/)
    {
        print STDERR "unknown option ", $_, "\n";
    }
    else
    {
        push @userTables, $_;
    }
}

if ($env eq "") { $env = 'prod'; }

if ($doImport) {
  print "Importing AppConsole data...";
  system $ENV{"HOME"} . "/bin/appconsoleimport.pl > /dev/null";
  print " done.\n";
}

if (! $groupRequested eq "") {
  require $ENV{"HOME"} . "/bin/groupmembers.pl";
  if ($port < 0) {
    $port = getPortForGroup ($groupRequested);
    if ($port < 0) {
      die "No port number specified - bailing out. ";
    }
  }

  if (!@hostList) {
    @hostList = hostsInGroup ($env, $groupRequested);
  }
  
  if (scalar @hostList == 0) {
    die "Sorry, the group you specified is empty in " . uc $env . "!\n";
  }
}

if ((scalar @hostList) > 1 && $listOnly)
{
  print STDERR "You can only list a single host.\n";
  die $usage;
}

if ((scalar @hostList) > 1 && $allTablesRequested)
{
  print STDERR "You can only freeze a single host.\n";
  die $usage;
}

if (!@hostList)
{
    print STDERR "You have to specify either the host/port pair or the hosts group.\n";
    die $usage;
};

if ((!$allTablesRequested && @userTables == 0) && !$listOnly)
{
    print STDERR "You have to specify either the --all-tables switch or a list of cache tables.\n";
    die $usage;
};

if ($allTablesRequested && @userTables != 0)
{
    print STDERR "You have to specify either the --all-tables switch or a list of cache tables, but not both!\n";
    die $usage;
};

my $host;
foreach  $host (@hostList)
{
    my @tables;
    my $table;
    
    my $socket = createSocket ($host, $port);
    if (!$socket) {
      next;
    }
    
    my %allTables = getTablesList ($socket);

    if ($allTablesRequested || ((@userTables == 0) && $listOnly))
    {
      @tables = keys (%allTables);
    }
    else
    {
      @tables = ();
      foreach $table (@userTables)
      {
        if ((!defined $allTables {$table}) && !$listOnly)
        {
          print STDERR "Table $table not found on host $host:$port!\n";
        }
        if (defined $allTables {$table})
        {
          push @tables, $table;
        }
      }
    }

    if ($listOnly)
    {
      foreach $table (@tables)
      {
        print "$table\n";
      }
      exit 0;
    }

    $| = 1;
    my $size = @tables;
    my $pos = 0;
    foreach $table (@tables)
    {
      $pos++;
      print "Flushing server $host:$port, table $table\n";
      sendCommand ($socket, 'FLSH', $table);
      sleep 1;
    }
    $socket->close();
    $| = 0;
#    print "Flush finished.\n";
}

exit;

sub getTablesList
{
  my $socket = shift;
  my $message = sendCommand ($socket, 'TATS', "");
  if ($message eq "")
  {
    die "\nEmpty response from server!\n\n";
  }
  my @fields = split '\|', $message;
  my $field;
  my %cacheNames;
  for( my $num = 0; $num <= $#fields; $num += 5)
  {
    my $field = $fields[$num];  
    if ($field ne '' && $field ne "\0")
    {
        $cacheNames{"$field"} = 1;
    }
  }
  return %cacheNames;
}

sub createSocket
{
  my $host = shift;
  my $port = shift;
  my $sock = new IO::Socket::INET(PeerAddr => $host, PeerPort => $port, Proto => 'tcp' );
  if (!$sock) {
    print STDERR "Could not connect to $host:$port, error message: " . $! . "\n";
  }
  else {
#    print "Connected to $host:$port\n";
  }
  return $sock;
}

sub sendCommand
{
    my $fullHeaderLen = 16;
    my $headerFormat = "N N A4 A4 A4";
    my $socket = shift;
    my $command = uc shift;
    my $payload = uc shift;
    
    if (length ($command) != 4) { die "Invalid command!" };
    
    my $request = pack ($headerFormat, $fullHeaderLen, length($payload), $command, '0001', '0000') . $payload;
    $socket->send ($request);

    my $header;
    $socket->recv($header, $fullHeaderLen+4);
    my ($headerLen, $payloadLen, $version, $revision);
    ($headerLen, $payloadLen, $command, $version, $revision) = unpack($headerFormat, $header);

    my $toRecv = $payloadLen;
    my $response = "";

    while ($toRecv > 0)
    {
        my $data;
        $socket->recv($data, $toRecv);
        $response .= $data;
        $toRecv -= length($data);
    }
    return $response;
}

sub getPortForGroup
{
  my $group = shift;
  if ($group =~ /faredisplay/i) {
    return 5002;
  }
  if ($group =~ /equinox/i) { # this must come before the pricing check as the group name is really PricingEquinox
    return 5004;
  }
  if ($group =~ /pricing/i) {
    return 5000;
  }
  if ($group =~ /shopping/i) {
    return 5001;
  }
  if ($group =~ /tax/i) {
    return 5003;
  }
  if ($group =~ /historical/i) {
    return 5006;
  }
  die "Unknown group: $group";
}
