package ProdServers;

my @is;
my @isExp200;
my @isTvly200;
my @isDPool;
my @isGPool;
my @mip;
my @mipExp200;
my @mipTvly200;
my @mipDPool;
my @mipGPool;

use constant IS_ALL    => 0; # ALL IS
use constant MIP_ALL   => 1; #ALL MIP
use constant IS        => 2; #IS GEN
use constant MIP       => 3; #MIP GEN
use constant IS_EXP    => 4; #IS EXPEDIA C POOL
use constant MIP_EXP   => 5; #MIP EXPEDIA C POOL
use constant IS_TVLY   => 6; #IS TVLY E POOL
use constant MIP_TVLY  => 7; #MIP TVLY E POOL
use constant IS_DPOOL  => 8; #IS EMEA D POOL
use constant MIP_DPOOL => 9; #MIP EMEA D POOL
use constant IS_GPOOL  => 10; #IS eTRAVELi G POOL
use constant MIP_GPOOL => 11; #MIP eTRAVELi G POOL
use constant IS_HPOOL  => 12; #IS TVLY H POOL
use constant MIP_HPOOL => 13; #MIP TVLY H POOL

sub new
{
    my $package = shift;
    return bless({}, $package);
}

sub printServers
{
  my $self = shift;
  print "is => @is \n";
  print "isExp200 => @isExp200 \n";
  print "isTvly200 => @isTvly200 \n";
  print "isDPool => @isDPool \n";
  print "isGPool => @isGPool \n";
  print "isHPool => @isHPool \n";
  print "mip => @mip \n";
  print "mipExp200 => @mipExp200 \n";
  print "mipTvly200 => @mipTvly200 \n";
  print "mipDPool => @mipDPool \n";
  print "mipGPool => @mipGPool \n";
  print "mipHPool => @mipHPool \n";
}

sub initServers
{
  my $self = shift;
  my ( $ncm ) = @_;
  #my $ncm = "/opt/atseintl/shopperftest/ncm.csv";
  @is         = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, A" );
  @isExp200   = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, C" );
  @isTvly200  = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, E" );
  @isDPool    = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, D" );
  @isGPool    = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, G" );
  @isHPool    = $self->initServerNames( $ncm, "ADMIN_SHOPPINGIS_PROD_GROUP, H" );
  @mip        = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, A"   );
  @mipExp200  = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, C"   );
  @mipTvly200 = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, E"   );
  @mipDPool   = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, D"   );
  @mipGPool   = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, G"   );
  @mipHPool   = $self->initServerNames( $ncm, "ADMIN_SHOPPING_PROD_GROUP, H"   );
}

sub initServerNames
{
  my $self = shift;
  my ( $ncmFile, $pool ) = @_;
  my $tmpFile = ".tmpfile.txt" . "." . rand(time());
  my @servers;

  #print "tmpfile = $tmpFile \n";
  unlink( $tmpFile );
  system( "grep \"$pool\" $ncmFile | grep -v '^#' | sed -e 's/^[ ]*//g' | cut -d',' -f1 > $tmpFile " );
  #system( "cat $tmpFile");
  open INPUT, "<$tmpFile" or die "could not open '$tmpFile' for reading";
  while( my $line = <INPUT> )
  {
    $line =~ s/\n$//;
    $line =~ s/\s+$//;
    @tokens = split / /, $line;
    foreach $token (@tokens) {
      if( $token =~ m/(.*?)-(.*?)$/ ) {
        $start = $1;
        $end = $2;
        #print "range: $start to $end\n";
        if( $start =~ m/([a-z]*)([0-9]*)/ ) {
          $prefix = $1;
          $startNode = $2;
        }
        if( $end =~ m/([a-z]*)([0-9]*)/ ) {
          $endNode = $2;
        }
        #print "prefix=$prefix, start=$startNode, endNode=$endNode\n";
        for( $node = $startNode; $node <= $endNode; $node++ ) {
          $serverName = $prefix . $node;
          #print "node=$serverName\n";
          push( @servers, $serverName );
        }
      }
      else {
        #print "single: $token \n";
        push( @servers, $token );
      }
    } #for each token in the line
  } #for each line in the file
  close INPUT;
  unlink( $tmpFile );
  #print "serversList = @servers \n";
  @uniqservers = sort {lc($a) cmp lc($b)} keys %{{ map { $_ => 1 } @servers }};
  #print "serversList = @uniqservers \n";
  return @uniqservers; 
}

################################################################################
#
# getServerName SERVERTYPE
#
#   Returns a node id for the specified serverType. serverType can be one of
#   the following.
#
#   0 => IS ALL
#   1 => MIP ALL
#   2 => IS General/Common Pool
#   3 => MIP General/Common Pool
#   4 => IS Expedia 200 Options
#   5 => MIP Expedia 200 Options
#   6 => IS Travelocity 200 Options
#   7 => MIP Travelocity 200 Options
#   8 => IS D Pool (EMEA)          
#   9 => MIP D Pool (EMEA)          
#   10=> IS G Pool (eTRAVELi)          
#   11=> MIP G Pool (eTRAVELi)          
#   12=> IS H Pool (TVLY INTL)          
#   13=> MIP H Pool (TVLY INTL)          
#
################################################################################
sub getServerNames
{
  my $self = shift;
  my ( $nodeType ) = @_;
  my $size;
  my @serverList;

  if ($nodeType == IS)
  {
    @serverList = @is;
  }
  elsif ($nodeType == MIP)
  {
    @serverList = @mip;
  }
  elsif ($nodeType == IS_EXP)
  {
    @serverList = @isExp200;
  }
  elsif ($nodeType == MIP_EXP)
  {
    @serverList = @mipExp200;
  }
  elsif ($nodeType == IS_TVLY)
  {
    @serverList = @isTvly200;
  }
  elsif ($nodeType == MIP_TVLY)
  {
    @serverList = @mipTvly200;
  }
  elsif ($nodeType == IS_DPOOL)
  {
    @serverList = @isDPool;  
  }
  elsif ($nodeType == MIP_DPOOL)
  {
    @serverList = @mipDPool;
  }
  elsif ($nodeType == IS_GPOOL)
  {
    @serverList = @isGPool;  
  }
  elsif ($nodeType == MIP_GPOOL)
  {
    @serverList = @mipGPool;
  }
  elsif ($nodeType == IS_HPOOL)
  {
    @serverList = @isHPool;  
  }
  elsif ($nodeType == MIP_HPOOL)
  {
    @serverList = @mipHPool;
  }
  elsif($nodeType == IS_ALL)
  {
    push(@serverList, @is);
    push(@serverList, @isExp200);
    push(@serverList, @isTvly200);
    push(@serverList, @isDPool);
    push(@serverList, @isGPool);
    push(@serverList, @isHPool);
  }
  elsif($nodeType == MIP_ALL)
  {
    push(@serverList, @mip);
    push(@serverList, @mipExp200);
    push(@serverList, @mipTvly200);
    push(@serverList, @mipDPool);
    push(@serverList, @mipGPool);
    push(@serverList, @mipHPool);
  }

  my ($lPos, $lRand);
  # For every position in the list, swap it with a random
  # position earlier in the list.
  foreach $lPos (1..$#serverList) {
    $lRand = int(rand($lPos+1));
    @serverList[$lPos, $lRand] = @serverList[$lRand, $lPos];
  }

  return @serverList;
}

sub getServerName
{
  my $self = shift;
  my ( $nodeType ) = @_;
  my $size;
  my @serverList = $self->getServerNames( $nodeType );

#  if ($nodeType == IS)
#  {
#    @serverList = @is;
#  }
#  elsif ($nodeType == MIP)
#  {
#    @serverList = @mip;
#  }
#  elsif ($nodeType == IS_EXP)
#  {
#    @serverList = @isExp200;
#  }
#  elsif ($nodeType == MIP_EXP)
#  {
#    @serverList = @mipExp200;
#  }
#  elsif ($nodeType == IS_TVLY)
#  {
#    @serverList = @isTvly200;
#  }
#  elsif ($nodeType == MIP_TVLY)
#  {
#    @serverList = @mipTvly200;
#  }
#  elsif ($nodeType == IS_DPOOL)
#  {
#    @serverList = @isDPool;  
#  }
#  elsif ($nodeType == MIP_DPOOL)
#  {
#    @serverList = @mipDPool;
#  }
#  elsif($nodeType == IS_ALL)
#  {
#    push(@serverList, @is);
#    push(@serverList, @isExp200);
#    push(@serverList, @isTvly200);
#    push(@serverList, @isDPool);
#  }
#  elsif($nodeType == MIP_ALL)
#  {
#    push(@serverList, @mip);
#    push(@serverList, @mipExp200);
#    push(@serverList, @mipTvly200);
#    push(@serverList, @mipDPool);
#  }
  $size = @serverList;

  my $serverIndex = int(rand($size));
  return $serverList[$serverIndex];
}

1;
__END__
